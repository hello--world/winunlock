# Cursor 自动化构建和修复工作流

这个工作流展示了如何使用 Cursor + GitHub MCP Server 实现自动化开发流程。

## 工作流程

```
1. 写代码 (Cursor AI)
   ↓
2. 提交代码 (自动)
   ↓
3. 推送到 GitHub (自动)
   ↓
4. 触发 GitHub Actions 构建 (自动)
   ↓
5. 检查构建状态 (GitHub MCP)
   ↓
6. 获取构建错误 (GitHub MCP)
   ↓
7. 自动修复错误 (Cursor AI)
   ↓
8. 重复步骤 1-7 直到构建成功
```

## 使用方法

### 方法一：使用自动化脚本

```bash
# 提交、推送并检查构建
node scripts/auto-build-check.js "你的提交信息"

# 只提交，不推送
node scripts/auto-build-check.js "你的提交信息" --no-push

# 提交、推送并创建 PR
node scripts/auto-build-check.js "你的提交信息" --create-pr
```

### 方法二：在 Cursor 中使用 GitHub MCP Server

#### 1. 提交代码后，检查构建状态

在 Cursor 聊天中询问：
```
"检查 winunlock 仓库中最新 PR 的构建状态"
```

或者：
```
"获取 PR #123 的构建状态和错误信息"
```

#### 2. 查看构建错误

```
"显示 PR #123 的构建日志中的错误"
```

#### 3. 自动修复错误

```
"根据这个构建错误修复代码：[粘贴错误信息]"
```

#### 4. 创建 Pull Request

```
"创建一个从 dev 分支到 main 分支的 Pull Request，标题是：修复构建错误"
```

## 完整自动化示例

### 场景：修复编译错误

1. **在 Cursor 中写代码**
   ```
   用户: "添加一个新的函数来验证密码"
   ```

2. **提交代码**
   ```bash
   node scripts/auto-build-check.js "添加密码验证函数"
   ```

3. **在 Cursor 中检查构建**
   ```
   用户: "检查最新的 PR 构建状态"
   ```

4. **如果构建失败，获取错误**
   ```
   用户: "显示构建错误详情"
   ```

5. **自动修复**
   ```
   用户: "根据这些错误修复代码：[粘贴错误]"
   ```

6. **重复步骤 2-5 直到成功**

## GitHub MCP Server 常用命令

### 检查构建状态
- `"获取 PR #123 的状态"`
- `"列出所有打开的 PR"`
- `"检查 PR #123 的构建状态"`

### 查看错误
- `"获取 PR #123 的 diff"`
- `"显示 PR #123 的审查评论"`

### 创建/更新 PR
- `"创建一个从 dev 到 main 的 PR"`
- `"更新 PR #123 的描述"`

### 搜索代码
- `"搜索包含 'CredentialProvider' 的代码"`
- `"在 winunlock 仓库中搜索 'build error'"`

## 高级自动化：使用 GitHub Copilot

### 分配 Copilot 到 Issue
```
"分配 Copilot 到 issue #123"
```
Copilot 会自动：
- 分析问题
- 创建修复代码
- 创建 Pull Request
- 等待审查

### 请求 Copilot 审查 PR
```
"请求 Copilot 审查 PR #123"
```
Copilot 会：
- 审查代码
- 提供改进建议
- 标记潜在问题

## 故障排除

### 构建一直失败
1. 检查 GitHub Actions 日志
2. 在 Cursor 中询问："分析这个构建错误：[粘贴错误]"
3. 根据建议修复代码

### MCP Server 无法连接
1. 检查 `~/.cursor/mcp.json` 配置
2. 确认 GitHub PAT 有效
3. 重启 Cursor

### 无法获取构建状态
- 确保 PR 已创建
- 检查 GitHub Actions 是否已运行
- 使用 `"列出所有打开的 PR"` 确认 PR 存在

## 最佳实践

1. **小步提交**：频繁提交小改动，便于定位问题
2. **描述性提交信息**：使用清晰的提交信息
3. **及时检查**：推送后立即检查构建状态
4. **利用 AI**：充分利用 Cursor AI 和 GitHub Copilot
5. **自动化测试**：在本地运行测试后再推送

## 示例工作流脚本

创建一个 `.cursor/auto-fix.sh` 脚本：

```bash
#!/bin/bash

# 1. 提交代码
git add -A
git commit -m "$1"

# 2. 推送到远程
git push origin dev

# 3. 在 Cursor 中提示用户检查构建
echo "✅ 代码已推送，请在 Cursor 中询问："
echo "   '检查最新的 PR 构建状态'"
```

然后在 Cursor 中：
```
"运行 .cursor/auto-fix.sh '修复编译错误'"
```

