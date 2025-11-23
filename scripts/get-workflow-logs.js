#!/usr/bin/env node

/**
 * 获取 GitHub Actions 工作流程运行日志
 */

import https from 'https';
import fs from 'fs';

const CONFIG = {
  owner: 'hello--world',
  repo: 'winunlock',
  runId: process.argv[2] || '19604278709',
  githubToken: process.env.GITHUB_TOKEN || '',
};

function githubApiRequest(endpoint) {
  return new Promise((resolve, reject) => {
    const options = {
      hostname: 'api.github.com',
      path: endpoint,
      method: 'GET',
      headers: {
        'User-Agent': 'Workflow-Log-Script',
        'Accept': 'application/vnd.github.v3+json',
        ...(CONFIG.githubToken && { 'Authorization': `token ${CONFIG.githubToken}` }),
      },
    };

    const req = https.request(options, (res) => {
      let data = '';
      res.on('data', (chunk) => { data += chunk; });
      res.on('end', () => {
        try {
          if (res.statusCode >= 200 && res.statusCode < 300) {
            resolve(data);
          } else {
            const json = JSON.parse(data);
            reject(new Error(`GitHub API error: ${res.statusCode} - ${JSON.stringify(json)}`));
          }
        } catch (e) {
          // 可能是文本响应
          if (res.statusCode >= 200 && res.statusCode < 300) {
            resolve(data);
          } else {
            reject(new Error(`Failed: ${res.statusCode} - ${data.substring(0, 200)}`));
          }
        }
      });
    });

    req.on('error', reject);
    req.end();
  });
}

async function getWorkflowRunLogs(runId) {
  try {
    console.log(`📥 获取工作流程运行 #${runId} 的日志...\n`);
    
    // 首先获取 jobs
    const jobsResponse = await githubApiRequest(
      `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/runs/${runId}/jobs`
    );
    const jobs = JSON.parse(jobsResponse);
    
    if (!jobs.jobs || jobs.jobs.length === 0) {
      console.log('未找到 jobs');
      return;
    }
    
    console.log(`找到 ${jobs.jobs.length} 个 jobs:\n`);
    
    for (const job of jobs.jobs) {
      console.log(`\n${'='.repeat(60)}`);
      console.log(`Job: ${job.name}`);
      console.log(`状态: ${job.status} (${job.conclusion || '进行中'})`);
      console.log('='.repeat(60) + '\n');
      
      // 获取 job 的日志 URL
      if (job.logs_url) {
        try {
          console.log('📋 下载日志...\n');
          const logData = await githubApiRequest(job.logs_url.replace('https://api.github.com', ''));
          
          // 解析日志（ZIP 格式需要解压，这里先显示原始数据的一部分）
          console.log('日志内容（前 2000 字符）：\n');
          console.log(logData.substring(0, 2000));
          console.log('\n...\n');
          
          // 保存到文件
          const logFile = `workflow-log-${runId}-${job.id}.txt`;
          fs.writeFileSync(logFile, logData);
          console.log(`✅ 完整日志已保存到: ${logFile}\n`);
        } catch (error) {
          console.error(`❌ 无法下载日志: ${error.message}\n`);
        }
      }
      
      // 显示 steps
      if (job.steps && job.steps.length > 0) {
        console.log('Steps:\n');
        job.steps.forEach(step => {
          const icon = step.conclusion === 'success' ? '✅' : step.conclusion === 'failure' ? '❌' : '⏳';
          console.log(`  ${icon} ${step.name}`);
          console.log(`    状态: ${step.status} (${step.conclusion || '进行中'})`);
        });
        console.log('');
      }
    }
    
  } catch (error) {
    console.error('❌ 无法获取日志:', error.message);
    if (error.message.includes('401') || error.message.includes('403')) {
      console.log('\n💡 提示: 需要设置 GITHUB_TOKEN 环境变量来访问日志');
      console.log('   export GITHUB_TOKEN=your_token');
    }
  }
}

async function main() {
  if (!CONFIG.githubToken) {
    console.warn('⚠️  警告: 未设置 GITHUB_TOKEN 环境变量');
    console.warn('   可能无法下载完整日志，但可以查看 GitHub 网页\n');
  }
  
  await getWorkflowRunLogs(CONFIG.runId);
  
  console.log('\n💡 提示: 要查看完整日志，请访问：');
  console.log(`   https://github.com/${CONFIG.owner}/${CONFIG.repo}/actions/runs/${CONFIG.runId}`);
}

if (import.meta.url === `file://${process.argv[1]}` || process.argv[1]?.endsWith('get-workflow-logs.js')) {
  main().catch(error => {
    console.error('❌ 错误:', error.message);
    process.exit(1);
  });
}

export { getWorkflowRunLogs };

