# 自动检测构建结果使用指南

## 🎯 快速开始

### 方式 1: 等待构建完成（推荐）

```bash
# 提交并推送代码后
npm run wait-build
```

脚本会自动：
- ✅ 检测最新提交的构建状态
- ✅ 每 30 秒检查一次
- ✅ 显示构建进度
- ✅ 返回构建结果（成功/失败）

### 方式 2: 完全自动化

```bash
# 一键完成：提交 → 推送 → 等待 → 检查
npm run auto-build-wait "你的提交信息"
```

## 📋 详细使用

### 1. 等待构建完成

```bash
# 使用最新提交
npm run wait-build

# 或指定提交 SHA
node scripts/wait-for-build.js <commit_sha>
```

**输出示例：**
```
⏳ 等待 GitHub Actions 构建完成...
📌 提交 SHA: 02fe0cf
⏱️  检查间隔: 30秒
⏱️  最大等待时间: 600秒

⏳ 状态: in_progress (运行 #19604615521)
⏳ 状态: completed (运行 #19604615521)

✅ 构建成功！
🎉 构建成功！任务完成！
✅ 运行 ID: 19604615521
📦 应用已构建，可在 GitHub Actions 下载
🔗 URL: https://github.com/...
```

### 2. 在 Cursor 中使用

在 Cursor 中，你可以直接说：

```
"等待构建完成并检查结果"
```

AI 会自动运行：
```bash
npm run wait-build
```

### 3. 结合自动化修复

```bash
# 1. 修复代码后
git add -A
git commit -m "修复编译错误"
git push origin dev

# 2. 等待构建
npm run wait-build

# 3. 如果失败，在 Cursor 中说：
"下载工作流程运行 #<run_id> 的日志"
"根据这个构建错误修复代码"

# 4. 修复后重复步骤 1-2
```

## ⚙️ 配置

### 设置 GitHub Token（推荐）

```bash
export GITHUB_TOKEN=your_github_personal_access_token
```

获取 Token: https://github.com/settings/tokens/new
需要权限：`repo`, `actions:read`

### 调整检查间隔

编辑 `scripts/wait-for-build.js`：

```javascript
const CONFIG = {
  checkInterval: 30000,  // 30秒（默认）
  maxWaitTime: 600000,   // 10分钟（默认）
};
```

## 🔍 故障排除

### 问题 1: "未设置 GITHUB_TOKEN"

**解决：**
```bash
export GITHUB_TOKEN=your_token
```

### 问题 2: 构建超时

**解决：** 增加 `maxWaitTime` 或检查构建是否真的在运行

### 问题 3: 找不到构建

**解决：** 确保提交已推送，并且 GitHub Actions 已触发

## 📊 工作流程

```
1. 提交代码
   ↓
2. 推送到远程
   ↓
3. 运行 npm run wait-build
   ↓
4. 脚本自动检测构建状态
   ↓
5. 返回结果（成功/失败）
   ↓
6. 如果失败，获取错误日志并修复
   ↓
7. 重复步骤 1-5 直到成功
```

## 💡 提示

- 脚本会自动匹配最新提交的构建
- 如果构建还在进行中，脚本会持续等待
- 构建完成后，脚本会立即返回结果
- 建议设置 `GITHUB_TOKEN` 以获得更好的体验

