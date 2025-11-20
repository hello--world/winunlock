#!/bin/bash

# 初始化 Git 仓库并推送到 GitHub

echo "初始化 Git 仓库..."
git init

echo "添加所有文件..."
git add .

echo "创建初始提交..."
git commit -m "Initial commit: WinUnlock 项目"

echo "创建 dev 分支..."
git checkout -b dev

echo "添加远程仓库..."
# 检查是否已存在远程仓库
if git remote get-url origin 2>/dev/null; then
    git remote set-url origin git@github.com:hello--world/winunlock.git
else
    git remote add origin git@github.com:hello--world/winunlock.git
fi

echo "推送到 dev 分支..."
git push -u origin dev

echo "完成！项目已推送到 GitHub dev 分支"

