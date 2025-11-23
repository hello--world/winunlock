#!/usr/bin/env node

/**
 * 完全自动化构建脚本：提交 → 推送 → 等待构建 → 检查结果 → 自动修复
 * 增强版：自动获取错误日志、提取错误信息、保存到文件
 */

import { execSync } from 'child_process';
import fs from 'fs';
import { waitForBuild } from './wait-for-build.js';
import { extractBuildErrors } from './extract-build-errors.js';

const CONFIG = {
  branch: 'dev',
  maxRetries: 10,
  errorFile: '.cursor/build-errors.txt',
  autoMode: false, // 完全自动化模式（无需用户交互）
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
 * 保存错误信息到文件
 */
async function saveBuildErrors(buildResult) {
  try {
    // 确保目录存在
    const errorDir = CONFIG.errorFile.substring(0, CONFIG.errorFile.lastIndexOf('/'));
    if (errorDir && !fs.existsSync(errorDir)) {
      fs.mkdirSync(errorDir, { recursive: true });
    }

    let errorContent = '';
    errorContent += '='.repeat(60) + '\n';
    errorContent += `构建失败报告 - 运行 #${buildResult.runId}\n`;
    errorContent += `时间: ${new Date().toLocaleString()}\n`;
    errorContent += `URL: ${buildResult.errorUrl}\n`;
    errorContent += '='.repeat(60) + '\n\n';

    // 如果有日志，提取错误
    if (buildResult.errorSummary && buildResult.errorSummary.logText) {
      try {
        const errorReport = await extractBuildErrors(buildResult.errorSummary.logText, {
          includeContext: true,
          maxErrors: 50
        });
        
        errorContent += formatErrorReport(errorReport);
      } catch (error) {
        console.warn(`⚠️  提取错误时出错: ${error.message}`);
        // 如果提取失败，直接保存原始日志
        errorContent += '原始日志:\n';
        errorContent += '-'.repeat(60) + '\n';
        errorContent += buildResult.errorSummary.logText.substring(0, 10000);
        if (buildResult.errorSummary.logText.length > 10000) {
          errorContent += '\n... (日志已截断，完整日志请查看 GitHub Actions)';
        }
      }
    } else {
      errorContent += '无法获取详细日志。请访问 GitHub Actions 查看完整错误信息。\n';
      errorContent += `URL: ${buildResult.errorUrl}\n`;
    }

    fs.writeFileSync(CONFIG.errorFile, errorContent);
    console.log(`✅ 错误信息已保存到: ${CONFIG.errorFile}`);
    return true;
  } catch (error) {
    console.error(`❌ 保存错误信息时出错: ${error.message}`);
    return false;
  }
}

/**
 * 格式化错误报告
 */
function formatErrorReport(report) {
  let output = '';
  
  output += '错误摘要:\n';
  output += `  总错误数: ${report.summary.totalErrors}\n`;
  output += `  总警告数: ${report.summary.totalWarnings}\n`;
  output += `  错误类型数: ${report.summary.errorTypes}\n\n`;

  // 显示关键错误
  if (report.errors.length > 0) {
    output += '关键错误:\n';
    output += '-'.repeat(60) + '\n';
    
    report.errors.slice(0, 20).forEach((error, index) => {
      output += `${index + 1}. [${error.type}] `;
      if (error.file) {
        output += `${error.file}`;
        if (error.line) {
          output += `:${error.line}`;
        }
        output += ' - ';
      }
      output += `${error.message}\n`;
    });
    
    if (report.errors.length > 20) {
      output += `... 还有 ${report.errors.length - 20} 个错误\n`;
    }
    output += '\n';
  }

  // 显示错误上下文
  if (report.context.length > 0) {
    output += '错误上下文:\n';
    output += '-'.repeat(60) + '\n';
    
    report.context.slice(0, 30).forEach(ctx => {
      output += `${ctx.lineNumber.toString().padStart(4, ' ')} | ${ctx.content}\n`;
    });
    
    if (report.context.length > 30) {
      output += `... 还有 ${report.context.length - 30} 行上下文\n`;
    }
    output += '\n';
  }

  return output;
}

/**
 * 主函数 - 完全自动化
 */
async function main() {
  const args = process.argv.slice(2);
  const initialMessage = args[0] || 'Auto commit: 自动构建修复';
  const autoMode = args.includes('--auto') || args.includes('-a');
  let attempt = 0;

  CONFIG.autoMode = autoMode;

  console.log('🚀 启动完全自动化构建流程...\n');
  console.log('📌 目标：自动修复直到构建成功\n');
  if (autoMode) {
    console.log('🤖 完全自动化模式：将自动循环直到成功\n');
  }

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
        
        // 自动获取并保存错误信息
        if (buildResult.errorSummary) {
          console.log(`\n📥 正在提取错误信息...`);
          await saveBuildErrors(buildResult);
        }
        
        if (attempt < CONFIG.maxRetries) {
          console.log('\n' + '='.repeat(60));
          console.log('💡 修复指南:');
          console.log('='.repeat(60));
          console.log(`\n1. 错误信息已保存到: ${CONFIG.errorFile}`);
          console.log(`\n2. 在 Cursor 中使用以下命令修复：`);
          console.log(`   "根据 .cursor/build-errors.txt 中的错误修复代码"`);
          console.log(`   或`);
          console.log(`   "分析这个构建错误并自动修复：[粘贴错误信息]"`);
          console.log(`\n3. 修复后：`);
          
          if (CONFIG.autoMode) {
            console.log(`   🤖 自动模式：将在 10 秒后自动继续...`);
            console.log(`   （按 Ctrl+C 可随时退出）\n`);
            await new Promise(resolve => setTimeout(resolve, 10000));
          } else {
            console.log(`   按 Enter 继续，或 Ctrl+C 退出\n`);
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

