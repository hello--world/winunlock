# 快速开始指南

## 🚀 快速部署到 GitHub

### 步骤 1: 配置 SSH 密钥

```bash
# 检查是否已有 SSH 密钥
ls ~/.ssh/id_ed25519.pub

# 如果没有，生成新的 SSH 密钥
ssh-keygen -t ed25519 -C "your_email@example.com"

# 复制公钥内容
cat ~/.ssh/id_ed25519.pub
```

在 GitHub 上添加 SSH 密钥：
1. 访问 https://github.com/settings/keys
2. 点击 "New SSH key"
3. 粘贴公钥内容并保存

### 步骤 2: 测试 SSH 连接

```bash
ssh -T git@github.com
```

应该看到：`Hi hello--world! You've successfully authenticated...`

### 步骤 3: 初始化并推送代码

**Windows:**
```cmd
setup-git.bat
git commit -m "Initial commit: WinUnlock with Tauri config tool"
git push -u origin main
```

**Linux/Mac:**
```bash
bash setup-git.sh
git commit -m "Initial commit: WinUnlock with Tauri config tool"
git push -u origin main
```

如果主分支是 `master` 而不是 `main`：
```bash
git branch -M main  # 重命名分支
git push -u origin main
```

## 📦 项目结构说明

```
winunlock/
├── 📁 C++ 凭据提供程序
│   ├── CredentialProvider.h/cpp
│   ├── Credential.h/cpp
│   └── winunlock.sln
│
├── 📁 Tauri 配置工具
│   ├── tauri-app/
│   │   ├── src-tauri/ (Rust 后端)
│   │   └── package.json
│   └── config-ui/ (前端界面)
│
├── 📁 GitHub Actions
│   └── .github/workflows/build.yml
│
└── 📁 脚本文件
    ├── install.bat
    ├── configure.bat
    └── setup-git.bat
```

## 🔧 本地开发

### 开发 C++ 凭据提供程序

1. 使用 Visual Studio 打开 `winunlock.sln`
2. 选择 Release x64 配置
3. 生成解决方案

### 开发 Tauri 配置工具

```bash
cd tauri-app
npm install
npm run dev  # 开发模式
npm run build  # 构建发布版本
```

## 🎯 使用流程

1. **编译项目** → 生成 DLL 和配置工具
2. **安装 DLL** → 运行 `install.bat`（需要管理员权限）
3. **配置凭据** → 运行 Tauri 配置工具或 `configure.bat`
4. **测试** → 锁定计算机（Win + L），查看锁定屏幕

## 📝 GitHub Actions 自动构建

推送代码后，GitHub Actions 会自动：

✅ 构建 C++ DLL (`winunlock.dll`)  
✅ 构建 Tauri 配置工具  
✅ 生成可下载的构建产物

**查看构建状态：**
- 访问仓库的 Actions 标签页
- 下载构建产物（Artifacts）

## ⚠️ 重要提示

1. **需要管理员权限**：安装和配置都需要管理员权限
2. **安全警告**：当前实现将密码以明文存储在注册表中，仅用于演示
3. **测试环境**：建议在虚拟机或测试机器上使用

## 🆘 遇到问题？

查看详细文档：
- [README.md](README.md) - 完整文档
- [DEPLOYMENT.md](DEPLOYMENT.md) - 部署指南

