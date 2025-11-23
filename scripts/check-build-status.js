#!/usr/bin/env node

/**
 * 快速检查构建状态脚本
 */

import https from 'https';

const CONFIG = {
  owner: 'hello--world',
  repo: 'winunlock',
  branch: 'dev',
  githubToken: process.env.GITHUB_TOKEN || '',
};

function githubApiRequest(endpoint) {
  return new Promise((resolve, reject) => {
    const options = {
      hostname: 'api.github.com',
      path: endpoint,
      method: 'GET',
      headers: {
        'User-Agent': 'Build-Check-Script',
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
            reject(new Error(`GitHub API error: ${res.statusCode}`));
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

async function checkBuildStatus() {
  try {
    console.log('📊 检查构建状态...\n');
    
    const endpoint = `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/runs?branch=${CONFIG.branch}&per_page=1`;
    const response = await githubApiRequest(endpoint);
    
    if (response.workflow_runs && response.workflow_runs.length > 0) {
      const run = response.workflow_runs[0];
      console.log(`运行 ID: ${run.id}`);
      console.log(`状态: ${run.status}`);
      console.log(`结论: ${run.conclusion || '进行中'}`);
      console.log(`提交 SHA: ${run.head_sha.substring(0, 7)}`);
      console.log(`工作流程: ${run.name}`);
      console.log(`创建时间: ${new Date(run.created_at).toLocaleString()}`);
      console.log(`URL: ${run.html_url}\n`);
      
      if (run.status === 'completed') {
        if (run.conclusion === 'success') {
          console.log('✅ 构建成功！');
          return { success: true, run };
        } else {
          console.log('❌ 构建失败！');
          return { success: false, run };
        }
      } else {
        console.log('⏳ 构建进行中...');
        return { success: null, run, status: 'running' };
      }
    } else {
      console.log('⚠️  未找到工作流程运行');
      return { success: null, run: null };
    }
  } catch (error) {
    console.error('❌ 检查构建状态时出错:', error.message);
    if (error.message.includes('401') || error.message.includes('403')) {
      console.log('\n💡 提示: 可能需要设置 GITHUB_TOKEN 环境变量');
      console.log('   export GITHUB_TOKEN=your_token');
    }
    return { success: null, error: error.message };
  }
}

// 检查是否是直接运行
if (import.meta.url === `file://${process.argv[1]}` || process.argv[1]?.endsWith('check-build-status.js')) {
  checkBuildStatus().then(result => {
    if (result.success === false) {
      console.log('\n💡 构建失败，请在 Cursor 中使用以下命令获取错误日志：');
      console.log('   "下载工作流程运行 #' + result.run.id + ' 的日志"');
      console.log('   "根据构建错误修复代码"');
      process.exit(1);
    } else if (result.success === true) {
      process.exit(0);
    } else if (result.status === 'running') {
      console.log('\n💡 构建进行中，请稍后再次检查');
      process.exit(2);
    }
  });
}

export { checkBuildStatus };

