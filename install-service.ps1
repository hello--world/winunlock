# Windows 服务安装脚本
# 需要以管理员权限运行

param(
    [string]$ServicePath = ".\bin\WinUnlockService.exe"
)

Write-Host "正在安装 WinUnlock 服务..." -ForegroundColor Green

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "错误: 需要管理员权限来安装服务" -ForegroundColor Red
    exit 1
}

# 检查服务可执行文件是否存在
if (-not (Test-Path $ServicePath)) {
    Write-Host "错误: 找不到服务可执行文件: $ServicePath" -ForegroundColor Red
    Write-Host "请先编译项目，确保 WinUnlockService.exe 存在于 bin 目录中" -ForegroundColor Yellow
    exit 1
}

# 获取绝对路径
$ServicePath = Resolve-Path $ServicePath

Write-Host "服务路径: $ServicePath" -ForegroundColor Yellow

# 检查服务是否已存在
$service = Get-Service -Name "WinUnlockService" -ErrorAction SilentlyContinue
if ($service) {
    Write-Host "服务已存在，正在停止并删除旧服务..." -ForegroundColor Yellow
    
    # 停止服务
    if ($service.Status -eq "Running") {
        Stop-Service -Name "WinUnlockService" -Force
        Start-Sleep -Seconds 2
    }
    
    # 删除服务
    sc.exe delete "WinUnlockService"
    Start-Sleep -Seconds 2
}

# 安装服务
Write-Host "正在安装服务..." -ForegroundColor Yellow
$result = & $ServicePath /install

if ($LASTEXITCODE -eq 0) {
    Write-Host "服务安装成功！" -ForegroundColor Green
    
    # 启动服务
    Write-Host "正在启动服务..." -ForegroundColor Yellow
    Start-Service -Name "WinUnlockService"
    
    # 检查服务状态
    Start-Sleep -Seconds 2
    $service = Get-Service -Name "WinUnlockService" -ErrorAction SilentlyContinue
    if ($service) {
        Write-Host "服务状态: $($service.Status)" -ForegroundColor Green
        Write-Host ""
        Write-Host "服务已成功安装并启动！" -ForegroundColor Green
        Write-Host "服务名称: WinUnlockService" -ForegroundColor Cyan
        Write-Host "显示名称: WinUnlock RDP Auto-Unlock Service" -ForegroundColor Cyan
    } else {
        Write-Host "警告: 无法查询服务状态" -ForegroundColor Yellow
    }
} else {
    Write-Host "错误: 服务安装失败" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "提示: 可以使用以下命令管理服务:" -ForegroundColor Yellow
Write-Host "  启动服务: Start-Service WinUnlockService" -ForegroundColor Cyan
Write-Host "  停止服务: Stop-Service WinUnlockService" -ForegroundColor Cyan
Write-Host "  查看状态: Get-Service WinUnlockService" -ForegroundColor Cyan
Write-Host "  卸载服务: .\uninstall-service.ps1" -ForegroundColor Cyan

