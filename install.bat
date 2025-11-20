@echo off
REM 安装 WinUnlock 凭据提供程序
REM 需要管理员权限运行

echo ========================================
echo WinUnlock 凭据提供程序安装脚本
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

REM 检查 DLL 是否存在
if not exist "%DLL_PATH%" (
    echo 错误: 找不到 %DLL_PATH%
    echo 请先编译项目生成 DLL 文件
    pause
    exit /b 1
)

echo 正在注册 DLL...
regsvr32 /s "%DLL_PATH%"
if %errorLevel% neq 0 (
    echo 错误: DLL 注册失败
    pause
    exit /b 1
)

echo DLL 注册成功
echo.

REM 注册凭据提供程序
echo 正在注册凭据提供程序...
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{A1B2C3D4-E5F6-7890-ABCD-EF1234567891}" /v "" /t REG_SZ /d "WinUnlock Provider" /f >nul 2>&1

REM 创建配置注册表项（可选）
echo 正在创建配置项...
reg add "HKLM\SOFTWARE\WinUnlock" /f >nul 2>&1

echo.
echo ========================================
echo 安装完成！
echo ========================================
echo.
echo 注意: 
echo 1. 此凭据提供程序将在下次登录/解锁时显示
echo 2. 要配置自动解锁凭据，请运行 configure.bat
echo 3. 要卸载，请运行 uninstall.bat
echo.
pause

