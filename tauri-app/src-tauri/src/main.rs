// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use serde::{Deserialize, Serialize};
use winreg::enums::*;
use winreg::RegKey;

#[derive(Debug, Serialize, Deserialize)]
struct Config {
    username: String,
    password: String,
    auto_unlock_enabled: bool,
}

#[derive(Debug, Serialize, Deserialize)]
struct ConfigResponse {
    success: bool,
    message: String,
    config: Option<Config>,
}

const REGISTRY_PATH: &str = r"SOFTWARE\WinUnlock";

// 读取配置
#[tauri::command]
fn get_config() -> Result<ConfigResponse, String> {
    let hklm = RegKey::predef(HKEY_LOCAL_MACHINE);
    let config_key = match hklm.open_subkey(REGISTRY_PATH) {
        Ok(key) => key,
        Err(_) => {
            return Ok(ConfigResponse {
                success: false,
                message: "配置项不存在".to_string(),
                config: None,
            });
        }
    };

    let username: String = config_key
        .get_value("Username")
        .unwrap_or_else(|_| String::new());
    let password: String = config_key
        .get_value("Password")
        .unwrap_or_else(|_| String::new());
    let auto_unlock_enabled: u32 = config_key
        .get_value("AutoUnlockEnabled")
        .unwrap_or(1);

    Ok(ConfigResponse {
        success: true,
        message: "配置读取成功".to_string(),
        config: Some(Config {
            username,
            password,
            auto_unlock_enabled: auto_unlock_enabled != 0,
        }),
    })
}

// 保存配置
#[tauri::command]
fn save_config(username: String, password: String, auto_unlock_enabled: bool) -> Result<ConfigResponse, String> {
    let hklm = RegKey::predef(HKEY_LOCAL_MACHINE);
    
    // 尝试打开或创建配置项
    let (config_key, _) = match hklm.create_subkey(REGISTRY_PATH) {
        Ok(key) => key,
        Err(e) => {
            return Ok(ConfigResponse {
                success: false,
                message: format!("无法创建注册表项: {}", e),
                config: None,
            });
        }
    };

    // 写入配置
    if let Err(e) = config_key.set_value("Username", &username) {
        return Ok(ConfigResponse {
            success: false,
            message: format!("无法保存用户名: {}", e),
            config: None,
        });
    }

    if let Err(e) = config_key.set_value("Password", &password) {
        return Ok(ConfigResponse {
            success: false,
            message: format!("无法保存密码: {}", e),
            config: None,
        });
    }

    let enabled_value: u32 = if auto_unlock_enabled { 1 } else { 0 };
    if let Err(e) = config_key.set_value("AutoUnlockEnabled", &enabled_value) {
        return Ok(ConfigResponse {
            success: false,
            message: format!("无法保存自动解锁设置: {}", e),
            config: None,
        });
    }

    Ok(ConfigResponse {
        success: true,
        message: "配置保存成功".to_string(),
        config: Some(Config {
            username,
            password: "***".to_string(), // 不返回实际密码
            auto_unlock_enabled,
        }),
    })
}

// 测试配置
#[tauri::command]
fn test_config() -> Result<String, String> {
    let config_result = get_config()?;
    if let Some(config) = config_result.config {
        if config.username.is_empty() || config.password.is_empty() {
            return Ok("配置不完整，请填写用户名和密码".to_string());
        }
        Ok("配置有效".to_string())
    } else {
        Ok("未找到配置".to_string())
    }
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![get_config, save_config, test_config])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

