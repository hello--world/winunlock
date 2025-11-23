#!/bin/bash

# 快速自动化修复脚本
# 使用 Cursor AI 自动修复构建错误直到成功

set -e

BRANCH="dev"
MAX_ATTEMPTS=10
ATTEMPT=0

echo "🚀 开始完全自动化构建修复流程..."
echo "📌 目标：自动修复直到构建成功"
echo ""

while [ $ATTEMPT -lt $MAX_ATTEMPTS ]; do
  ATTEMPT=$((ATTEMPT + 1))
  
  echo "=========================================="
  echo "🔄 尝试 #$ATTEMPT/$MAX_ATTEMPTS"
  echo "=========================================="
  
  # 1. 检查是否有未提交的更改
  if [ -n "$(git status --porcelain)" ]; then
    echo "📝 提交更改..."
    git add -A
    git commit -m "Auto fix: 修复构建错误 (尝试 $ATTEMPT)"
    echo "✅ 代码已提交"
  else
    echo "✅ 没有需要提交的更改"
    if [ $ATTEMPT -eq 1 ]; then
      echo "ℹ️  没有更改，退出"
      exit 0
    fi
  fi
  
  # 2. 推送代码
  echo "🚀 推送到远程分支 $BRANCH..."
  git push origin $BRANCH
  echo "✅ 代码已推送"
  
  # 3. 获取提交 SHA
  COMMIT_SHA=$(git rev-parse HEAD)
  echo "📌 提交 SHA: ${COMMIT_SHA:0:7}..."
  echo ""
  
  # 4. 提示用户在 Cursor 中检查
  echo "⏳ 等待 GitHub Actions 构建..."
  echo ""
  echo "💡 请在 Cursor 中执行以下命令检查构建状态："
  echo ""
  echo "   1. '列出 Build WinUnlock 工作流程的最新运行'"
  echo "   2. '如果构建失败，下载最新失败的工作流程运行的日志'"
  echo "   3. '根据这个构建错误修复代码：[粘贴错误]'"
  echo "   4. 修复后，按 Enter 继续，或输入 'q' 退出"
  echo ""
  
  read -p "按 Enter 继续检查，或输入 'q' 退出: " user_input
  
  if [ "$user_input" = "q" ]; then
    echo "👋 退出"
    exit 0
  fi
  
  # 5. 询问构建是否成功
  read -p "构建成功了吗？(y/n): " success
  
  if [ "$success" = "y" ]; then
    echo ""
    echo "🎉 构建成功！任务完成！"
    exit 0
  else
    echo ""
    echo "❌ 构建失败，继续尝试修复..."
    echo ""
  fi
done

echo ""
echo "⚠️  达到最大重试次数，构建仍未成功"
echo "💡 请手动检查错误并修复"
exit 1

