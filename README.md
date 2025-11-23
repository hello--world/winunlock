# WinUnlock - Windows 自动解锁凭据提供程序

这是一个自定义的 Windows 凭据提供程序（Credential Provider），用于实现 Windows 工作站的自动解锁功能。

## 原理

Windows 凭据提供程序是 Windows 登录架构的一部分，允许开发者创建自定义的登录界面和认证流程。本项目通过实现 `ICredentialProvider` 和 `ICredentialProviderCredential` 接口，创建了一个可以在锁定屏幕上自动解锁 Windows 的凭据提供程序。

参考文档：[Windows 凭据提供程序](https://learn.microsoft.com/zh-cn/windows/win32/secauthn/credential-providers-in-windows)

## 功能特性

- ✅ 自动解锁 Windows 工作站
- ✅ **RDP 断开时自动解锁**（新功能）
- ✅ 使用 DPAPI 加密存储密码
- ✅ 支持本地账户解锁
- ✅ 无需用户交互即可完成解锁
- ✅ Tauri 图形界面配置工具
- ✅ GitHub Actions 自动构建

## 系统要求

- Windows 10/11 或 Windows Server 2016 及以上版本
- Visual Studio 2019 或更高版本（用于编译）
- CMake 3.15 或更高版本
- 管理员权限（用于注册和配置）

## 编译步骤

### 前置要求

- Windows 10/11 或 Windows Server 2016 及以上版本
- Visual Studio 2019 或更高版本（包含 C++ 工作负载）
- CMake 3.15 或更高版本
- Windows SDK 10.0 或更高版本

### 使用 Visual Studio

1. 打开 Visual Studio
2. 选择 "文件" -> "打开" -> "CMake..."
3. 选择项目根目录下的 `CMakeLists.txt`
4. 等待 CMake 配置完成
5. 选择 "生成" -> "生成解决方案" 或按 `Ctrl+Shift+B`

**编译目标：**
- `WinUnlock.dll` - 凭据提供程序 DLL
- `WinUnlockService.exe` - Windows 服务可执行文件

### 使用命令行（推荐）

在项目根目录打开 PowerShell 或命令提示符，执行：

```powershell
# 创建构建目录
mkdir build
cd build

# 配置 CMake（使用 Visual Studio 生成器）
cmake .. -G "Visual Studio 17 2022" -A x64

# 或者使用默认生成器
# cmake ..

# 编译 Release 版本
cmake --build . --config Release
```

**或者使用单行命令：**

```powershell
mkdir build; cd build; cmake ..; cmake --build . --config Release
```

### 编译输出

编译完成后，文件将位于以下位置：

```
build/
└── bin/
    ├── WinUnlock.dll          # 凭据提供程序 DLL
    └── WinUnlockService.exe   # Windows 服务可执行文件
```

### 使用 MSBuild（如果已安装）

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
msbuild WinUnlock.sln /p:Configuration=Release /p:Platform=x64
```

### 编译特定目标

如果只想编译其中一个目标：

```powershell
# 只编译凭据提供程序
cmake --build . --config Release --target WinUnlock

# 只编译服务
cmake --build . --config Release --target WinUnlockService
```

### 使用 GitHub Actions 编译（云端编译）

如果本地没有 Windows 开发环境，可以使用 GitHub Actions 在云端自动编译：

#### 方法一：推送代码触发构建

1. **提交并推送代码到 GitHub**
   ```bash
   git add .
   git commit -m "更新代码"
   git push origin dev  # 或 main
   ```

2. **查看构建状态**
   - 访问 GitHub 仓库的 "Actions" 标签页
   - 查看 "Build WinUnlock" 工作流程
   - 等待构建完成（通常 5-10 分钟）

3. **下载构建产物**
   - 点击成功的构建运行
   - 在 "Artifacts" 部分下载 `WinUnlock-Binaries`
   - 解压后包含：
     - `WinUnlock.dll` - 凭据提供程序
     - `WinUnlockService.exe` - Windows 服务

#### 方法二：在 Cursor 中使用 GitHub MCP Server（推荐）

```bash
# 1. 推送代码后，检查构建状态
"获取最新工作流程运行的详情"

# 2. 如果构建失败，查看错误
"下载最新失败的工作流程运行的日志"

# 3. 构建成功后，下载产物
# （需要在 GitHub 网页上下载 Artifacts）
```

#### 方法三：创建 Release 触发完整构建

```bash
# 创建并推送 tag
git tag v1.0.0
git push origin v1.0.0
```

这会触发：
- 编译 DLL 和服务
- 构建 Tauri 应用程序
- 创建 GitHub Release
- 上传所有构建产物

#### GitHub Actions 构建的优势

✅ **无需本地环境** - 不需要安装 Visual Studio、CMake 等  
✅ **自动化** - 每次推送代码自动构建  
✅ **可下载** - 构建产物可直接下载使用  
✅ **跨平台** - 可以在任何系统上触发 Windows 构建  
✅ **可追溯** - 所有构建历史都有记录

### 故障排除

**问题：找不到 credentialprovider.h**
- 确保已安装 Windows SDK 10.0 或更高版本
- 在 Visual Studio Installer 中安装 "Windows 10 SDK" 或 "Windows 11 SDK"

**问题：CMake 找不到 Visual Studio**
- 确保已安装 Visual Studio 2019 或更高版本
- 使用 `-G` 参数指定生成器：`cmake .. -G "Visual Studio 17 2022"`

**问题：链接错误**
- 确保以管理员权限运行编译命令（某些情况下需要）
- 检查 Windows SDK 是否正确安装

## 安装和配置

### 1. 注册凭据提供程序

以管理员身份运行 PowerShell，执行：

```powershell
.\register.ps1
```

或者手动注册：

```powershell
regsvr32.exe .\bin\WinUnlock.dll
```

### 2. 设置自动解锁密码

以管理员身份运行 PowerShell，执行：

```powershell
$securePassword = Read-Host -AsSecureString "请输入要用于自动解锁的密码"
.\SetPassword.ps1 -Password $securePassword
```

**重要提示：**
- 设置的密码应该是当前登录用户的密码
- 密码使用 DPAPI 加密存储在注册表中
- 密码存储在 `HKEY_LOCAL_MACHINE\SOFTWARE\WinUnlock\EncryptedPassword`

### 3. 安装 RDP 自动解锁服务（可选）

如果需要 RDP 断开时自动解锁功能，需要安装 Windows 服务：

```powershell
.\install-service.ps1
```

服务安装后会自动启动，并在系统启动时自动运行。

### 4. 重启或注销

注册完成后，需要注销当前会话或重启计算机，新的凭据提供程序才会生效。

## 使用方法

### 基本解锁功能

1. 锁定 Windows（Win + L）
2. 在锁定屏幕上，系统会自动使用存储的密码进行解锁
3. 无需手动输入密码即可解锁

### RDP 断开自动解锁功能

1. 通过 RDP 远程连接到 Windows 计算机
2. 当 RDP 连接断开时，Windows 服务会自动检测断开事件
3. 如果所有 RDP 会话都已断开，服务会触发自动解锁
4. 系统会自动使用存储的密码解锁本地工作站

**注意**：此功能需要安装并运行 `WinUnlockService` 服务。

## 卸载

### 卸载凭据提供程序

以管理员身份运行 PowerShell，执行：

```powershell
.\unregister.ps1
```

或者手动卸载：

```powershell
regsvr32.exe /u .\bin\WinUnlock.dll
```

### 卸载 RDP 自动解锁服务

如果安装了服务，需要先卸载服务：

```powershell
.\uninstall-service.ps1
```

或者手动卸载：

```powershell
Stop-Service WinUnlockService
sc.exe delete WinUnlockService
```

## 安全注意事项

⚠️ **重要安全警告：**

1. **密码存储**：虽然密码使用 DPAPI 加密，但存储在本地注册表中仍存在安全风险
2. **权限控制**：确保注册表项 `HKEY_LOCAL_MACHINE\SOFTWARE\WinUnlock` 的访问权限受到限制
3. **使用场景**：建议仅在受控环境中使用，例如：
   - 个人开发机器
   - 测试环境
   - 需要快速解锁的开发工作站
4. **不适用于**：
   - 生产环境
   - 包含敏感数据的计算机
   - 需要高安全性的系统

## 项目结构

```
winunlock/
├── CredentialProvider.h      # 头文件，定义所有类和接口
├── CredentialProvider.cpp    # 实现文件，包含所有功能实现
├── WinUnlockService.h        # Windows 服务头文件
├── WinUnlockService.cpp      # Windows 服务实现文件
├── dllmain.cpp              # DLL 入口点和 COM 注册函数
├── WinUnlock.def            # DLL 导出定义文件
├── CMakeLists.txt           # CMake 构建配置
├── register.ps1             # 注册脚本
├── unregister.ps1           # 卸载脚本
├── SetPassword.ps1          # 密码设置脚本
├── install-service.ps1      # 服务安装脚本
├── uninstall-service.ps1    # 服务卸载脚本
└── README.md                # 本文件
```

## 技术实现细节

### 核心接口

1. **ICredentialProvider**：凭据提供程序主接口
   - `SetUsageScenario()`: 设置使用场景（解锁/登录）
   - `GetCredentialCount()`: 返回凭据数量
   - `GetCredentialAt()`: 获取凭据对象

2. **ICredentialProviderCredential**：凭据接口
   - `SetSelected()`: 当凭据被选中时触发自动解锁
   - `GetSerialization()`: 序列化凭据信息用于认证

### 自动解锁流程

#### 基本解锁流程

1. 用户锁定屏幕
2. Windows 加载所有已注册的凭据提供程序
3. 本提供程序被选中（`SetSelected()`）
4. 从注册表读取加密的密码
5. 使用 DPAPI 解密密码
6. 通过 `GetSerialization()` 返回 Kerberos 认证包
7. Windows 使用返回的凭据自动解锁

#### RDP 断开自动解锁流程

1. Windows 服务监听 RDP 会话事件（使用 WTS API）
2. 检测到 RDP 会话断开（`WTS_SESSION_DISCONNECT` 或 `WTS_REMOTE_DISCONNECT`）
3. 服务检查是否还有其他活动的 RDP 会话
4. 如果没有其他 RDP 会话，服务设置注册表解锁请求标志
5. 服务调用 `LockWorkStation()` 锁定工作站
6. 凭据提供程序检测到解锁请求标志
7. 凭据提供程序自动执行解锁流程（同基本解锁流程）
8. 解锁完成后，清除解锁请求标志

### 密码加密

使用 Windows Data Protection API (DPAPI) 的 `CryptProtectData` 函数加密密码，使用 `CryptUnprotectData` 解密。加密数据存储在注册表中。

## 故障排除

### 凭据提供程序未显示

1. 确认 DLL 已正确注册：
   ```powershell
   reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers"
   ```

2. 检查事件查看器中的错误日志

3. 确认已注销并重新登录

### 自动解锁失败

1. 确认密码已正确设置：
   ```powershell
   reg query "HKLM\SOFTWARE\WinUnlock"
   ```

2. 确认设置的密码与当前用户密码匹配

3. 检查是否有权限问题

### 编译错误

1. 确认已安装 Windows SDK
2. 确认已安装 Visual Studio C++ 工具链
3. 检查 CMake 版本是否符合要求

## 开发说明

### 修改 CLSID

如果需要修改 CLSID（例如避免与其他提供程序冲突），需要：

1. 在 `CredentialProvider.h` 中修改 `CLSID_WinUnlockCredentialProvider`
2. 重新编译
3. 卸载旧版本
4. 注册新版本

### 调试

1. 使用 Visual Studio 附加到 `winlogon.exe` 进程（需要特殊权限）
2. 使用事件查看器查看系统日志
3. 在代码中添加日志输出（需要配置日志系统）

## 许可证

本项目仅供学习和研究使用。

## 参考资料

- [Windows 凭据提供程序文档](https://learn.microsoft.com/zh-cn/windows/win32/secauthn/credential-providers-in-windows)
- [ICredentialProvider 接口](https://learn.microsoft.com/en-us/windows/win32/api/credentialprovider/nn-credentialprovider-icredentialprovider)
- [ICredentialProviderCredential 接口](https://learn.microsoft.com/en-us/windows/win32/api/credentialprovider/nn-credentialprovider-icredentialprovidercredential)
- [Windows Data Protection API](https://learn.microsoft.com/en-us/windows/win32/api/dpapi/)

## 贡献

欢迎提交 Issue 和 Pull Request。

## GitHub Actions 构建

项目使用 GitHub Actions 进行自动构建。构建逻辑如下：

- **触发条件**：
  - 推送到 `dev` 或 `main` 分支
  - 创建 Pull Request 到 `dev` 或 `main`
  - 推送以 `v` 开头的 tag（用于发布）
- **构建流程**：
  1. 构建凭据提供程序 DLL（WinUnlock.dll）
  2. 构建 Windows 服务（WinUnlockService.exe）
  3. 构建 Tauri 应用程序（如果推送了 tag 或创建了 PR）
  4. 创建 GitHub Release（如果推送了 tag）

### 查看构建状态

#### 在 GitHub 网页上查看
1. 访问仓库的 "Actions" 标签页
2. 查看 "Build WinUnlock" 工作流程的运行状态
3. 点击运行查看详细日志

#### 在 Cursor 中使用 GitHub MCP Server（推荐）

```bash
# 列出所有工作流程
"列出所有工作流程"

# 查看最新构建运行
"获取最新工作流程运行的详情"

# 查看失败的构建
"列出状态为失败的工作流程运行"

# 下载构建日志
"下载最新工作流程运行的日志"
"下载最新失败的工作流程运行的日志"
```

详细使用说明请查看：`GITHUB_WORKFLOWS_GUIDE.md`

### 下载构建产物

构建成功后，可以在 GitHub Actions 运行页面下载：

1. 访问 Actions 标签页
2. 点击成功的运行
3. 在 "Artifacts" 部分下载：
   - `WinUnlock-Binaries` - 包含 `WinUnlock.dll` 和 `WinUnlockService.exe`
   - `WinUnlock-Installer` - Tauri 应用程序安装包（如果构建了）

### 创建 Release

要触发构建和发布，需要创建一个以 `v` 开头的 tag：

```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub Actions 会自动：
- 构建 DLL、服务可执行文件和 Tauri 应用
- 创建 GitHub Release
- 上传构建产物

### 本地编译 vs GitHub Actions 编译

| 方式 | 优点 | 缺点 |
|------|------|------|
| **本地编译** | 快速、可调试、无需等待 | 需要安装开发环境 |
| **GitHub Actions** | 自动化、无需本地环境、可下载产物 | 需要等待、需要推送代码 |

**推荐**：本地开发时使用本地编译，提交代码后使用 GitHub Actions 验证构建。

## 免责声明

本工具仅用于合法的系统管理和自动化目的。使用者需自行承担使用本工具的风险和责任。开发者不对任何因使用本工具而造成的损失或损害负责。

