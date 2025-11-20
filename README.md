# WinUnlock - Windows 自动解锁凭据提供程序

这是一个自定义的 Windows 凭据提供程序（Credential Provider），用于实现 Windows 系统的自动解锁功能。

## 原理

Windows 凭据提供程序是 Windows 登录界面和凭据 UI 的扩展机制。通过实现 `ICredentialProvider` 和 `ICredentialProviderCredential` 接口，可以创建自定义的登录/解锁方式。

本项目实现了一个简单的自动解锁凭据提供程序，它会在锁定屏幕上显示一个选项，当被选中时自动使用预配置的凭据进行解锁。

## 功能特性

- 支持 Windows 登录和锁定屏幕解锁场景
- 自动检测并应用预配置的凭据
- 可配置的用户名和密码存储（当前使用注册表，仅用于演示）
- **图形化配置工具**：基于 Tauri 的现代化配置界面
- **自动化构建**：GitHub Actions 自动构建和发布

## 系统要求

- Windows 10/11 (x64)
- Visual Studio 2019 或更高版本
- Windows SDK 10.0 或更高版本
- 管理员权限（用于安装和注册）

## 编译说明

### 编译 C++ 凭据提供程序

1. 使用 Visual Studio 打开 `winunlock.sln`
2. 选择 Release x64 配置
3. 生成解决方案（Build Solution）
4. 编译后的 DLL 将位于 `x64\Release\winunlock.dll`

### 编译 Tauri 配置工具

1. 安装 Node.js (v18 或更高版本)
2. 安装 Rust (stable 版本)
3. 进入 `tauri-app` 目录
4. 运行 `npm install` 安装依赖
5. 运行 `npm run build` 构建发布版本
6. 可执行文件将位于 `tauri-app/src-tauri/target/release/`

### 使用 GitHub Actions 自动构建

项目已配置 GitHub Actions 工作流，推送到 GitHub 后会自动构建：
- C++ DLL 文件
- Tauri 配置工具

构建产物可在 GitHub Actions 页面下载。

## 安装步骤

1. **编译项目**（见编译说明）

2. **以管理员身份运行 `install.bat`**
   - 这将注册 DLL 并添加必要的注册表项

3. **配置凭据**（推荐使用图形界面）
   - **方法一（推荐）**：运行 Tauri 配置工具
     - 进入 `tauri-app` 目录
     - 运行 `npm install` 安装依赖
     - 运行 `npm run dev` 启动开发模式，或 `npm run build` 构建发布版本
     - 以管理员身份运行生成的配置工具
     - 在图形界面中填写用户名和密码并保存
   - **方法二**：使用命令行脚本
     - 以管理员身份运行 `configure.bat`
     - 输入要用于自动解锁的用户名和密码
   - 注意：当前实现将密码以明文形式存储在注册表中，仅用于演示

4. **测试**
   - 锁定计算机（Win + L）
   - 在锁定屏幕上，您应该能看到 "自动解锁" 选项
   - 选择该选项后，系统将尝试使用配置的凭据自动解锁

## 卸载步骤

以管理员身份运行 `uninstall.bat` 即可卸载凭据提供程序。

## 安全注意事项

⚠️ **重要安全警告**：

1. **当前实现仅用于演示目的**。代码中的密码存储方式（注册表明文）**不适合生产环境**。

2. **生产环境建议**：
   - 使用 Windows Credential Manager API 安全存储凭据
   - 使用 DPAPI (Data Protection API) 加密存储敏感信息
   - 实现基于证书或智能卡的身份验证
   - 添加额外的安全验证机制（如设备指纹、时间窗口等）

3. **自动解锁会降低系统安全性**，请谨慎使用。

## 代码结构

