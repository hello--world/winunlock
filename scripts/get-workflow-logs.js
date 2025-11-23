#!/usr/bin/env node

/**
 * 获取 GitHub Actions 工作流程运行日志
 * 增强版：支持 ZIP 日志解析和错误提取
 */

import https from 'https';
import fs from 'fs';
import { createWriteStream } from 'fs';
import { pipeline } from 'stream/promises';
import { createUnzip } from 'zlib';
import { Readable } from 'stream';

const CONFIG = {
  owner: 'hello--world',
  repo: 'winunlock',
  runId: process.argv[2] || '19604278709',
  githubToken: process.env.GITHUB_TOKEN || '',
};

/**
 * GitHub API 请求（支持二进制数据）
 */
function githubApiRequest(endpoint, options = {}) {
  return new Promise((resolve, reject) => {
    const requestOptions = {
      hostname: 'api.github.com',
      path: endpoint,
      method: 'GET',
      headers: {
        'User-Agent': 'Workflow-Log-Script',
        'Accept': options.binary ? 'application/vnd.github.v3.raw' : 'application/vnd.github.v3+json',
        ...(CONFIG.githubToken && { 'Authorization': `token ${CONFIG.githubToken}` }),
      },
    };

    const req = https.request(requestOptions, (res) => {
      const chunks = [];
      res.on('data', (chunk) => { chunks.push(chunk); });
      res.on('end', () => {
        const data = options.binary ? Buffer.concat(chunks) : Buffer.concat(chunks).toString('utf-8');
        try {
          if (res.statusCode >= 200 && res.statusCode < 300) {
            resolve(data);
          } else {
            if (options.binary) {
              reject(new Error(`GitHub API error: ${res.statusCode}`));
            } else {
              const json = JSON.parse(data);
              reject(new Error(`GitHub API error: ${res.statusCode} - ${JSON.stringify(json)}`));
            }
          }
        } catch (e) {
          // 可能是文本响应
          if (res.statusCode >= 200 && res.statusCode < 300) {
            resolve(data);
          } else {
            reject(new Error(`Failed: ${res.statusCode} - ${data.toString().substring(0, 200)}`));
          }
        }
      });
    });

    req.on('error', reject);
    req.end();
  });
}

/**
 * 解压 ZIP 格式的日志数据
 * 注意：GitHub Actions 日志是 ZIP 格式，但 Node.js 内置库不支持 ZIP
 * 这里尝试 gzip 解压（某些情况下可能是 gzip），如果失败则尝试作为文本读取
 */
async function extractZipLogs(zipBuffer) {
  try {
    // 检查是否是 ZIP 文件（ZIP 文件以 PK 开头）
    if (zipBuffer.length >= 2 && zipBuffer[0] === 0x50 && zipBuffer[1] === 0x4B) {
      // 这是 ZIP 文件，但 Node.js 内置库不支持 ZIP
      // 返回提示信息，建议使用外部工具或库
      console.warn('⚠️  检测到 ZIP 格式日志，但需要额外库来解压');
      console.warn('   建议：安装 adm-zip 或使用 GitHub API 的文本格式');
      // 尝试提取 ZIP 中的文本内容（简单尝试）
      // 对于 GitHub Actions，日志通常在 ZIP 的某个文件中
      // 这里返回一个提示，实际使用时可能需要安装 adm-zip
      return `[ZIP 格式日志，需要解压]\n文件大小: ${zipBuffer.length} 字节\n提示: 请使用 GitHub Actions 网页查看完整日志，或安装 adm-zip 库来解压`;
    }
    
    // 尝试 gzip 解压（某些情况下可能是 gzip）
    try {
      const unzip = createUnzip();
      const chunks = [];
      const stream = Readable.from([zipBuffer]);
      
      stream.pipe(unzip);
      
      return new Promise((resolve, reject) => {
        unzip.on('data', (chunk) => chunks.push(chunk));
        unzip.on('end', () => {
          resolve(Buffer.concat(chunks).toString('utf-8'));
        });
        unzip.on('error', (err) => {
          // 如果不是 gzip，尝试作为文本读取
          const text = zipBuffer.toString('utf-8');
          // 检查是否是有效的 UTF-8 文本
          if (text.length > 0 && !text.includes('\0')) {
            resolve(text);
          } else {
            resolve(`[无法解析的二进制数据]\n大小: ${zipBuffer.length} 字节\n提示: 这可能是 ZIP 格式，需要解压`);
          }
        });
      });
    } catch (e) {
      // 如果解压失败，尝试直接作为文本读取
      const text = zipBuffer.toString('utf-8');
      if (text.length > 0 && !text.includes('\0')) {
        return text;
      } else {
        return `[无法解析的二进制数据]\n大小: ${zipBuffer.length} 字节`;
      }
    }
  } catch (error) {
    // 最后的 fallback
    const text = zipBuffer.toString('utf-8');
    return text.length > 0 ? text : `[错误: ${error.message}]`;
  }
}

/**
 * 获取单个 job 的日志内容
 */
async function getJobLogs(job) {
  if (!job.logs_url) {
    return null;
  }

  try {
    const endpoint = job.logs_url.replace('https://api.github.com', '');
    const logData = await githubApiRequest(endpoint, { binary: true });
    
    // 尝试解压日志
    const extractedLogs = await extractZipLogs(logData);
    return extractedLogs;
  } catch (error) {
    console.warn(`⚠️  无法下载 job ${job.name} 的日志: ${error.message}`);
    return null;
  }
}

