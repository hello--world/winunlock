# Windows 凭据提供程序注册脚本
# 需要以管理员权限运行

param(
    [string]$DllPath = ".\bin\WinUnlock.dll"
)

Write-Host "正在注册 WinUnlock 凭据提供程序..." -ForegroundColor Green

# 检查 DLL 是否存在
if (-not (Test-Path $DllPath)) {
    Write-Host "错误: 找不到 DLL 文件: $DllPath" -ForegroundColor Red
    exit 1
}

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "错误: 需要管理员权限来注册 DLL" -ForegroundColor Red
    exit 1
}

# 注册 DLL
Write-Host "正在注册 COM 组件..." -ForegroundColor Yellow
$result = regsvr32.exe /s $DllPath
if ($LASTEXITCODE -ne 0) {
    Write-Host "错误: 注册失败" -ForegroundColor Red
    exit 1
}

# 或者使用 DllRegisterServer
Write-Host "正在调用 DllRegisterServer..." -ForegroundColor Yellow
$assembly = [System.Reflection.Assembly]::LoadFile((Resolve-Path $DllPath).Path)
$type = $assembly.GetType("WinUnlock.DllRegisterServer")
if ($type) {
    $method = $type.GetMethod("DllRegisterServer", [System.Reflection.BindingFlags]::Static -bor [System.Reflection.BindingFlags]::Public)
    if ($method) {
        $method.Invoke($null, $null)
    }
}

Write-Host "注册完成！" -ForegroundColor Green
Write-Host "请重启计算机或注销后重新登录以使更改生效。" -ForegroundColor Yellow

