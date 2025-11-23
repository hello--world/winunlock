#!/usr/bin/env node

/**
 * 完全自动化构建系统
 * 
 * 功能：
 * 1. 自动提交代码
 * 2. 自动推送
 * 3. 自动检查构建状态
 * 4. 如果失败，自动获取错误日志
 * 5. 自动分析并修复错误（通过 AI）
 * 6. 自动提交修复
 * 7. 重复直到成功
 */

import { execSync } from 'child_process';
import https from 'https';
import fs from 'fs';
import { checkBuildStatus } from './check-build-status.js';

const CONFIG = {
  owner: 'hello--world',
  repo: 'winunlock',
  branch: 'dev',
  maxRetries: 10,
  checkInterval: 30000,
  maxWaitTime: 600000,
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
 * 等待构建完成
 */
async function waitForBuildCompletion(commitSha) {
  console.log('⏳ 等待构建完成...');
  const startTime = Date.now();
  let lastStatus = null;

  while (Date.now() - startTime < CONFIG.maxWaitTime) {
    const result = await checkBuildStatus();
    
    if (!result.run || result.run.head_sha !== commitSha) {
      await new Promise(resolve => setTimeout(resolve, CONFIG.checkInterval));
      continue;
    }

    if (result.status !== lastStatus) {
      console.log(`📊 状态: ${result.status || (result.success === null ? '进行中' : result.success ? '成功' : '失败')}`);
      lastStatus = result.status;
    }

    if (result.success === true) {
      return { success: true, runId: result.run.id };
    } else if (result.success === false) {
      return { success: false, runId: result.run.id, run: result.run };
    }

    await new Promise(resolve => setTimeout(resolve, CONFIG.checkInterval));
  }

  throw new Error('构建超时');
}

/**
 * 获取构建日志（简化版）
 */
async function getBuildLogs(runId) {
  // 这里返回提示，实际日志需要通过 GitHub MCP Server 获取
  return {
    message: `构建失败，运行 ID: ${runId}`,
    runId: runId,
    url: `https://github.com/${CONFIG.owner}/${CONFIG.repo}/actions/runs/${runId}`
  };
}

/**
 * 分析错误并生成修复建议
 */
function analyzeError(errorLog) {
  // 简单的错误模式匹配
  const errorPatterns = [
    {
      pattern: /missing.*header.*file|找不到.*头文件/i,
      fix: '添加缺失的头文件包含',
      example: '#include <missing_header.h>'
    },
    {
      pattern: /undefined.*reference|未定义的引用/i,
      fix: '检查链接库和函数定义',
      example: '检查 CMakeLists.txt 中的链接库'
    },
    {
      pattern: /syntax.*error|语法错误/i,
      fix: '检查语法错误',
      example: '检查括号、分号等'
    },
    {
      pattern: /CMake.*error|CMake.*failed/i,
      fix: '检查 CMake 配置',
      example: '检查 CMakeLists.txt 配置'
    }
  ];

  for (const pattern of errorPatterns) {
    if (pattern.pattern.test(errorLog)) {
      return {
        detected: true,
        fix: pattern.fix,
        example: pattern.example
      };
    }
  }

  return {
    detected: false,
    message: '需要手动分析错误'
  };
}

/**
 * 自动修复错误（基础版本）
 */
async function autoFixError(errorInfo) {
  console.log('\n🔧 尝试自动修复错误...');
  
  // 这里应该调用 Cursor AI 来修复
  // 目前返回提示信息
  console.log('💡 请在 Cursor 中使用以下命令自动修复：');
  console.log(`   "下载工作流程运行 #${errorInfo.runId} 的日志"`);
  console.log(`   "根据这个构建错误修复代码"`);
  console.log(`   "分析并修复构建错误"`);
  
  return {
    fixed: false,
    message: '需要 AI 辅助修复'
  };
}

/**
 * 主函数 - 完全自动化
 */
async function main() {
  const args = process.argv.slice(2);
  const initialMessage = args[0] || 'Auto commit: 自动构建修复';
  let attempt = 0;

  console.log('🚀 启动完全自动化构建系统...\n');
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
      // 即使没有更改，也检查现有构建状态
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
      const buildResult = await waitForBuildCompletion(commitSha);
      
      if (buildResult.success) {
        console.log('\n🎉 构建成功！任务完成！');
        console.log(`✅ 运行 ID: ${buildResult.runId}`);
        console.log(`📦 应用已构建，可在 GitHub Actions 下载`);
        process.exit(0);
      } else {
        console.log('\n❌ 构建失败！');
        console.log(`📋 运行 ID: ${buildResult.runId}`);
        
        // 5. 获取错误信息
        const errorInfo = await getBuildLogs(buildResult.runId);
        console.log(`\n📝 错误信息: ${errorInfo.message}`);
        console.log(`🔗 查看详情: ${errorInfo.url}`);
        
        // 6. 分析错误
        const analysis = analyzeError(errorInfo.message);
        if (analysis.detected) {
          console.log(`\n🔍 检测到错误类型: ${analysis.fix}`);
          console.log(`💡 建议: ${analysis.example}`);
        }
        
        // 7. 尝试自动修复
        if (attempt < CONFIG.maxRetries) {
          console.log('\n🤖 启动 AI 自动修复...');
          const fixResult = await autoFixError(errorInfo);
          
          if (!fixResult.fixed) {
            console.log('\n⏸️  需要 AI 辅助修复');
            console.log('💡 请在 Cursor 中使用以下命令：');
            console.log(`   "下载工作流程运行 #${buildResult.runId} 的日志"`);
            console.log(`   "根据这个构建错误修复代码"`);
            console.log('\n   修复后，按 Enter 继续，或 Ctrl+C 退出');
            
            // 等待用户修复
            await new Promise(resolve => {
              process.stdin.once('data', resolve);
            });
          }
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
}

// 运行主函数
if (import.meta.url === `file://${process.argv[1]}` || process.argv[1]?.endsWith('fully-auto-build.js')) {
  if (!CONFIG.githubToken) {
    console.warn('⚠️  警告: 未设置 GITHUB_TOKEN 环境变量');
    console.warn('   某些功能可能无法使用');
    console.warn('');
  }
  
  main().catch(error => {
    console.error('❌ 错误:', error.message);
    process.exit(1);
  });
}

export { main };