/**
 * 获取工作流程运行的所有日志
 * @param {string} runId - 运行 ID
 * @param {object} options - 选项
 * @returns {Promise<object>} 包含所有日志的结构化数据
 */
async function getWorkflowRunLogs(runId, options = {}) {
  const { 
    extractErrors = false, 
    saveToFile = false,
    onlyFailed = false 
  } = options;

  try {
    if (!runId) {
      throw new Error('runId 是必需的');
    }

    if (options.verbose !== false) {
      console.log(`📥 获取工作流程运行 #${runId} 的日志...\n`);
    }
    
    // 首先获取 jobs
    const jobsResponse = await githubApiRequest(
      `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/runs/${runId}/jobs`
    );
    const jobs = JSON.parse(jobsResponse);
    
    if (!jobs.jobs || jobs.jobs.length === 0) {
      if (options.verbose !== false) {
        console.log('未找到 jobs');
      }
      return { jobs: [], allLogs: '', failedJobs: [] };
    }
    
    if (options.verbose !== false) {
      console.log(`找到 ${jobs.jobs.length} 个 jobs:\n`);
    }
    
    const result = {
      jobs: [],
      allLogs: '',
      failedJobs: [],
      failedSteps: []
    };
    
    for (const job of jobs.jobs) {
      // 如果只获取失败的，跳过成功的
      if (onlyFailed && job.conclusion !== 'failure') {
        continue;
      }

      const jobInfo = {
        id: job.id,
        name: job.name,
        status: job.status,
        conclusion: job.conclusion,
        steps: job.steps || [],
        logs: null
      };

      if (options.verbose !== false) {
        console.log(`\n${'='.repeat(60)}`);
        console.log(`Job: ${job.name}`);
        console.log(`状态: ${job.status} (${job.conclusion || '进行中'})`);
        console.log('='.repeat(60) + '\n');
      }
      
      // 获取 job 的日志
      if (job.logs_url) {
        if (options.verbose !== false) {
          console.log('📋 下载日志...\n');
        }
        
        const logs = await getJobLogs(job);
        if (logs) {
          jobInfo.logs = logs;
          result.allLogs += `\n=== Job: ${job.name} ===\n${logs}\n`;
          
          if (options.verbose !== false) {
            // 显示日志的前 2000 字符
            console.log('日志内容（前 2000 字符）：\n');
            console.log(logs.substring(0, 2000));
            console.log('\n...\n');
          }
          
          // 保存到文件
          if (saveToFile) {
            const logFile = `workflow-log-${runId}-${job.id}.txt`;
            fs.writeFileSync(logFile, logs);
            if (options.verbose !== false) {
              console.log(`✅ 完整日志已保存到: ${logFile}\n`);
            }
          }
        }
      }
      
      // 处理失败的 job
      if (job.conclusion === 'failure') {
        result.failedJobs.push(jobInfo);
        
        // 收集失败的 steps
        if (job.steps) {
          job.steps.forEach(step => {
            if (step.conclusion === 'failure') {
              result.failedSteps.push({
                jobName: job.name,
                stepName: step.name,
                stepNumber: step.number
              });
            }
          });
        }
      }
      
      // 显示 steps
      if (job.steps && job.steps.length > 0 && options.verbose !== false) {
        console.log('Steps:\n');
        job.steps.forEach(step => {
          const icon = step.conclusion === 'success' ? '✅' : step.conclusion === 'failure' ? '❌' : '⏳';
          console.log(`  ${icon} ${step.name}`);
          console.log(`    状态: ${step.status} (${step.conclusion || '进行中'})`);
        });
        console.log('');
      }

      result.jobs.push(jobInfo);
    }
    
    return result;
    
  } catch (error) {
    console.error('❌ 无法获取日志:', error.message);
    if (error.message.includes('401') || error.message.includes('403')) {
      console.log('\n💡 提示: 需要设置 GITHUB_TOKEN 环境变量来访问日志');
      console.log('   export GITHUB_TOKEN=your_token');
    }
    throw error;
  }
}

async function main() {
  if (!CONFIG.githubToken) {
    console.warn('⚠️  警告: 未设置 GITHUB_TOKEN 环境变量');
    console.warn('   可能无法下载完整日志，但可以查看 GitHub 网页\n');
  }
  
  const result = await getWorkflowRunLogs(CONFIG.runId, { 
    saveToFile: true,
    verbose: true 
  });
  
  if (result.failedJobs.length > 0) {
    console.log(`\n❌ 发现 ${result.failedJobs.length} 个失败的 job`);
  }
  
  console.log('\n💡 提示: 要查看完整日志，请访问：');
  console.log(`   https://github.com/${CONFIG.owner}/${CONFIG.repo}/actions/runs/${CONFIG.runId}`);
  
  return result;
}

if (import.meta.url === `file://${process.argv[1]}` || process.argv[1]?.endsWith('get-workflow-logs.js')) {
  main().catch(error => {
    console.error('❌ 错误:', error.message);
    process.exit(1);
  });
}

export { getWorkflowRunLogs, getJobLogs };

