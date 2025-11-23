#!/usr/bin/env node

/**
 * 完全自动化构建脚本：提交 → 推送 → 等待构建 → 检查结果 → 自动修复
 */

import { execSync } from 'child_process';
import { waitForBuild } from './wait-for-build.js';

const CONFIG = {
  branch: 'dev',
  maxRetries: 10,
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
function commitChanges(message) {
  if (!hasUncommittedChanges()) {
    return false;
  }
  gitCommand('git add -A');
  gitCommand(`git commit -m "${message}"`);
  return true;
}

/**
 * 推送代码
 */
function pushToRemote() {
  gitCommand(`git push origin ${CONFIG.branch}`);
}

/**
 * 获取最新提交 SHA
 */
function getLatestCommitSha() {
  return gitCommand('git rev-parse HEAD');
}

/**
 * 主函数 - 完全自动化
 */
async function main() {
  const args = process.argv.slice(2);
  const initialMessage = args[0] || 'Auto commit: 自动构建修复';
  let attempt = 0;

  console.log('🚀 启动完全自动化构建流程...\n');
  console.log('📌 目标：自动修复直到构建成功\n');

  while (attempt < CONFIG.maxRetries) {
    attempt++;
    console.log(`\n${'='.repeat(60)}`);
    console.log(`🔄 尝试 #${attempt}/${CONFIG.maxRetries}`);
    console.log('='.repeat(60));

    // 1. 检查并提交代码
    const commitMessage = attempt === 1 
      ? initialMessage 
      : `Auto fix: 修复构建错误 (尝试 ${attempt})`;
    
    const hasChanges = commitChanges(commitMessage);
    
    if (!hasChanges && attempt === 1) {
      console.log('ℹ️  没有需要提交的更改');
    }

    // 2. 推送代码（如果有更改）
    if (hasChanges) {
      console.log('📝 提交更改...');
      console.log('🚀 推送到远程...');
      pushToRemote();
      console.log('✅ 代码已推送\n');
    }

    // 3. 获取提交 SHA
    const commitSha = getLatestCommitSha();
    console.log(`📌 提交 SHA: ${commitSha.substring(0, 7)}...\n`);

    // 4. 等待构建完成
    try {
      const buildResult = await waitForBuild(commitSha, {
        onStatusChange: (status, result) => {
          if (status === 'running') {
            console.log(`  运行中... (${new Date().toLocaleTimeString()})`);
          }
        }
      });
      
      if (buildResult.success) {
        console.log('\n🎉 构建成功！任务完成！');
        console.log(`✅ 运行 ID: ${buildResult.runId}`);
        console.log(`📦 应用已构建，可在 GitHub Actions 下载`);
        console.log(`🔗 URL: ${buildResult.run.html_url}`);
        process.exit(0);
      } else {
        console.log('\n❌ 构建失败！');
        console.log(`📋 运行 ID: ${buildResult.runId}`);
        console.log(`🔗 查看错误: ${buildResult.errorUrl}`);
        
        if (attempt < CONFIG.maxRetries) {
          console.log('\n💡 请在 Cursor 中使用以下命令获取错误日志并修复：');
          console.log(`   "下载工作流程运行 #${buildResult.runId} 的日志"`);
          console.log(`   "根据这个构建错误修复代码"`);
          console.log('\n   修复后，按 Enter 继续，或 Ctrl+C 退出');
          
          // 等待用户修复
          await new Promise(resolve => {
            process.stdin.once('data', resolve);
          });
        }
      }
    } catch (error) {
      console.error('❌ 检查构建状态时出错:', error.message);
      if (attempt < CONFIG.maxRetries) {
        console.log('⏸️  等待后重试...');
        await new Promise(resolve => setTimeout(resolve, 30000));
      }
    }
  }

  console.log('\n⚠️  达到最大重试次数，构建仍未成功');
  console.log('💡 请手动检查错误并修复');
  process.exit(1);
}

// 运行主函数
if (import.meta.url === `file://${process.argv[1]}` || process.argv[1]?.endsWith('auto-build-and-wait.js')) {
  main().catch(error => {
    console.error('❌ 错误:', error.message);
    process.exit(1);
  });
}

export { main };

