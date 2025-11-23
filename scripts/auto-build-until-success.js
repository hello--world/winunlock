#!/usr/bin/env node

/**
 * 完全自动化构建脚本 - 直到编译成功
 * 
 * 功能：
 * 1. 提交代码到 GitHub
 * 2. 推送到远程
 * 3. 等待并检查 GitHub Actions 构建状态
 * 4. 如果失败，获取错误日志
 * 5. 分析错误并自动修复（通过 Cursor AI）
 * 6. 重复步骤 1-5 直到构建成功
 */

const { execSync } = require('child_process');
const https = require('https');
const fs = require('fs');
const path = require('path');

// 配置
const CONFIG = {
  owner: 'hello--world',
  repo: 'winunlock',
  branch: 'dev',
  workflowName: 'Build WinUnlock',
  maxRetries: 10,
  checkInterval: 30000, // 30秒检查一次
  maxWaitTime: 600000, // 10分钟超时
  githubToken: process.env.GITHUB_TOKEN || '',
};

/**
 * 执行 Git 命令
 */
function gitCommand(cmd) {
  try {
    return execSync(cmd, { encoding: 'utf-8', stdio: 'pipe' }).trim();
  } catch (error) {
    throw new Error(`Git command failed: ${cmd}\n${error.message}`);
  }
}

/**
 * 检查是否有未提交的更改
 */
function hasUncommittedChanges() {
  const status = gitCommand('git status --porcelain');
  return status.length > 0;
}

/**
 * 提交代码
 */
function commitChanges(message = 'Auto commit: 修复构建错误') {
  if (!hasUncommittedChanges()) {
    console.log('✅ 没有需要提交的更改');
    return false;
  }

  console.log('📝 提交更改...');
  gitCommand('git add -A');
  gitCommand(`git commit -m "${message}"`);
  console.log('✅ 代码已提交');
  return true;
}

/**
 * 推送代码到远程
 */
function pushToRemote(branch = CONFIG.branch) {
  console.log(`🚀 推送到远程分支 ${branch}...`);
  gitCommand(`git push origin ${branch}`);
  console.log('✅ 代码已推送');
}

/**
 * 获取最新的提交 SHA
 */
function getLatestCommitSha() {
  return gitCommand('git rev-parse HEAD');
}

/**
 * GitHub API 请求
 */
function githubApiRequest(endpoint, method = 'GET') {
  return new Promise((resolve, reject) => {
    const options = {
      hostname: 'api.github.com',
      path: endpoint,
      method: method,
      headers: {
        'User-Agent': 'Auto-Build-Script',
        'Accept': 'application/vnd.github.v3+json',
        ...(CONFIG.githubToken && { 'Authorization': `token ${CONFIG.githubToken}` }),
      },
    };

    const req = https.request(options, (res) => {
      let data = '';
      res.on('data', (chunk) => { data += chunk; });
      res.on('end', () => {
        try {
          const json = JSON.parse(data);
          if (res.statusCode >= 200 && res.statusCode < 300) {
            resolve(json);
          } else {
            reject(new Error(`GitHub API error: ${res.statusCode} - ${JSON.stringify(json)}`));
          }
        } catch (e) {
          reject(new Error(`Failed to parse response: ${e.message}`));
        }
      });
    });

    req.on('error', reject);
    req.end();
  });
}

/**
 * 获取工作流程 ID
 */
async function getWorkflowId() {
  try {
    const workflows = await githubApiRequest(
      `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/workflows`
    );
    const workflow = workflows.workflows.find(w => w.name === CONFIG.workflowName);
    if (!workflow) {
      throw new Error(`Workflow "${CONFIG.workflowName}" not found`);
    }
    return workflow.id;
  } catch (error) {
    console.warn('⚠️  无法获取工作流程 ID，将使用备用方法');
    return null;
  }
}

/**
 * 获取最新的工作流程运行
 */
async function getLatestWorkflowRun(workflowId = null) {
  try {
    let endpoint = `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/runs?branch=${CONFIG.branch}&per_page=1`;
    if (workflowId) {
      endpoint = `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/workflows/${workflowId}/runs?per_page=1`;
    }
    
    const response = await githubApiRequest(endpoint);
    if (response.workflow_runs && response.workflow_runs.length > 0) {
      return response.workflow_runs[0];
    }
    return null;
  } catch (error) {
    console.warn('⚠️  无法获取工作流程运行:', error.message);
    return null;
  }
}

/**
 * 获取工作流程运行详情
 */
async function getWorkflowRunDetails(runId) {
  try {
    return await githubApiRequest(
      `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/runs/${runId}`
    );
  } catch (error) {
    console.warn('⚠️  无法获取运行详情:', error.message);
    return null;
  }
}

/**
 * 检查构建状态
 */
async function checkBuildStatus(commitSha, workflowId = null) {
  console.log('📊 检查构建状态...');
  
  const run = await getLatestWorkflowRun(workflowId);
  if (!run) {
    return { status: 'not_found', message: '未找到工作流程运行' };
  }

  // 检查是否是我们的提交
  if (run.head_sha !== commitSha) {
    return { status: 'waiting', message: '等待新的运行开始...' };
  }

  const status = run.status; // queued, in_progress, completed
  const conclusion = run.conclusion; // success, failure, cancelled, etc.

  if (status === 'completed') {
    if (conclusion === 'success') {
      return { status: 'success', runId: run.id, run };
    } else if (conclusion === 'failure') {
      return { status: 'failure', runId: run.id, run };
    } else {
      return { status: 'unknown', conclusion, runId: run.id, run };
    }
  } else {
    return { status: 'running', runId: run.id, run };
  }
}

/**
 * 获取构建日志（简化版 - 实际需要通过 jobs 和 steps 获取）
 */
