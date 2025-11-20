// 检查是否在 Tauri 环境中
const isTauri = typeof window.__TAURI__ !== 'undefined';

// 显示状态消息
function showStatus(message, type = 'info') {
    const statusEl = document.getElementById('statusMessage');
    statusEl.textContent = message;
    statusEl.className = `status-message show ${type}`;
    
    // 3秒后自动隐藏成功消息
    if (type === 'success') {
        setTimeout(() => {
            statusEl.classList.remove('show');
        }, 3000);
    }
}

// 加载配置
async function loadConfig() {
    if (!isTauri) {
        showStatus('此功能需要在 Tauri 应用中运行', 'error');
        return;
    }

    try {
        const { invoke } = window.__TAURI__.tauri;
        const response = await invoke('get_config');
        
        if (response.success && response.config) {
            document.getElementById('username').value = response.config.username;
            document.getElementById('password').value = response.config.password;
            document.getElementById('autoUnlockEnabled').checked = response.config.auto_unlock_enabled;
            showStatus('配置加载成功', 'success');
        } else {
            showStatus(response.message || '未找到配置', 'info');
        }
    } catch (error) {
        showStatus(`加载配置失败: ${error}`, 'error');
        console.error('Load config error:', error);
    }
}

// 保存配置
async function saveConfig(event) {
    event.preventDefault();
    
    if (!isTauri) {
        showStatus('此功能需要在 Tauri 应用中运行', 'error');
        return;
    }

    const username = document.getElementById('username').value.trim();
    const password = document.getElementById('password').value.trim();
    const autoUnlockEnabled = document.getElementById('autoUnlockEnabled').checked;

    if (!username || !password) {
        showStatus('请填写用户名和密码', 'error');
        return;
    }

    try {
        const { invoke } = window.__TAURI__.tauri;
        const response = await invoke('save_config', {
            username: username,
            password: password,
            autoUnlockEnabled: autoUnlockEnabled
        });

        if (response.success) {
            showStatus('配置保存成功！', 'success');
        } else {
            showStatus(`保存失败: ${response.message}`, 'error');
        }
    } catch (error) {
        showStatus(`保存配置失败: ${error}`, 'error');
        console.error('Save config error:', error);
    }
}

// 测试配置
async function testConfig() {
    if (!isTauri) {
        showStatus('此功能需要在 Tauri 应用中运行', 'error');
        return;
    }

    try {
        const { invoke } = window.__TAURI__.tauri;
        const message = await invoke('test_config');
        showStatus(message, message.includes('有效') ? 'success' : 'info');
    } catch (error) {
        showStatus(`测试失败: ${error}`, 'error');
        console.error('Test config error:', error);
    }
}

// 页面加载时的事件绑定
document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('configForm');
    const loadBtn = document.getElementById('loadBtn');
    const testBtn = document.getElementById('testBtn');

    form.addEventListener('submit', saveConfig);
    loadBtn.addEventListener('click', loadConfig);
    testBtn.addEventListener('click', testConfig);

    // 如果不在 Tauri 环境中，显示提示
    if (!isTauri) {
        showStatus('注意：当前在浏览器中运行，某些功能可能不可用。请在 Tauri 应用中运行以获得完整功能。', 'info');
    } else {
        // 自动加载配置
        loadConfig();
    }
});

