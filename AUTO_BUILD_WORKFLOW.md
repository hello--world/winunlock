# Cursor + GitHub Actions 自动化构建和修复工作流

## ✅ 已完成的配置

### 1. GitHub Actions 配置更新

已修改 `.github/workflows/build.yml`：
- ✅ **每次 push 都会触发构建**（不仅仅是 tag）
- ✅ **添加了错误报告功能**，构建失败时会输出详细错误信息
- ✅ **支持 Pull Request 构建**，可以在 PR 中看到构建状态

### 2. 自动化脚本

创建了 `scripts/auto-build-check.js`：
- 自动提交代码
- 推送到 GitHub
- 检查构建状态（需要配合 GitHub MCP Server）

### 3. 工作流文档

- `scripts/cursor-auto-fix.md` - 详细使用说明
- `.cursor/workflow-example.md` - 实际使用示例

---

## 🚀 如何使用自动化工作流

### 方法一：在 Cursor 中直接使用（推荐）

#### 完整流程示例：

```
1. 写代码
   你: "添加一个密码验证函数"

2. 提交代码
   git add -A
   git commit -m "添加密码验证函数"
   git push origin dev

3. 创建 PR（如果需要）
   你: "创建一个从 dev 到 main 的 Pull Request"

4. 检查构建状态
   你: "检查最新 PR 的构建状态"

5. 如果构建失败，获取错误
   你: "显示构建错误详情"

6. 自动修复
   你: "根据这些错误修复代码：[粘贴错误]"

7. 重复步骤 2-6 直到成功
```

### 方法二：使用自动化脚本

```bash
# 提交、推送并检查构建
node scripts/auto-build-check.js "你的提交信息"

# 只提交，不推送
node scripts/auto-build-check.js "你的提交信息" --no-push

# 提交、推送并创建 PR
node scripts/auto-build-check.js "你的提交信息" --create-pr
```

---

## 📋 GitHub MCP Server 常用命令

### 工作流程（Workflows/GitHub Actions）管理 ⭐ **官方功能**

GitHub MCP Server 提供了完整的工作流程管理功能，可以直接管理 GitHub Actions：

#### 列出工作流程
```
"列出 winunlock 仓库的所有工作流程"
"列出所有工作流程"
```

#### 查看工作流程运行（支持过滤）
```
"列出 Build WinUnlock 工作流程的运行"
"列出工作流程运行，状态为失败"
"列出工作流程运行，分支为 dev"
"列出状态为进行中的工作流程运行"
```

#### 获取运行详情和日志 ⭐ **最实用**
```
"获取工作流程运行 #123 的详情"
"下载工作流程运行 #123 的日志"  # 直接获取完整构建日志
"获取最新工作流程运行的详情"
"下载最新工作流程运行的日志"  # 快速查看最新构建错误
"下载最新失败的工作流程运行的日志"  # 直接获取失败原因
```

#### 使用指标
```
"获取工作流程运行的使用指标"
```

**详细使用说明请查看：`GITHUB_WORKFLOWS_GUIDE.md`**

### 检查构建状态（通过 PR）
```
"检查 winunlock 仓库中最新 PR 的构建状态"
"获取 PR #123 的状态"
"列出所有打开的 PR"
```

### 查看错误
```
"显示 PR #123 的构建错误"
"获取 PR #123 的 diff"
"显示 PR #123 的审查评论"
"下载工作流程运行 #123 的日志"  # 直接获取构建日志
```

### 创建/管理 PR
```
"创建一个从 dev 到 main 的 PR，标题：修复构建错误"
"更新 PR #123 的描述"
"合并 PR #123"
```

### 使用 GitHub Copilot
```
"分配 Copilot 到 issue #123"  # Copilot 会自动修复并创建 PR
"请求 Copilot 审查 PR #123"   # Copilot 会审查代码
```

---

## 🔄 完整自动化循环

### 场景：修复编译错误