```
winunlock/
├── CredentialProvider.h/cpp    # ICredentialProvider 接口实现
├── Credential.h/cpp             # ICredentialProviderCredential 接口实现
├── dllmain.cpp                  # DLL 入口点和类工厂
├── pch.h                        # 预编译头文件
├── winunlock.def                # DLL 导出定义
├── winunlock.vcxproj            # Visual Studio 项目文件
├── winunlock.sln                # Visual Studio 解决方案
├── install.bat                  # 安装脚本
├── uninstall.bat                # 卸载脚本
├── configure.bat                # 配置脚本（命令行方式）
├── tauri-app/                   # Tauri 配置工具
│   ├── src-tauri/               # Rust 后端代码
│   │   ├── src/main.rs          # Tauri 主程序
│   │   └── Cargo.toml           # Rust 依赖配置
│   ├── package.json             # Node.js 配置
│   └── tauri.conf.json          # Tauri 配置文件
├── config-ui/                   # 前端界面
│   ├── index.html               # 主页面
│   ├── styles.css               # 样式文件
│   └── main.js                  # JavaScript 逻辑
├── .github/workflows/           # GitHub Actions
│   └── build.yml                # 自动构建工作流
├── setup-git.bat                # Git 初始化脚本（Windows）
├── setup-git.sh                 # Git 初始化脚本（Linux/Mac）
└── README.md                    # 本文件
```

## 主要接口实现

### WinUnlockProvider (ICredentialProvider)
- `SetUsageScenario`: 设置使用场景（登录/解锁）
- `GetCredentialCount`: 返回凭据数量
- `GetCredentialAt`: 获取凭据对象

### WinUnlockCredential (ICredentialProviderCredential)
- `SetSelected`: 当凭据被选中时触发，检查是否可以自动解锁
- `GetSerialization`: 序列化凭据数据，用于实际的身份验证

## 自定义凭据获取

当前实现中，`_GetAutoUnlockCredentials` 方法从注册表读取凭据。您可以修改此方法以：

1. 从 Windows Credential Manager 读取
2. 从加密文件读取
3. 从网络服务获取
4. 使用硬件令牌
5. 实现其他自定义逻辑

## 故障排除

### 凭据提供程序未显示
- 确保已正确安装（运行 install.bat）
- 检查注册表项是否存在
- 查看 Windows 事件查看器中的错误日志
- 可能需要重新启动计算机

### 自动解锁失败
- 检查配置的用户名和密码是否正确
- 确保用户账户存在且未被禁用
- 检查账户锁定策略

### DLL 注册失败
- 确保以管理员权限运行
- 检查 DLL 文件是否存在
- 确保没有其他程序正在使用该 DLL

## Git 仓库设置

### 首次推送项目到 GitHub

1. **配置 SSH 密钥**（如果还没有）
   ```bash
   ssh-keygen -t ed25519 -C "your_email@example.com"
   # 将公钥添加到 GitHub: Settings > SSH and GPG keys
   ```

2. **初始化 Git 仓库**
   - Windows: 运行 `setup-git.bat`
   - Linux/Mac: 运行 `bash setup-git.sh`

3. **提交并推送**
   ```bash
   git add .
   git commit -m "Initial commit: WinUnlock credential provider with Tauri config tool"
   git branch -M main  # 如果主分支不是 main
   git push -u origin main
   ```

### GitHub Actions 构建

项目已配置自动构建工作流：
- 推送到 `main` 或 `master` 分支时自动触发
- 构建 C++ DLL 和 Tauri 配置工具
- 构建产物可在 Actions 页面下载

## 参考资料

- [Windows 中的凭据提供程序](https://learn.microsoft.com/zh-cn/windows/win32/secauthn/credential-providers-in-windows)
- [ICredentialProvider 接口](https://learn.microsoft.com/zh-cn/windows/win32/api/credentialprovider/nn-credentialprovider-icredentialprovider)
- [ICredentialProviderCredential 接口](https://learn.microsoft.com/zh-cn/windows/win32/api/credentialprovider/nn-credentialprovider-icredentialprovidercredential)
- [Tauri 文档](https://tauri.app/)
- [GitHub Actions 文档](https://docs.github.com/en/actions)

## 许可证

本项目仅供学习和研究使用。

## 免责声明

本软件按"原样"提供，不提供任何明示或暗示的保证。使用本软件的风险由用户自行承担。作者不对因使用本软件而造成的任何损害承担责任。

