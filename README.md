# WinUnlock - Windows 自动解锁凭据提供程序

这是一个自定义的 Windows 凭据提供程序（Credential Provider），用于实现 Windows 工作站的自动解锁功能。

## 原理

Windows 凭据提供程序是 Windows 登录架构的一部分，允许开发者创建自定义的登录界面和认证流程。本项目通过实现 `ICredentialProvider` 和 `ICredentialProviderCredential` 接口，创建了一个可以在锁定屏幕上自动解锁 Windows 的凭据提供程序。

参考文档：[Windows 凭据提供程序](https://learn.microsoft.com/zh-cn/windows/win32/secauthn/credential-providers-in-windows)

## 功能特性

- ✅ 自动解锁 Windows 工作站
- ✅ 使用 DPAPI 加密存储密码
- ✅ 支持本地账户解锁
- ✅ 无需用户交互即可完成解锁

## 系统要求

- Windows 10/11 或 Windows Server 2016 及以上版本
- Visual Studio 2019 或更高版本（用于编译）
- CMake 3.15 或更高版本
- 管理员权限（用于注册和配置）

## 编译步骤

### 使用 Visual Studio

1. 打开 Visual Studio
2. 选择 "文件" -> "打开" -> "CMake..."
3. 选择项目根目录下的 `CMakeLists.txt`
4. 等待 CMake 配置完成
5. 选择 "生成" -> "生成解决方案"

### 使用命令行

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

编译完成后，DLL 文件将位于 `build/bin/WinUnlock.dll`

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

### 3. 重启或注销

注册完成后，需要注销当前会话或重启计算机，新的凭据提供程序才会生效。

## 使用方法

1. 锁定 Windows（Win + L）
2. 在锁定屏幕上，系统会自动使用存储的密码进行解锁
3. 无需手动输入密码即可解锁

## 卸载

以管理员身份运行 PowerShell，执行：

```powershell
.\unregister.ps1
```

或者手动卸载：

```powershell
regsvr32.exe /u .\bin\WinUnlock.dll
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
├── dllmain.cpp              # DLL 入口点和 COM 注册函数
├── WinUnlock.def            # DLL 导出定义文件
├── CMakeLists.txt           # CMake 构建配置
├── register.ps1             # 注册脚本
├── unregister.ps1           # 卸载脚本
├── SetPassword.ps1          # 密码设置脚本
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

1. 用户锁定屏幕
2. Windows 加载所有已注册的凭据提供程序
3. 本提供程序被选中（`SetSelected()`）
4. 从注册表读取加密的密码
5. 使用 DPAPI 解密密码
6. 通过 `GetSerialization()` 返回 Kerberos 认证包
7. Windows 使用返回的凭据自动解锁

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

## 免责声明

本工具仅用于合法的系统管理和自动化目的。使用者需自行承担使用本工具的风险和责任。开发者不对任何因使用本工具而造成的损失或损害负责。

