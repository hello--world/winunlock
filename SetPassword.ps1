# 设置自动解锁密码的脚本
# 使用 DPAPI 加密存储密码到注册表
# 需要以管理员权限运行

param(
    [Parameter(Mandatory=$true)]
    [SecureString]$Password,
    
    [string]$RegistryPath = "HKLM:\SOFTWARE\WinUnlock"
)

Write-Host "正在设置自动解锁密码..." -ForegroundColor Green

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "错误: 需要管理员权限" -ForegroundColor Red
    exit 1
}

# 将 SecureString 转换为字节数组
$BSTR = [System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($Password)
$PlainPassword = [System.Runtime.InteropServices.Marshal]::PtrToStringAuto($BSTR)
[System.Runtime.InteropServices.Marshal]::ZeroFreeBSTR($BSTR)

# 使用 DPAPI 加密密码
$Entropy = $null
$EncryptedData = [System.Security.Cryptography.ProtectedData]::Protect(
    [System.Text.Encoding]::Unicode.GetBytes($PlainPassword),
    $Entropy,
    [System.Security.Cryptography.DataProtectionScope]::LocalMachine
)

# 将加密数据转换为 Base64 字符串
$EncryptedBase64 = [Convert]::ToBase64String($EncryptedData)

# 创建注册表项
if (-not (Test-Path $RegistryPath)) {
    New-Item -Path $RegistryPath -Force | Out-Null
}

# 存储加密的密码
Set-ItemProperty -Path $RegistryPath -Name "EncryptedPassword" -Value $EncryptedBase64 -Type String

Write-Host "密码已成功设置并加密存储！" -ForegroundColor Green
Write-Host "警告: 请确保注册表项的安全性，限制访问权限。" -ForegroundColor Yellow

# 清理内存中的明文密码
$PlainPassword = $null
[System.GC]::Collect()

