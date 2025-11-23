#!/usr/bin/env node

/**
 * 自动化构建检查脚本
 * 
 * 功能：
 * 1. 提交代码到 GitHub
 * 2. 创建 Pull Request
 * 3. 检查 GitHub Actions 构建状态
 * 4. 获取构建错误信息
 * 5. 自动修正错误（通过 Cursor AI）
 */

const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

// 配置
const CONFIG = {
  owner: 'hello--world',
  repo: 'winunlock',
  branch: 'dev',
  baseBranch: 'main',
  maxRetries: 3,
  checkInterval: 30000, // 30秒检查一次
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
function commitChanges(message = 'Auto commit from Cursor') {
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
 * 等待 GitHub Actions 完成
 * 
 * 注意：实际检查构建状态应该使用 GitHub MCP Server 的官方功能：
 * - list_workflows - 列出所有工作流程
 * - list_workflow_runs - 列出工作流程运行
 * - get_workflow_run - 获取工作流程运行详情
 * - get_workflow_run_logs - 下载工作流程运行日志
 * 
 * 在 Cursor 中直接使用这些命令，例如：
 * "列出 winunlock 仓库的所有工作流程"
 * "列出 Build WinUnlock 工作流程的运行"
 * "获取工作流程运行 #123 的详情和日志"
 */
async function waitForBuildCompletion(commitSha, timeout = 600000) {
  console.log('⏳ 等待 GitHub Actions 构建完成...');
  console.log('💡 提示: 在 Cursor 中使用 GitHub MCP Server 检查构建状态:');
  console.log('   "列出 Build WinUnlock 工作流程的运行"');
  console.log('   "获取最新工作流程运行的详情"');
  console.log('   "下载工作流程运行 #123 的日志"');
  
  // 实际检查应该通过 GitHub MCP Server 完成
  // 这里只是提示用户
  await new Promise(resolve => setTimeout(resolve, 2000));
  
  return { status: 'check_manually', message: '请在 Cursor 中使用 GitHub MCP Server 检查' };
}

/**
 * 获取构建错误信息
 * 
 * 注意：使用 GitHub MCP Server 的 get_workflow_run_logs 功能
 * 在 Cursor 中询问："下载工作流程运行 #123 的日志"
 */
async function getBuildErrors(commitSha) {
  console.log('🔍 获取构建错误信息...');
  console.log('💡 提示: 在 Cursor 中使用 GitHub MCP Server:');
  console.log('   "下载最新工作流程运行的日志"');
  console.log('   "获取工作流程运行 #123 的详情"');
  
  return {
    hasErrors: false,
    errors: [],
    logs: '',
    message: '请在 Cursor 中使用 GitHub MCP Server 获取日志'
  };
}

/**
 * 主函数
 */
async function main() {
  const args = process.argv.slice(2);
  const commitMessage = args[0] || 'Auto commit from Cursor';
  const shouldPush = !args.includes('--no-push');
  const shouldCreatePR = args.includes('--create-pr');

  try {
    console.log('🚀 开始自动化构建检查流程...\n');

    // 1. 提交代码
    const hasChanges = commitChanges(commitMessage);
    if (!hasChanges && !shouldPush) {
      console.log('ℹ️  没有更改，跳过推送');
      return;
    }

    // 2. 推送代码
    if (shouldPush && hasChanges) {
      pushToRemote();
    }

    // 3. 获取提交 SHA
    const commitSha = getLatestCommitSha();
    console.log(`📌 提交 SHA: ${commitSha}\n`);

    // 4. 创建 PR（如果需要）
    if (shouldCreatePR) {
      console.log('📋 创建 Pull Request...');
      console.log('ℹ️  请在 Cursor 中使用 GitHub MCP Server 创建 PR');
      console.log('   命令示例: "创建一个从 dev 到 main 的 PR"');
    }

    // 5. 等待构建完成
    if (shouldPush) {
      const buildResult = await waitForBuildCompletion(commitSha);
      
      if (buildResult.success) {
        console.log('\n✅ 构建成功！');
      } else {
        console.log('\n❌ 构建失败！');
        
        // 6. 获取错误信息
        const errors = await getBuildErrors(commitSha);
        if (errors.hasErrors) {
          console.log('\n📋 构建错误:');
          console.log(errors.errors.join('\n'));
          console.log('\n💡 提示: 可以在 Cursor 中请求 AI 帮助修复这些错误');
        }
      }
    }

    console.log('\n✨ 流程完成！');
  } catch (error) {
    console.error('\n❌ 错误:', error.message);
    process.exit(1);
  }
}

// 运行主函数
if (require.main === module) {
  main();
}

module.exports = { main, commitChanges, pushToRemote, waitForBuildCompletion, getBuildErrors };

