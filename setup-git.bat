@echo off
REM Git 仓库初始化脚本 (Windows)

echo 初始化 Git 仓库...

REM 检查是否已经是 Git 仓库
if exist .git (
    echo 已经是 Git 仓库
) else (
    git init
    echo Git 仓库初始化完成
)

REM 检查远程仓库
git remote show origin >nul 2>&1
if %errorLevel% equ 0 (
    echo 更新远程仓库 URL...
    git remote set-url origin git@github.com:hello--world/winunlock.git
    echo 远程仓库 URL 已更新
) else (
    echo 添加远程仓库...
    git remote add origin git@github.com:hello--world/winunlock.git
    echo 远程仓库已添加: git@github.com:hello--world/winunlock.git
)

REM 添加所有文件
echo.
echo 添加文件到 Git...
git add .

REM 显示状态
echo.
echo 当前 Git 状态:
git status

echo.
echo ========================================
echo 下一步操作:
echo ========================================
echo 1. 检查 SSH 密钥是否已配置:
echo    ssh -T git@github.com
echo.
echo 2. 提交更改:
echo    git commit -m "Initial commit: WinUnlock credential provider with Tauri config tool"
echo.
echo 3. 推送到 GitHub:
echo    git push -u origin main
echo.
echo 如果主分支是 master 而不是 main，请使用:
echo    git push -u origin master
echo.
pause

