#!/bin/bash

# 自动启动完全自动化构建流程

echo "🚀 启动完全自动化构建流程..."
echo ""

# 检查是否有未提交的更改
if [ -n "$(git status --porcelain)" ]; then
  echo "📝 发现未提交的更改，正在提交..."
  git add -A
  git commit -m "Auto commit: $(date +%Y%m%d_%H%M%S)"
  echo "✅ 代码已提交"
fi

# 推送到远程
echo "🚀 推送到远程分支..."
git push origin dev
echo "✅ 代码已推送"
echo ""

# 提示用户在 Cursor 中使用自动化
echo "💡 现在请在 Cursor 聊天中输入以下命令："
echo ""
echo "   '自动修复构建错误直到成功'"
echo ""
echo "或者："
echo ""
echo "   '帮我实现完全自动化构建，自动检查构建状态，如果失败就分析错误并修复，重复直到编译成功'"
echo ""
echo "AI 会自动："
echo "  ✅ 检查构建状态"
echo "  ✅ 获取错误日志（如果失败）"
echo "  ✅ 分析并修复错误"
echo "  ✅ 提交并推送修复"
echo "  ✅ 重复直到成功"
echo ""

