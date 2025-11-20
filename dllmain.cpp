#include <windows.h>
#include <new>
#include "CredentialProvider.h"

HINSTANCE g_hinst = nullptr;

// DLL 引用计数
static volatile long g_cRef = 0;

void DllAddRef()
{
    InterlockedIncrement(&g_cRef);
}

void DllRelease()
{
    InterlockedDecrement(&g_cRef);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hinst = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// COM 导出函数
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
    
    if (IsEqualCLSID(rclsid, CLSID_WinUnlockCredentialProvider))
    {
        CWinUnlockProviderFactory* pFactory = new(std::nothrow) CWinUnlockProviderFactory();
        if (pFactory)
        {
            hr = pFactory->QueryInterface(riid, ppv);
            pFactory->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    return hr;
}

STDAPI DllCanUnloadNow()
{
    return (g_cRef > 0) ? S_FALSE : S_OK;
}

// 注册表注册函数
STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;
    WCHAR szModulePath[MAX_PATH];
    
    if (GetModuleFileNameW(g_hinst, szModulePath, ARRAYSIZE(szModulePath)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    
    if (SUCCEEDED(hr))
    {
        // 注册 CLSID
        WCHAR szKeyName[256];
        HKEY hKey = nullptr;
        
        StringCchPrintfW(szKeyName, ARRAYSIZE(szKeyName), L"CLSID\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            CLSID_WinUnlockCredentialProvider.Data1,
            CLSID_WinUnlockCredentialProvider.Data2,
            CLSID_WinUnlockCredentialProvider.Data3,
            CLSID_WinUnlockCredentialProvider.Data4[0],
            CLSID_WinUnlockCredentialProvider.Data4[1],
            CLSID_WinUnlockCredentialProvider.Data4[2],
            CLSID_WinUnlockCredentialProvider.Data4[3],
            CLSID_WinUnlockCredentialProvider.Data4[4],
            CLSID_WinUnlockCredentialProvider.Data4[5],
            CLSID_WinUnlockCredentialProvider.Data4[6],
            CLSID_WinUnlockCredentialProvider.Data4[7]);
        
        LONG lResult = RegCreateKeyExW(
            HKEY_CLASSES_ROOT,
            szKeyName,
            0,
            nullptr,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            nullptr,
            &hKey,
            nullptr
        );
        
        if (lResult == ERROR_SUCCESS)
        {
            RegSetValueExW(hKey, nullptr, 0, REG_SZ, (LPBYTE)L"WinUnlock Credential Provider", sizeof(L"WinUnlock Credential Provider"));
            RegCloseKey(hKey);
            
            // 注册 InprocServer32
            StringCchPrintfW(szKeyName, ARRAYSIZE(szKeyName), L"CLSID\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\\InprocServer32",
                CLSID_WinUnlockCredentialProvider.Data1,
                CLSID_WinUnlockCredentialProvider.Data2,
                CLSID_WinUnlockCredentialProvider.Data3,
                CLSID_WinUnlockCredentialProvider.Data4[0],
                CLSID_WinUnlockCredentialProvider.Data4[1],
                CLSID_WinUnlockCredentialProvider.Data4[2],
                CLSID_WinUnlockCredentialProvider.Data4[3],
                CLSID_WinUnlockCredentialProvider.Data4[4],
                CLSID_WinUnlockCredentialProvider.Data4[5],
                CLSID_WinUnlockCredentialProvider.Data4[6],
                CLSID_WinUnlockCredentialProvider.Data4[7]);
            
            lResult = RegCreateKeyExW(
                HKEY_CLASSES_ROOT,
                szKeyName,
                0,
                nullptr,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                nullptr,
                &hKey,
                nullptr
            );
            
            if (lResult == ERROR_SUCCESS)
            {
                RegSetValueExW(hKey, nullptr, 0, REG_SZ, (LPBYTE)szModulePath, (DWORD)((wcslen(szModulePath) + 1) * sizeof(WCHAR)));
                RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, (LPBYTE)L"Apartment", sizeof(L"Apartment"));
                RegCloseKey(hKey);
            }
            
            // 注册到凭据提供程序列表
            lResult = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers",
                0,
                KEY_WRITE,
                &hKey
            );
            
            if (lResult == ERROR_SUCCESS)
            {
                StringCchPrintfW(szKeyName, ARRAYSIZE(szKeyName), L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                    CLSID_WinUnlockCredentialProvider.Data1,
                    CLSID_WinUnlockCredentialProvider.Data2,
                    CLSID_WinUnlockCredentialProvider.Data3,
                    CLSID_WinUnlockCredentialProvider.Data4[0],
                    CLSID_WinUnlockCredentialProvider.Data4[1],
                    CLSID_WinUnlockCredentialProvider.Data4[2],
                    CLSID_WinUnlockCredentialProvider.Data4[3],
                    CLSID_WinUnlockCredentialProvider.Data4[4],
                    CLSID_WinUnlockCredentialProvider.Data4[5],
                    CLSID_WinUnlockCredentialProvider.Data4[6],
                    CLSID_WinUnlockCredentialProvider.Data4[7]);
                
                RegSetValueExW(hKey, szKeyName, 0, REG_SZ, (LPBYTE)L"WinUnlock Credential Provider", sizeof(L"WinUnlock Credential Provider"));
                RegCloseKey(hKey);
            }
        }
    }
    
    return hr;
}

STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;
    WCHAR szKeyName[256];
    
    // 从凭据提供程序列表中删除
    HKEY hKey = nullptr;
    LONG lResult = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers",
        0,
        KEY_WRITE,
        &hKey
    );
    
    if (lResult == ERROR_SUCCESS)
    {
        StringCchPrintfW(szKeyName, ARRAYSIZE(szKeyName), L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            CLSID_WinUnlockCredentialProvider.Data1,
            CLSID_WinUnlockCredentialProvider.Data2,
            CLSID_WinUnlockCredentialProvider.Data3,
            CLSID_WinUnlockCredentialProvider.Data4[0],
            CLSID_WinUnlockCredentialProvider.Data4[1],
            CLSID_WinUnlockCredentialProvider.Data4[2],
            CLSID_WinUnlockCredentialProvider.Data4[3],
            CLSID_WinUnlockCredentialProvider.Data4[4],
            CLSID_WinUnlockCredentialProvider.Data4[5],
            CLSID_WinUnlockCredentialProvider.Data4[6],
            CLSID_WinUnlockCredentialProvider.Data4[7]);
        
        RegDeleteValueW(hKey, szKeyName);
        RegCloseKey(hKey);
    }
    
    // 删除 CLSID 注册
    StringCchPrintfW(szKeyName, ARRAYSIZE(szKeyName), L"CLSID\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        CLSID_WinUnlockCredentialProvider.Data1,
        CLSID_WinUnlockCredentialProvider.Data2,
        CLSID_WinUnlockCredentialProvider.Data3,
        CLSID_WinUnlockCredentialProvider.Data4[0],
        CLSID_WinUnlockCredentialProvider.Data4[1],
        CLSID_WinUnlockCredentialProvider.Data4[2],
        CLSID_WinUnlockCredentialProvider.Data4[3],
        CLSID_WinUnlockCredentialProvider.Data4[4],
        CLSID_WinUnlockCredentialProvider.Data4[5],
        CLSID_WinUnlockCredentialProvider.Data4[6],
        CLSID_WinUnlockCredentialProvider.Data4[7]);
    
    // 递归删除注册表项
    // 使用 RegDeleteTreeW（Windows Vista+）或手动删除
    HKEY hKey = nullptr;
    LONG lResult = RegOpenKeyExW(HKEY_CLASSES_ROOT, szKeyName, 0, KEY_ALL_ACCESS, &hKey);
    if (lResult == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        // 递归删除子项
        lResult = RegDeleteTreeW(HKEY_CLASSES_ROOT, szKeyName);
    }
    
    return hr;
}

