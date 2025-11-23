#!/usr/bin/env node

/**
 * 检查 GitHub Actions 工作流程运行状态
 */

import https from 'https';

const CONFIG = {
  owner: 'hello--world',
  repo: 'winunlock',
  branch: 'dev',
  workflowName: 'Build WinUnlock',
  githubToken: process.env.GITHUB_TOKEN || '',
};

function githubApiRequest(endpoint) {
  return new Promise((resolve, reject) => {
    const options = {
      hostname: 'api.github.com',
      path: endpoint,
      method: 'GET',
      headers: {
        'User-Agent': 'Workflow-Check-Script',
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

async function listWorkflows() {
  try {
    console.log('📋 列出所有工作流程...\n');
    const workflows = await githubApiRequest(
      `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/workflows`
    );
    
    if (workflows.workflows && workflows.workflows.length > 0) {
      console.log('找到以下工作流程：\n');
      workflows.workflows.forEach(workflow => {
        console.log(`  - ${workflow.name} (ID: ${workflow.id})`);
        console.log(`    状态: ${workflow.state}`);
        console.log(`    路径: ${workflow.path}\n`);
      });
      return workflows.workflows;
    } else {
      console.log('未找到工作流程');
      return [];
    }
  } catch (error) {
    console.error('❌ 无法列出工作流程:', error.message);
    if (error.message.includes('401') || error.message.includes('403')) {
      console.log('\n💡 提示: 可能需要设置 GITHUB_TOKEN 环境变量');
    }
    return [];
  }
}

async function listWorkflowRuns(workflowId = null) {
  try {
    console.log('📊 列出工作流程运行...\n');
    
    let endpoint;
    if (workflowId) {
      endpoint = `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/workflows/${workflowId}/runs?branch=${CONFIG.branch}&per_page=5`;
    } else {
      endpoint = `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/runs?branch=${CONFIG.branch}&per_page=5`;
    }
    
    const response = await githubApiRequest(endpoint);
    
    if (response.workflow_runs && response.workflow_runs.length > 0) {
      console.log(`找到 ${response.workflow_runs.length} 个运行：\n`);
      response.workflow_runs.forEach((run, index) => {
        const status = run.status;
        const conclusion = run.conclusion || '进行中';
        const statusIcon = conclusion === 'success' ? '✅' : conclusion === 'failure' ? '❌' : '⏳';
        
        console.log(`${statusIcon} 运行 #${run.id} - ${run.name}`);
        console.log(`   状态: ${status} (${conclusion})`);
        console.log(`   提交: ${run.head_sha.substring(0, 7)} - ${run.head_commit?.message || 'N/A'}`);
        console.log(`   分支: ${run.head_branch}`);
        console.log(`   创建时间: ${new Date(run.created_at).toLocaleString()}`);
        console.log(`   URL: ${run.html_url}\n`);
      });
      
      return response.workflow_runs[0]; // 返回最新的运行
    } else {
      console.log('未找到工作流程运行');
      return null;
    }
  } catch (error) {
    console.error('❌ 无法列出工作流程运行:', error.message);
    return null;
  }
}

async function getWorkflowRunDetails(runId) {
  try {
    console.log(`📋 获取工作流程运行 #${runId} 的详情...\n`);
    
    const run = await githubApiRequest(
      `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/runs/${runId}`
    );
    
    console.log('运行详情：\n');
    console.log(`  ID: ${run.id}`);
    console.log(`  名称: ${run.name}`);
    console.log(`  状态: ${run.status}`);
    console.log(`  结论: ${run.conclusion || '进行中'}`);
    console.log(`  提交 SHA: ${run.head_sha}`);
    console.log(`  分支: ${run.head_branch}`);
    console.log(`  触发者: ${run.triggering_actor?.login || 'N/A'}`);
    console.log(`  创建时间: ${new Date(run.created_at).toLocaleString()}`);
    if (run.updated_at) {
      console.log(`  更新时间: ${new Date(run.updated_at).toLocaleString()}`);
    }
    if (run.run_started_at) {
      console.log(`  开始时间: ${new Date(run.run_started_at).toLocaleString()}`);
    }
    console.log(`  运行尝试: ${run.run_attempt || 1}`);
    console.log(`  URL: ${run.html_url}\n`);
    
    // 获取 jobs
    try {
      const jobs = await githubApiRequest(
        `/repos/${CONFIG.owner}/${CONFIG.repo}/actions/runs/${runId}/jobs`
      );
      
      if (jobs.jobs && jobs.jobs.length > 0) {
        console.log('Jobs:\n');
        jobs.jobs.forEach(job => {
          const conclusion = job.conclusion || '进行中';
          const icon = conclusion === 'success' ? '✅' : conclusion === 'failure' ? '❌' : '⏳';
          console.log(`  ${icon} ${job.name}`);
          console.log(`    状态: ${job.status} (${conclusion})`);
          console.log(`    开始时间: ${new Date(job.started_at).toLocaleString()}`);
          if (job.completed_at) {
            console.log(`    完成时间: ${new Date(job.completed_at).toLocaleString()}`);
          }
          console.log(`    URL: ${job.html_url}\n`);
        });
      }
    } catch (error) {
      console.log('无法获取 jobs 详情');
    }
    
    return run;
  } catch (error) {
    console.error('❌ 无法获取运行详情:', error.message);
    return null;
  }
}

async function main() {
  console.log('🚀 检查 GitHub Actions 工作流程状态\n');
  console.log('='.repeat(60) + '\n');
  
  if (!CONFIG.githubToken) {
    console.warn('⚠️  警告: 未设置 GITHUB_TOKEN 环境变量');
    console.warn('   某些功能可能无法使用\n');
  }
  
  // 1. 列出工作流程
  const workflows = await listWorkflows();
  const buildWorkflow = workflows.find(w => w.name === CONFIG.workflowName);
  
  // 2. 列出运行
  console.log('\n' + '='.repeat(60) + '\n');
  const latestRun = await listWorkflowRuns(buildWorkflow?.id);
  
  if (latestRun) {
    // 3. 获取最新运行的详情
    console.log('\n' + '='.repeat(60) + '\n');
    await getWorkflowRunDetails(latestRun.id);
    
    // 4. 检查是否失败
    if (latestRun.conclusion === 'failure') {
      console.log('\n' + '='.repeat(60) + '\n');
      console.log('❌ 构建失败！');
      console.log('\n💡 要获取详细日志，请在 Cursor 中使用：');
      console.log(`   "下载工作流程运行 #${latestRun.id} 的日志"`);
      console.log(`   或访问: ${latestRun.html_url}`);
    } else if (latestRun.conclusion === 'success') {
      console.log('\n' + '='.repeat(60) + '\n');
      console.log('✅ 构建成功！');
    } else {
      console.log('\n' + '='.repeat(60) + '\n');
      console.log('⏳ 构建进行中...');
    }
  }
}

if (import.meta.url === `file://${process.argv[1]}` || process.argv[1]?.endsWith('check-workflow.js')) {
  main().catch(error => {
    console.error('❌ 错误:', error.message);
    process.exit(1);
  });
}

export { listWorkflows, listWorkflowRuns, getWorkflowRunDetails };

