// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use std::process::Command;
use winreg::enums::*;
use winreg::RegKey;
use windows::{
    core::*,
    Win32::Security::Cryptography::*,
    Win32::System::Memory::*,
    Win32::Foundation::*,
};

#[tauri::command]
fn set_password(password: String) -> Result<String, String> {
    // 使用 PowerShell 和 DPAPI 加密密码
    let password_escaped = password.replace('"', r#"\""#);
    let ps_script = format!(
        r#"
        $password = ConvertTo-SecureString -String "{}" -AsPlainText -Force
        $BSTR = [System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($password)
        $PlainPassword = [System.Runtime.InteropServices.Marshal]::PtrToStringAuto($BSTR)
        [System.Runtime.InteropServices.Marshal]::ZeroFreeBSTR($BSTR)
        
        $Entropy = $null
        $EncryptedData = [System.Security.Cryptography.ProtectedData]::Protect(
            [System.Text.Encoding]::Unicode.GetBytes($PlainPassword),
            $Entropy,
            [System.Security.Cryptography.DataProtectionScope]::LocalMachine
        )
        
        $EncryptedBase64 = [Convert]::ToBase64String($EncryptedData)
        
        if (-not (Test-Path "HKLM:\SOFTWARE\WinUnlock")) {{
            New-Item -Path "HKLM:\SOFTWARE\WinUnlock" -Force | Out-Null
        }}
        
        Set-ItemProperty -Path "HKLM:\SOFTWARE\WinUnlock" -Name "EncryptedPassword" -Value $EncryptedBase64 -Type String
        Write-Output "SUCCESS"
        "#,
        password_escaped
    );
    
    let output = Command::new("powershell")
        .arg("-Command")
        .arg(&ps_script)
        .output()
        .map_err(|e| format!("无法执行 PowerShell: {}", e))?;
    
    if output.status.success() {
        let result = String::from_utf8_lossy(&output.stdout);
        if result.contains("SUCCESS") {
            Ok("密码设置成功".to_string())
        } else {
            Err("密码设置失败".to_string())
        }
    } else {
        let error = String::from_utf8_lossy(&output.stderr);
        Err(format!("密码设置失败: {}", error))
    }
}

#[tauri::command]
fn get_current_user() -> Result<String, String> {
    let output = Command::new("whoami")
        .output()
        .map_err(|e| format!("无法获取当前用户: {}", e))?;
    
    let username = String::from_utf8(output.stdout)
        .map_err(|e| format!("无法解析用户名: {}", e))?;
    
    Ok(username.trim().to_string())
}

#[tauri::command]
fn get_computer_name() -> Result<String, String> {
    let output = Command::new("hostname")
        .output()
        .map_err(|e| format!("无法获取计算机名: {}", e))?;
    
    let hostname = String::from_utf8(output.stdout)
        .map_err(|e| format!("无法解析计算机名: {}", e))?;
    
    Ok(hostname.trim().to_string())
}

#[tauri::command]
fn check_password_set() -> Result<bool, String> {
    let hkcu = RegKey::predef(HKEY_LOCAL_MACHINE);
    let key = hkcu.open_subkey("SOFTWARE\\WinUnlock")
        .map_err(|_| "注册表项不存在".to_string())?;
    
    let password: Result<String, _> = key.get_value("EncryptedPassword");
    Ok(password.is_ok())
}

#[tauri::command]
fn register_dll(dll_path: String) -> Result<String, String> {
    let output = Command::new("regsvr32")
        .arg("/s")
        .arg(&dll_path)
        .output()
        .map_err(|e| format!("无法执行 regsvr32: {}", e))?;
    
    if output.status.success() {
        Ok("DLL 注册成功".to_string())
    } else {
        let error = String::from_utf8_lossy(&output.stderr);
        Err(format!("DLL 注册失败: {}", error))
    }
}

#[tauri::command]
fn unregister_dll(dll_path: String) -> Result<String, String> {
    let output = Command::new("regsvr32")
        .arg("/s")
        .arg("/u")
        .arg(&dll_path)
        .output()
        .map_err(|e| format!("无法执行 regsvr32: {}", e))?;
    
    if output.status.success() {
        Ok("DLL 卸载成功".to_string())
    } else {
        let error = String::from_utf8_lossy(&output.stderr);
        Err(format!("DLL 卸载失败: {}", error))
    }
}

#[tauri::command]
fn is_admin() -> Result<bool, String> {
    // 检查是否以管理员权限运行
    let output = Command::new("net")
        .arg("session")
        .output();
    
    Ok(output.is_ok() && output.unwrap().status.success())
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![
            set_password,
            get_current_user,
            get_computer_name,
            check_password_set,
            register_dll,
            unregister_dll,
            is_admin
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

