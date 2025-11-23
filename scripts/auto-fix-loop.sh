#!/bin/bash

# 完全自动化修复循环脚本

MAX_ATTEMPTS=10
ATTEMPT=0
LATEST_SHA=""

echo "🚀 启动完全自动化修复循环"
echo "📌 目标：自动修复直到构建成功"
echo ""

while [ $ATTEMPT -lt $MAX_ATTEMPTS ]; do
  ATTEMPT=$((ATTEMPT + 1))
  
  echo "=========================================="
  echo "🔄 尝试 #$ATTEMPT/$MAX_ATTEMPTS"
  echo "=========================================="
  
  # 等待构建完成
  echo "⏳ 等待构建完成..."
  sleep 60
  
  # 检查构建状态
  echo "📊 检查构建状态..."
  node scripts/check-workflow.js 2>&1 | tail -30
  
  # 获取最新提交 SHA
  LATEST_SHA=$(git rev-parse HEAD)
  echo ""
  echo "📌 当前提交: ${LATEST_SHA:0:7}"
  echo ""
  
  # 检查是否有新的失败构建
  echo "💡 如果构建失败，请在 Cursor 中使用："
  echo '   "下载最新失败的工作流程运行的日志"'
  echo '   "根据构建错误自动修复代码"'
  echo ""
  
  read -p "构建成功了吗？(y/n/q): " success
  
  if [ "$success" = "y" ]; then
    echo ""
    echo "🎉 构建成功！任务完成！"
    exit 0
  elif [ "$success" = "q" ]; then
    echo ""
    echo "👋 退出"
    exit 0
  else
    echo ""
    echo "❌ 构建失败，继续修复..."
    echo ""
  fi
done

echo ""
echo "⚠️  达到最大重试次数"
exit 1

