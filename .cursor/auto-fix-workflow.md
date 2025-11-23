# Cursor 完全自动化构建工作流

这个工作流实现了完全自动化：提交代码 → 检查构建 → 自动修复 → 重复直到成功

## 🚀 使用方法

### 方法一：使用自动化脚本（需要 GitHub Token）

```bash
# 设置 GitHub Token（可选，但推荐）
export GITHUB_TOKEN=your_github_token

# 运行自动化脚本
node scripts/auto-build-until-success.js "你的提交信息"
```

脚本会：
1. 提交并推送代码
2. 等待 GitHub Actions 构建
3. 如果失败，提示你修复
4. 重复直到成功

### 方法二：在 Cursor 中使用完全自动化工作流（推荐）

#### 步骤 1: 提交代码
```bash
git add -A
git commit -m "你的提交信息"
git push origin dev
```

#### 步骤 2: 在 Cursor 中启动自动化流程

```
你: "帮我实现完全自动化构建，直到编译成功"
```

AI 会执行以下流程：

1. **检查构建状态**
   ```
   AI: "让我检查最新的工作流程运行..."
   AI: [使用 GitHub MCP] "列出 Build WinUnlock 工作流程的运行"
   ```

2. **如果构建失败，获取错误**
   ```
   AI: "构建失败，让我获取错误日志..."
   AI: [使用 GitHub MCP] "下载最新失败的工作流程运行的日志"
   ```

3. **分析并修复错误**
   ```
   AI: [分析错误] "错误是：缺少头文件 'credentialprovider.h'"
   AI: [修复代码] "已修复，添加了缺失的头文件"
   ```

4. **提交修复并再次检查**
   ```
   AI: [提交代码] "代码已修复并提交"
   AI: [检查构建] "再次检查构建状态..."
   ```

5. **重复直到成功**
   ```
   AI: "✅ 构建成功！"
   ```

## 📋 完整自动化命令序列

你可以在 Cursor 中依次执行：

```
1. "列出 Build WinUnlock 工作流程的最新运行"

2. "如果构建失败，下载最新失败的工作流程运行的日志"

3. "根据这个构建错误修复代码：[粘贴错误]"

4. "提交修复并推送"

5. "再次检查最新工作流程运行的状态"

6. 如果还是失败，重复步骤 2-5
```

## 🤖 使用 AI 自动修复

### 场景 1: 编译错误

```
你: "我刚刚推送了代码，帮我检查构建并自动修复错误直到成功"

AI: [检查构建]
AI: "构建失败，错误是：..."
AI: [分析并修复]
AI: [提交并推送]
AI: [再次检查]
AI: "✅ 构建成功！"
```

### 场景 2: 链接错误

```
你: "自动修复构建错误直到成功"

AI: [完整自动化流程]
```

## 🔧 高级自动化

### 创建自动化脚本

创建一个 `.cursor/auto-build.sh` 脚本：

```bash
#!/bin/bash

# 完全自动化构建
while true; do
  # 1. 提交并推送
  git add -A
  git commit -m "Auto fix: $(date +%Y%m%d_%H%M%S)"
  git push origin dev
  
  # 2. 等待构建
  sleep 60
  
  # 3. 在 Cursor 中检查状态
  echo "请在 Cursor 中检查构建状态："
  echo "  '列出最新工作流程运行'"
  echo "  '如果失败，下载日志并修复'"
  
  read -p "构建成功了吗？(y/n): " success
  if [ "$success" = "y" ]; then
    echo "✅ 构建成功！"
    break
  fi
done
```

### 使用 Cursor AI 完全自动化

在 Cursor 中创建一个自动化任务：

```
你: "创建一个自动化任务：每次我推送代码到 dev 分支后，自动检查构建状态，如果失败就分析错误并修复，重复直到成功"
```

## 📊 监控和日志

### 查看构建历史

```
你: "列出 Build WinUnlock 工作流程的所有运行"
你: "列出状态为失败的工作流程运行"
```

### 分析失败原因

```
你: "分析最近 5 次失败的构建，找出共同原因"
```

## ⚙️ 配置

### 设置 GitHub Token

```bash
export GITHUB_TOKEN=your_token_here
```

### 调整检查间隔

在 `scripts/auto-build-until-success.js` 中修改：
```javascript
checkInterval: 30000, // 30秒
maxWaitTime: 600000,  // 10分钟
maxRetries: 10,       // 最多重试10次
```

## 🎯 最佳实践

1. **小步提交**：每次修复一个错误，便于定位问题
2. **描述性提交信息**：使用清晰的提交信息
3. **及时检查**：推送后立即检查构建状态
4. **利用 AI**：充分利用 Cursor AI 分析错误
5. **本地测试**：在本地先测试，减少失败次数

## 🚨 故障排除

### 构建一直失败

```
你: "分析这个构建错误并修复：[完整错误日志]"
```

### 无法获取工作流程运行

- 检查 GitHub Token 是否正确
- 确认仓库权限
- 检查网络连接

### 自动化脚本无法运行

- 确保已安装 Node.js
- 检查脚本权限：`chmod +x scripts/auto-build-until-success.js`
- 设置 GitHub Token

---

**现在你可以实现完全自动化的构建流程了！** 🎉

