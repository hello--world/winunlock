# 部署指南

## 快速开始

### 1. 设置 Git 仓库

#### Windows
```cmd
setup-git.bat
```

#### Linux/Mac
```bash
bash setup-git.sh
```

### 2. 配置 SSH 密钥（如果还没有）

```bash
# 生成 SSH 密钥
ssh-keygen -t ed25519 -C "your_email@example.com"

# 复制公钥内容
cat ~/.ssh/id_ed25519.pub

# 在 GitHub 上添加 SSH 密钥：
# Settings > SSH and GPG keys > New SSH key
```

### 3. 测试 SSH 连接

```bash
ssh -T git@github.com
```

应该看到：`Hi hello--world! You've successfully authenticated...`

### 4. 提交并推送代码

```bash
# 添加所有文件
git add .

# 提交
git commit -m "Initial commit: WinUnlock credential provider with Tauri config tool"

# 设置主分支名称（如果需要）
git branch -M main

# 推送到 GitHub
git push -u origin main
```

## GitHub Actions 构建

### 自动触发

当代码推送到 `main` 或 `master` 分支时，GitHub Actions 会自动：

1. **构建 C++ DLL**
   - 使用 MSBuild 编译凭据提供程序
   - 生成 `winunlock.dll`

2. **构建 Tauri 配置工具**
   - 安装 Node.js 和 Rust 依赖
   - 编译 Tauri 应用
   - 生成可执行文件

### 下载构建产物

1. 访问 GitHub 仓库的 Actions 页面
2. 选择最新的工作流运行
3. 在 Artifacts 部分下载：
   - `winunlock-dll` - 凭据提供程序 DLL
   - `winunlock-config-tool` - 配置工具可执行文件

### 手动触发构建

在 GitHub 仓库页面：
1. 点击 Actions 标签
2. 选择 "Build WinUnlock" 工作流
3. 点击 "Run workflow"

## 本地构建

### 构建 C++ 项目

```cmd
# 使用 Visual Studio Developer Command Prompt
msbuild winunlock.sln /p:Configuration=Release /p:Platform=x64
```

### 构建 Tauri 应用

```bash
cd tauri-app
npm install
npm run build
```

## 安装和配置

### 安装凭据提供程序

1. 以管理员身份运行 `install.bat`
2. 或手动注册 DLL：
   ```cmd
   regsvr32 x64\Release\winunlock.dll
   ```

### 配置凭据

**方法一：使用 Tauri 配置工具（推荐）**
1. 以管理员身份运行配置工具
2. 填写用户名和密码
3. 点击保存

**方法二：使用命令行**
```cmd
configure.bat
```

**方法三：手动编辑注册表**
```cmd
reg add "HKLM\SOFTWARE\WinUnlock" /v "Username" /t REG_SZ /d "your_username" /f
reg add "HKLM\SOFTWARE\WinUnlock" /v "Password" /t REG_SZ /d "your_password" /f
```

## 故障排除

### Git 推送失败

**错误：Permission denied (publickey)**
- 检查 SSH 密钥是否已添加到 GitHub
- 测试连接：`ssh -T git@github.com`

**错误：Repository not found**
- 确认仓库 URL 正确：`git@github.com:hello--world/winunlock.git`
- 确认有仓库访问权限

### GitHub Actions 构建失败

**MSBuild 未找到**
- 确保使用 `windows-latest` runner（已配置）

**Tauri 构建失败**
- 检查 Node.js 和 Rust 版本
- 查看 Actions 日志获取详细错误信息

### 本地构建问题

**C++ 编译错误**
- 确保安装了 Visual Studio 2019+ 和 Windows SDK
- 检查项目配置中的平台工具集版本

**Tauri 构建错误**
- 确保 Rust 已正确安装：`rustc --version`
- 确保 Node.js 版本 >= 18：`node --version`
- 清理并重新构建：`cd tauri-app && npm run tauri clean && npm run build`

## 安全注意事项

⚠️ **重要**：
- 密码以明文形式存储在注册表中（仅用于演示）
- 生产环境应使用更安全的存储方式
- 不要将包含真实密码的代码推送到公共仓库

