#!/bin/bash
# Git 仓库初始化脚本

echo "初始化 Git 仓库..."

# 检查是否已经是 Git 仓库
if [ -d .git ]; then
    echo "已经是 Git 仓库"
else
    git init
    echo "Git 仓库初始化完成"
fi

# 添加远程仓库（如果还没有）
if ! git remote | grep -q origin; then
    echo "添加远程仓库..."
    git remote add origin git@github.com:hello--world/winunlock.git
    echo "远程仓库已添加: git@github.com:hello--world/winunlock.git"
else
    echo "远程仓库已存在"
    git remote set-url origin git@github.com:hello--world/winunlock.git
    echo "远程仓库 URL 已更新"
fi

# 添加所有文件
echo "添加文件到 Git..."
git add .

# 显示状态
echo ""
echo "当前 Git 状态:"
git status

echo ""
echo "下一步操作:"
echo "1. 检查 SSH 密钥是否已配置: ssh -T git@github.com"
echo "2. 提交更改: git commit -m 'Initial commit: WinUnlock credential provider with Tauri config tool'"
echo "3. 推送到 GitHub: git push -u origin main"
echo ""
echo "如果主分支是 master 而不是 main，请使用: git push -u origin master"

