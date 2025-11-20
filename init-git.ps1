# PowerShell 脚本：初始化 Git 仓库并推送到 GitHub

Write-Host "初始化 Git 仓库..." -ForegroundColor Green
git init

Write-Host "添加所有文件..." -ForegroundColor Green
git add .

Write-Host "创建初始提交..." -ForegroundColor Green
git commit -m "Initial commit: WinUnlock 项目"

Write-Host "创建 dev 分支..." -ForegroundColor Green
git checkout -b dev

Write-Host "添加远程仓库..." -ForegroundColor Green
# 检查是否已存在远程仓库
$remoteExists = git remote get-url origin 2>$null
if ($LASTEXITCODE -ne 0) {
    git remote add origin git@github.com:hello--world/winunlock.git
} else {
    git remote set-url origin git@github.com:hello--world/winunlock.git
}

Write-Host "推送到 dev 分支..." -ForegroundColor Green
git push -u origin dev

Write-Host "完成！项目已推送到 GitHub dev 分支" -ForegroundColor Green