```
1. 你: "添加一个新功能"
   → Cursor AI 帮你写代码

2. 你: "提交代码并创建 PR"
   → 代码被提交，PR 被创建

3. 你: "检查构建状态"
   → AI 使用 GitHub MCP 检查构建
   → "构建失败，错误是：缺少头文件"

4. 你: "修复这个错误"
   → AI 分析错误，修复代码

5. 你: "再次检查构建"
   → AI 检查："✅ 构建成功！"
```

---

## 🎯 实际使用示例

### 示例 1: 快速修复构建错误

```
你: "我刚刚推送了代码，帮我检查构建状态"

AI: [使用 GitHub MCP 检查]
"构建失败了，错误是：缺少头文件 'credentialprovider.h'"

你: "帮我修复这个错误"

AI: [分析代码，添加缺失的头文件，修复代码]

你: "提交修复并推送"

AI: [提交并推送]

你: "再次检查构建状态"

AI: [检查] "✅ 构建成功！"
```

### 示例 2: 使用 Copilot 自动修复

```
你: "分配 Copilot 到 issue #123"

AI: [使用 GitHub MCP]
"已分配 Copilot 到 issue #123，Copilot 正在分析问题并创建修复..."

[几分钟后]

你: "检查 Copilot 创建的 PR"

AI: [检查] "PR #45 已创建，构建中..."

你: "构建完成了吗？"

AI: [检查] "✅ 构建成功！PR 已准备好合并。"
```

---

## ⚙️ 配置说明

### GitHub Actions 触发条件

现在构建会在以下情况触发：
- ✅ 推送到 `dev` 或 `main` 分支
- ✅ 创建 Pull Request 到 `dev` 或 `main`
- ✅ 推送以 `v` 开头的 tag（用于发布）

### 构建错误报告

构建失败时，GitHub Actions 会：
- 输出详细的错误信息
- 标记为失败状态
- 在 PR 中显示构建状态

---

## 💡 最佳实践

1. **小步提交**：频繁提交小改动，便于定位问题
2. **描述性提交信息**：使用清晰的提交信息，便于 AI 理解
3. **及时检查**：推送后立即检查构建状态
4. **利用 AI**：充分利用 Cursor AI 和 GitHub Copilot 自动修复
5. **本地测试**：在本地运行测试后再推送

---

## 🔧 故障排除

### 构建一直失败
```
你: "分析这个构建错误并修复：[粘贴完整错误日志]"
```

### 找不到 PR
```
你: "列出所有打开的 PR"
你: "创建从 dev 到 main 的 PR"
```

### MCP Server 无响应
- 检查 `~/.cursor/mcp.json` 配置
- 确认 GitHub PAT 有效
- 重启 Cursor

### GitHub Actions 没有运行
- 检查 `.github/workflows/build.yml` 配置
- 确认已推送到正确的分支
- 查看 GitHub Actions 标签页

---

## 📚 相关文档

- **`GITHUB_WORKFLOWS_GUIDE.md`** ⭐ - GitHub MCP Server 工作流程功能完整指南
- `scripts/cursor-auto-fix.md` - 详细使用说明
- `.cursor/workflow-example.md` - 实际使用示例
- `scripts/auto-build-check.js` - 自动化脚本源码

---

## ✨ 总结

现在你可以：

1. ✅ **在 Cursor 中写代码** - 使用 AI 辅助编程
2. ✅ **自动触发构建** - 每次 push 都会触发 GitHub Actions
3. ✅ **检查构建状态** - 使用 GitHub MCP Server 检查状态
4. ✅ **获取错误信息** - 直接从 GitHub 获取构建错误
5. ✅ **自动修复错误** - 让 Cursor AI 根据错误自动修复代码
6. ✅ **使用 Copilot** - 让 GitHub Copilot 自动处理 issue 和审查 PR

**完整的自动化开发循环已就绪！** 🎉

