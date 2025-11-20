@echo off
REM 卸载 WinUnlock 凭据提供程序
REM 需要管理员权限运行

echo ========================================
echo WinUnlock 凭据提供程序卸载脚本
echo ========================================
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo 错误: 需要管理员权限运行此脚本
    echo 请右键点击此文件，选择"以管理员身份运行"
    pause
    exit /b 1
)

set DLL_NAME=winunlock.dll
set DLL_PATH=%~dp0x64\Release\%DLL_NAME%

echo 正在注销 DLL...
regsvr32 /u /s "%DLL_PATH%"
if %errorLevel% neq 0 (
    echo 警告: DLL 注销可能失败（可能未安装）
)

echo 正在删除注册表项...
reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{A1B2C3D4-E5F6-7890-ABCD-EF1234567891}" /f >nul 2>&1
reg delete "HKLM\SOFTWARE\WinUnlock" /f >nul 2>&1

echo.
echo ========================================
echo 卸载完成！
echo ========================================
echo.
echo 注意: 可能需要重新启动计算机才能完全移除凭据提供程序
echo.
pause

