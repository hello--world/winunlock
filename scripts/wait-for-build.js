#!/usr/bin/env node

/**
 * 等待并检测 GitHub Actions 构建结果
 * 
 * 功能：
 * 1. 等待构建完成
 * 2. 定期检查构建状态
 * 3. 返回构建结果
 * 4. 如果失败，返回错误信息
 */

import https from 'https';
import { checkBuildStatus, getWorkflowRunDetails } from './check-workflow.js';
import { getWorkflowRunLogs } from './get-workflow-logs.js';

const CONFIG = {
  owner: 'hello--world',
  repo: 'winunlock',
  branch: 'dev',
  checkInterval: 30000, // 30秒检查一次
  maxWaitTime: 600000,   // 10分钟超时
  githubToken: process.env.GITHUB_TOKEN || '',
};

/**
 * 等待构建完成并返回结果
 */
async function waitForBuild(commitSha, options = {}) {
  const {
    checkInterval = CONFIG.checkInterval,
    maxWaitTime = CONFIG.maxWaitTime,
    onStatusChange = null,
  } = options;

  console.log('⏳ 等待 GitHub Actions 构建完成...');
  console.log(`📌 提交 SHA: ${commitSha.substring(0, 7)}`);
  console.log(`⏱️  检查间隔: ${checkInterval / 1000}秒`);
  console.log(`⏱️  最大等待时间: ${maxWaitTime / 1000}秒\n`);

  const startTime = Date.now();
  let lastStatus = null;
  let lastRunId = null;

  while (Date.now() - startTime < maxWaitTime) {
    try {
      // 检查构建状态
      const result = await checkBuildStatus();
      
      if (!result.run) {
        console.log('⏳ 等待构建开始...');
        await new Promise(resolve => setTimeout(resolve, checkInterval));
        continue;
      }

      // 检查是否是我们的提交
      if (result.run.head_sha !== commitSha) {
        console.log(`⏳ 等待新的构建（当前运行是提交 ${result.run.head_sha.substring(0, 7)}）...`);
        await new Promise(resolve => setTimeout(resolve, checkInterval));
        continue;
      }

      const currentStatus = result.status || (result.success === null ? 'running' : result.success ? 'success' : 'failure');
      
      // 状态变化时输出
      if (currentStatus !== lastStatus || result.run.id !== lastRunId) {
        const statusIcon = currentStatus === 'success' ? '✅' : currentStatus === 'failure' ? '❌' : '⏳';
        console.log(`${statusIcon} 状态: ${currentStatus} (运行 #${result.run.id})`);
        
        if (onStatusChange) {
          onStatusChange(currentStatus, result);
        }
        
        lastStatus = currentStatus;
        lastRunId = result.run.id;
      }

      // 构建完成
      if (result.success === true) {
        console.log('\n✅ 构建成功！');
        const details = await getWorkflowRunDetails(result.run.id);
        return {
          success: true,
          runId: result.run.id,
          run: result.run,
          details: details,
          message: '构建成功'
        };
      } else if (result.success === false) {
        console.log('\n❌ 构建失败！');
        const details = await getWorkflowRunDetails(result.run.id);
        
        // 自动获取失败日志
        let logs = null;
        let errorSummary = null;
        
        if (options.autoFetchLogs !== false) {
          try {
            console.log('📥 正在获取失败日志...');
            logs = await getWorkflowRunLogs(result.run.id, {
              onlyFailed: true,
              verbose: false
            });
            
            if (logs && logs.allLogs) {
              errorSummary = {
                failedJobs: logs.failedJobs.length,
                failedSteps: logs.failedSteps.length,
                logText: logs.allLogs
              };
              console.log(`✅ 已获取日志（${logs.failedJobs.length} 个失败的 job）`);
            }
          } catch (error) {
            console.warn(`⚠️  无法获取日志: ${error.message}`);
          }
        }
        
        return {
          success: false,
          runId: result.run.id,
          run: result.run,
          details: details,
          message: '构建失败',
          errorUrl: result.run.html_url,
          logs: logs,
          errorSummary: errorSummary
        };
      }

      // 继续等待
      await new Promise(resolve => setTimeout(resolve, checkInterval));
    } catch (error) {
      console.error(`❌ 检查构建状态时出错: ${error.message}`);
      await new Promise(resolve => setTimeout(resolve, checkInterval));
    }
  }

  // 超时
  throw new Error(`构建超时（等待超过 ${maxWaitTime / 1000} 秒）`);
}

/**
 * 主函数
 */
async function main() {
  const args = process.argv.slice(2);
  const commitSha = args[0] || null;
  
  if (!commitSha) {
    // 如果没有提供 SHA，使用最新提交
    const { execSync } = await import('child_process');
    const latestSha = execSync('git rev-parse HEAD', { encoding: 'utf-8' }).trim();
    console.log(`📌 使用最新提交: ${latestSha.substring(0, 7)}\n`);
    
    try {
      const result = await waitForBuild(latestSha, {
        onStatusChange: (status, result) => {
          if (status === 'running') {
            console.log(`  运行中... (${new Date().toLocaleTimeString()})`);
          }
        }
      });
      
      if (result.success) {
        console.log(`\n🎉 构建成功！`);
        console.log(`📦 运行 ID: ${result.runId}`);
        console.log(`🔗 URL: ${result.run.html_url}`);
        process.exit(0);
      } else {
        console.log(`\n❌ 构建失败！`);
        console.log(`📦 运行 ID: ${result.runId}`);
        console.log(`🔗 查看错误: ${result.errorUrl}`);
        
        if (result.errorSummary) {
          console.log(`\n📋 错误摘要:`);
          console.log(`   失败的 Jobs: ${result.errorSummary.failedJobs}`);
          console.log(`   失败的 Steps: ${result.errorSummary.failedSteps}`);
        }
        
        console.log(`\n💡 要获取详细错误日志，请在 Cursor 中使用：`);
        console.log(`   "下载工作流程运行 #${result.runId} 的日志"`);
        process.exit(1);
      }
    } catch (error) {
      console.error(`\n❌ 错误: ${error.message}`);
      process.exit(1);
    }
  } else {
    // 使用提供的 SHA
    try {
      const result = await waitForBuild(commitSha);
      process.exit(result.success ? 0 : 1);
    } catch (error) {
      console.error(`\n❌ 错误: ${error.message}`);
      process.exit(1);
    }
  }
}

// 运行主函数
if (import.meta.url === `file://${process.argv[1]}` || process.argv[1]?.endsWith('wait-for-build.js')) {
  if (!CONFIG.githubToken) {
    console.warn('⚠️  警告: 未设置 GITHUB_TOKEN 环境变量');
    console.warn('   某些功能可能无法使用\n');
  }
  
  main().catch(error => {
    console.error('❌ 错误:', error.message);
    process.exit(1);
  });
}

export { waitForBuild };