async function getBuildLogs(runId) {
  try {
    // 获取运行的所有 jobs
    const jobs = await githubApiRequest(
      `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/runs/${runId}/jobs`
    );
    
    let logs = '';
    for (const job of jobs.jobs || []) {
      if (job.conclusion === 'failure') {
        logs += `\n=== Job: ${job.name} ===\n`;
        logs += `Status: ${job.status}, Conclusion: ${job.conclusion}\n`;
        // 注意：实际日志需要通过 steps 获取，这里只显示基本信息
      }
    }
    
    return logs || '无法获取详细日志，请查看 GitHub Actions 页面';
  } catch (error) {
    return `无法获取日志: ${error.message}`;
  }
}

/**
 * 等待构建完成
 */
async function waitForBuildCompletion(commitSha, workflowId = null) {
  console.log('⏳ 等待 GitHub Actions 构建完成...');
  const startTime = Date.now();
  let lastStatus = null;

  while (Date.now() - startTime < CONFIG.maxWaitTime) {
    const result = await checkBuildStatus(commitSha, workflowId);
    
    if (result.status !== lastStatus) {
      console.log(`📊 状态: ${result.status}`);
      lastStatus = result.status;
    }

    if (result.status === 'success') {
      console.log('✅ 构建成功！');
      return { success: true, runId: result.runId };
    } else if (result.status === 'failure') {
      console.log('❌ 构建失败！');
      const logs = await getBuildLogs(result.runId);
      return { success: false, runId: result.runId, logs, run: result.run };
    } else if (result.status === 'running' || result.status === 'waiting') {
      // 继续等待
      await new Promise(resolve => setTimeout(resolve, CONFIG.checkInterval));
    } else {
      await new Promise(resolve => setTimeout(resolve, CONFIG.checkInterval));
    }
  }

  throw new Error('构建超时');
}

/**
 * 主函数 - 自动化构建直到成功
 */
async function main() {
  const args = process.argv.slice(2);
  const initialCommitMessage = args[0] || 'Auto commit: 初始提交';
  let attempt = 0;

  try {
    console.log('🚀 开始完全自动化构建流程...\n');
    console.log('📌 目标：自动修复直到构建成功\n');

    // 获取工作流程 ID
    const workflowId = await getWorkflowId();
    if (workflowId) {
      console.log(`✅ 找到工作流程 ID: ${workflowId}\n`);
    }

    while (attempt < CONFIG.maxRetries) {
      attempt++;
      console.log(`\n${'='.repeat(60)}`);
      console.log(`🔄 尝试 #${attempt}/${CONFIG.maxRetries}`);
      console.log('='.repeat(60));

      // 1. 检查并提交代码
      const commitMessage = attempt === 1 
        ? initialCommitMessage 
        : `Auto commit: 修复构建错误 (尝试 ${attempt})`;
      
      const hasChanges = commitChanges(commitMessage);
      
      if (!hasChanges && attempt === 1) {
        console.log('ℹ️  没有需要提交的更改');
        break;
      }

      // 2. 推送代码
      if (hasChanges) {
        pushToRemote();
      }

      // 3. 获取提交 SHA
      const commitSha = getLatestCommitSha();
      console.log(`📌 提交 SHA: ${commitSha.substring(0, 7)}...\n`);

      // 4. 等待构建完成
      try {
        const buildResult = await waitForBuildCompletion(commitSha, workflowId);
        
        if (buildResult.success) {
          console.log('\n🎉 构建成功！任务完成！');
          console.log(`✅ 运行 ID: ${buildResult.runId}`);
          process.exit(0);
        } else {
          console.log('\n❌ 构建失败！');
          console.log(`📋 运行 ID: ${buildResult.runId}`);
          console.log('\n📝 错误信息:');
          console.log(buildResult.logs || '请查看 GitHub Actions 页面获取详细错误');
          
          if (attempt < CONFIG.maxRetries) {
            console.log('\n💡 提示: 请在 Cursor 中使用 AI 分析错误并修复代码');
            console.log('   然后再次运行此脚本，或使用 Cursor 的自动化工作流');
            console.log('\n   在 Cursor 中询问:');
            console.log('   "根据这个构建错误修复代码：[粘贴错误信息]"');
            console.log('   "分析 GitHub Actions 构建失败的原因并修复"');
            
            // 等待用户修复
            console.log('\n⏸️  等待代码修复...');
            console.log('   修复后，按 Enter 继续，或 Ctrl+C 退出');
            
            // 在实际使用中，这里可以集成 Cursor AI 自动修复
            // 目前需要手动修复
            await new Promise(resolve => {
              process.stdin.once('data', resolve);
            });
          }
        }
      } catch (error) {
        console.error('❌ 检查构建状态时出错:', error.message);
        if (attempt < CONFIG.maxRetries) {
          console.log('⏸️  等待后重试...');
          await new Promise(resolve => setTimeout(resolve, CONFIG.checkInterval));
        }
      }
    }

    console.log('\n⚠️  达到最大重试次数，构建仍未成功');
    console.log('💡 请手动检查错误并修复');
    process.exit(1);
  } catch (error) {
    console.error('\n❌ 错误:', error.message);
    process.exit(1);
  }
}

// 运行主函数
if (require.main === module) {
  // 检查 GitHub Token
  if (!CONFIG.githubToken) {
    console.warn('⚠️  警告: 未设置 GITHUB_TOKEN 环境变量');
    console.warn('   某些功能可能无法使用，建议设置:');
    console.warn('   export GITHUB_TOKEN=your_token_here');
    console.warn('');
  }
  
  main();
}

module.exports = { main, checkBuildStatus, waitForBuildCompletion };

