# Windows 服务卸载脚本
# 需要以管理员权限运行

Write-Host "正在卸载 WinUnlock 服务..." -ForegroundColor Green

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "错误: 需要管理员权限来卸载服务" -ForegroundColor Red
    exit 1
}

# 检查服务是否存在
$service = Get-Service -Name "WinUnlockService" -ErrorAction SilentlyContinue
if (-not $service) {
    Write-Host "服务不存在，无需卸载" -ForegroundColor Yellow
    exit 0
}

# 停止服务
if ($service.Status -eq "Running") {
    Write-Host "正在停止服务..." -ForegroundColor Yellow
    Stop-Service -Name "WinUnlockService" -Force
    Start-Sleep -Seconds 2
}

# 卸载服务
Write-Host "正在卸载服务..." -ForegroundColor Yellow
$result = sc.exe delete "WinUnlockService"

if ($LASTEXITCODE -eq 0) {
    Write-Host "服务卸载成功！" -ForegroundColor Green
} else {
    Write-Host "错误: 服务卸载失败" -ForegroundColor Red
    Write-Host "错误代码: $LASTEXITCODE" -ForegroundColor Red
    exit 1
}

# 验证服务已删除
Start-Sleep -Seconds 1
$service = Get-Service -Name "WinUnlockService" -ErrorAction SilentlyContinue
if (-not $service) {
    Write-Host "服务已成功从系统中移除" -ForegroundColor Green
} else {
    Write-Host "警告: 服务可能未完全删除" -ForegroundColor Yellow
}

