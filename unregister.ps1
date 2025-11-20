# Windows 凭据提供程序卸载脚本
# 需要以管理员权限运行

param(
    [string]$DllPath = ".\bin\WinUnlock.dll"
)

Write-Host "正在卸载 WinUnlock 凭据提供程序..." -ForegroundColor Green

# 检查 DLL 是否存在
if (-not (Test-Path $DllPath)) {
    Write-Host "警告: 找不到 DLL 文件: $DllPath" -ForegroundColor Yellow
}

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "错误: 需要管理员权限来卸载 DLL" -ForegroundColor Red
    exit 1
}

# 卸载 DLL
Write-Host "正在卸载 COM 组件..." -ForegroundColor Yellow
$result = regsvr32.exe /s /u $DllPath
if ($LASTEXITCODE -ne 0) {
    Write-Host "警告: 卸载可能失败" -ForegroundColor Yellow
}

Write-Host "卸载完成！" -ForegroundColor Green
Write-Host "请重启计算机或注销后重新登录以使更改生效。" -ForegroundColor Yellow

