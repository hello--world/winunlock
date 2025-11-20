@echo off
REM 配置 WinUnlock 自动解锁凭据
REM 需要管理员权限运行

echo ========================================
echo WinUnlock 凭据配置脚本
echo ========================================
echo.
echo 警告: 此脚本将在注册表中存储用户名和密码（明文）
echo 仅用于测试目的，生产环境应使用更安全的方法
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo 错误: 需要管理员权限运行此脚本
    echo 请右键点击此文件，选择"以管理员身份运行"
    pause
    exit /b 1
)

set /p USERNAME="请输入用户名: "
set /p PASSWORD="请输入密码: "

if "%USERNAME%"=="" (
    echo 错误: 用户名不能为空
    pause
    exit /b 1
)

if "%PASSWORD%"=="" (
    echo 错误: 密码不能为空
    pause
    exit /b 1
)

echo.
echo 正在保存配置...
reg add "HKLM\SOFTWARE\WinUnlock" /v "Username" /t REG_SZ /d "%USERNAME%" /f >nul 2>&1
reg add "HKLM\SOFTWARE\WinUnlock" /v "Password" /t REG_SZ /d "%PASSWORD%" /f >nul 2>&1

if %errorLevel% equ 0 (
    echo 配置已保存
) else (
    echo 错误: 配置保存失败
    pause
    exit /b 1
)

echo.
echo ========================================
echo 配置完成！
echo ========================================
echo.
echo 注意: 密码以明文形式存储在注册表中，请确保系统安全
echo.
pause

