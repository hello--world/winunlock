#!/usr/bin/env node

/**
 * 等待构建完成并检查结果
 */

import { checkBuildStatus } from './check-build-status.js';

const CHECK_INTERVAL = 30000; // 30秒
const MAX_WAIT_TIME = 600000; // 10分钟

async function waitForBuild() {
  console.log('⏳ 等待构建完成...\n');
  const startTime = Date.now();
  let lastStatus = null;

  while (Date.now() - startTime < MAX_WAIT_TIME) {
    const result = await checkBuildStatus();
    
    if (result.status !== lastStatus) {
      console.log(`\n📊 状态更新: ${result.status || result.success === null ? '进行中' : result.success ? '成功' : '失败'}`);
      lastStatus = result.status;
    }

    if (result.success === true) {
      console.log('\n🎉 构建成功！任务完成！');
      return { success: true };
    } else if (result.success === false) {
      console.log('\n❌ 构建失败！');
      console.log(`\n💡 请在 Cursor 中使用以下命令获取错误日志并修复：`);
      console.log(`   "下载工作流程运行 #${result.run.id} 的日志"`);
      console.log(`   "根据这个构建错误修复代码"`);
      return { success: false, runId: result.run.id };
    } else if (result.status === 'running' || result.success === null) {
      console.log(`\n⏳ 等待 ${CHECK_INTERVAL / 1000} 秒后再次检查...`);
      await new Promise(resolve => setTimeout(resolve, CHECK_INTERVAL));
    } else {
      await new Promise(resolve => setTimeout(resolve, CHECK_INTERVAL));
    }
  }

  console.log('\n⚠️  等待超时');
  return { success: null, timeout: true };
}

if (import.meta.url === `file://${process.argv[1]}` || process.argv[1]?.endsWith('wait-and-check.js')) {
  waitForBuild().then(result => {
    if (result.success === false) {
      process.exit(1);
    } else if (result.success === true) {
      process.exit(0);
    } else {
      process.exit(2);
    }
  });
}

export { waitForBuild };

