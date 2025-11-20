#include "pch.h"
#include "Credential.h"
#include <lm.h>
#include <ntsecapi.h>

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "secur32.lib")

WinUnlockCredential::WinUnlockCredential() :
    _cRef(1),
    _cpus(CPUS_INVALID),
    _pcpce(nullptr),
    _pszUserSid(nullptr),
    _pszQualifiedUserName(nullptr),
    _bAutoSubmit(false)
{
    ZeroMemory(_rgFieldDescriptors, sizeof(_rgFieldDescriptors));
    ZeroMemory(_rgFieldIDs, sizeof(_rgFieldIDs));
}

WinUnlockCredential::~WinUnlockCredential()
{
    if (_pszUserSid)
    {
        CoTaskMemFree(_pszUserSid);
        _pszUserSid = nullptr;
    }
    if (_pszQualifiedUserName)
    {
        CoTaskMemFree(_pszQualifiedUserName);
        _pszQualifiedUserName = nullptr;
    }
}

// IUnknown
IFACEMETHODIMP_(ULONG) WinUnlockCredential::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) WinUnlockCredential::Release()
{
    LONG cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
        delete this;
    return cRef;
}

IFACEMETHODIMP WinUnlockCredential::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(WinUnlockCredential, ICredentialProviderCredential),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

HRESULT WinUnlockCredential::Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgFieldDescriptors, const FIELDID* rgFieldIDs)
{
    HRESULT hr = S_OK;
    _cpus = cpus;

    if (rgFieldDescriptors)
    {
        CopyMemory(_rgFieldDescriptors, rgFieldDescriptors, sizeof(_rgFieldDescriptors));
    }

    if (rgFieldIDs)
    {
        CopyMemory(_rgFieldIDs, rgFieldIDs, sizeof(_rgFieldIDs));
    }
    else
    {
        // 默认字段ID映射
        _rgFieldIDs[SFI_TILEIMAGE] = FIELDID_NONE;
        _rgFieldIDs[SFI_LARGE_TEXT] = FIELDID_NONE;
        _rgFieldIDs[SFI_SMALL_TEXT] = FIELDID_NONE;
        _rgFieldIDs[SFI_SUBMIT_BUTTON] = FIELDID_NONE;
    }

    // 初始化字段描述符
    _rgFieldDescriptors[SFI_TILEIMAGE].cpft = CPFT_TILE_IMAGE;
    _rgFieldDescriptors[SFI_TILEIMAGE].dwFieldID = SFI_TILEIMAGE;
    _rgFieldDescriptors[SFI_TILEIMAGE].pszLabel = nullptr;
    _rgFieldDescriptors[SFI_TILEIMAGE].guidFieldType = GUID_TILE_IMAGE;

    _rgFieldDescriptors[SFI_LARGE_TEXT].cpft = CPFT_LARGE_TEXT;
    _rgFieldDescriptors[SFI_LARGE_TEXT].dwFieldID = SFI_LARGE_TEXT;
    _rgFieldDescriptors[SFI_LARGE_TEXT].pszLabel = nullptr;
    _rgFieldDescriptors[SFI_LARGE_TEXT].guidFieldType = GUID_LARGE_TEXT;

    _rgFieldDescriptors[SFI_SMALL_TEXT].cpft = CPFT_SMALL_TEXT;
    _rgFieldDescriptors[SFI_SMALL_TEXT].dwFieldID = SFI_SMALL_TEXT;
    _rgFieldDescriptors[SFI_SMALL_TEXT].pszLabel = nullptr;
    _rgFieldDescriptors[SFI_SMALL_TEXT].guidFieldType = GUID_SMALL_TEXT;

    _rgFieldDescriptors[SFI_SUBMIT_BUTTON].cpft = CPFT_SUBMIT_BUTTON;
    _rgFieldDescriptors[SFI_SUBMIT_BUTTON].dwFieldID = SFI_SUBMIT_BUTTON;
    _rgFieldDescriptors[SFI_SUBMIT_BUTTON].pszLabel = nullptr;
    _rgFieldDescriptors[SFI_SUBMIT_BUTTON].guidFieldType = GUID_SUBMIT_BUTTON;

    return hr;
}

IFACEMETHODIMP WinUnlockCredential::Advise(ICredentialProviderCredentialEvents* pcpce)
{
    if (_pcpce != nullptr)
    {
        _pcpce->Release();
    }
    _pcpce = pcpce;
    if (_pcpce != nullptr)
    {
        _pcpce->AddRef();
    }
    return S_OK;
}

IFACEMETHODIMP WinUnlockCredential::UnAdvise()
{
    if (_pcpce)
    {
        _pcpce->Release();
        _pcpce = nullptr;
    }
    return S_OK;
}

IFACEMETHODIMP WinUnlockCredential::SetSelected(BOOL* pbAutoLogon)
{
    *pbAutoLogon = FALSE;

    // 检查是否可以自动解锁
    PWSTR pszUsername = nullptr;
    PWSTR pszPassword = nullptr;
    HRESULT hr = _GetAutoUnlockCredentials(&pszUsername, &pszPassword);
    if (SUCCEEDED(hr) && pszUsername && pszPassword)
    {
        *pbAutoLogon = TRUE;
        _bAutoSubmit = TRUE;
    }

    if (pszUsername)
    {
        CoTaskMemFree(pszUsername);
    }
    if (pszPassword)
    {
        CoTaskMemFree(pszPassword);
    }

    return S_OK;
}

IFACEMETHODIMP WinUnlockCredential::SetDeselected()
{
    _bAutoSubmit = FALSE;
    return S_OK;
}

IFACEMETHODIMP WinUnlockCredential::GetFieldState(DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis)
{
    HRESULT hr = E_INVALIDARG;

    if (pcpfs && pcpfis && (dwFieldID < SFI_NUM_FIELDS))
    {
        *pcpfs = CPFS_DISPLAY_IN_SELECTED_TILE;
        *pcpfis = CPFIS_NONE;

        switch (dwFieldID)
        {
        case SFI_TILEIMAGE:
            *pcpfs = CPFS_DISPLAY_IN_BOTH;
            break;
        case SFI_LARGE_TEXT:
        case SFI_SMALL_TEXT:
            *pcpfs = CPFS_DISPLAY_IN_SELECTED_TILE;
            break;
        case SFI_SUBMIT_BUTTON:
            *pcpfs = CPFS_DISPLAY_IN_SELECTED_TILE;
            break;
        }
        hr = S_OK;
    }
    return hr;
}

IFACEMETHODIMP WinUnlockCredential::GetStringValue(DWORD dwFieldID, LPWSTR* ppsz)
{
    HRESULT hr = E_INVALIDARG;

    if (ppsz && (dwFieldID < SFI_NUM_FIELDS))
    {
        *ppsz = nullptr;
        size_t cch = 0;
        PWSTR psz = nullptr;

        switch (dwFieldID)
        {
        case SFI_LARGE_TEXT:
            hr = SHStrDupW(L"自动解锁", &psz);
            break;
        case SFI_SMALL_TEXT:
            hr = SHStrDupW(L"使用预配置的凭据自动解锁系统", &psz);
            break;
        default:
            hr = E_INVALIDARG;
            break;
        }

        if (SUCCEEDED(hr))
        {
            *ppsz = psz;
        }
    }
    return hr;
}

IFACEMETHODIMP WinUnlockCredential::GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp)
{
    HRESULT hr = E_INVALIDARG;
    if (phbmp && (dwFieldID == SFI_TILEIMAGE))
    {
        *phbmp = nullptr;
        hr = S_OK;
    }
    return hr;
}

IFACEMETHODIMP WinUnlockCredential::GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, LPWSTR* ppszLabel)
{
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(pbChecked);
    UNREFERENCED_PARAMETER(ppszLabel);
    return E_NOTIMPL;
}

IFACEMETHODIMP WinUnlockCredential::GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo)
{
    HRESULT hr = E_INVALIDARG;
    if (pdwAdjacentTo && (dwFieldID == SFI_SUBMIT_BUTTON))
    {
        *pdwAdjacentTo = SFI_SMALL_TEXT;
        hr = S_OK;
    }
    return hr;
}

IFACEMETHODIMP WinUnlockCredential::SetStringValue(DWORD dwFieldID, LPCWSTR psz)
{
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(psz);
    return E_NOTIMPL;
}

IFACEMETHODIMP WinUnlockCredential::SetCheckboxValue(DWORD dwFieldID, BOOL bChecked)
{
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(bChecked);
    return E_NOTIMPL;
}

IFACEMETHODIMP WinUnlockCredential::CommandLinkClicked(DWORD dwFieldID)
{
    UNREFERENCED_PARAMETER(dwFieldID);
    return E_NOTIMPL;
}

IFACEMETHODIMP WinUnlockCredential::GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, LPWSTR* ppszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    HRESULT hr = E_UNEXPECTED;
    *pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;

    PWSTR pszUsername = nullptr;
    PWSTR pszPassword = nullptr;

    hr = _GetAutoUnlockCredentials(&pszUsername, &pszPassword);
    if (SUCCEEDED(hr) && pszUsername && pszPassword)
    {
        // 准备序列化凭据
        DWORD cbUsername = (DWORD)((wcslen(pszUsername) + 1) * sizeof(WCHAR));
        DWORD cbPassword = (DWORD)((wcslen(pszPassword) + 1) * sizeof(WCHAR));
        DWORD cbSerialization = sizeof(KERB_INTERACTIVE_UNLOCK_LOGON) + cbUsername + cbPassword;

        BYTE* pbSerialization = (BYTE*)CoTaskMemAlloc(cbSerialization);
        if (pbSerialization)
        {
            ZeroMemory(pbSerialization, cbSerialization);

            KERB_INTERACTIVE_UNLOCK_LOGON* pkiul = (KERB_INTERACTIVE_UNLOCK_LOGON*)pbSerialization;
            pkiul->Logon.MessageType = KerbInteractiveUnlockLogon;
            pkiul->Logon.LogonDomainName.Length = 0;
            pkiul->Logon.LogonDomainName.MaximumLength = 0;
            pkiul->Logon.LogonDomainName.Buffer = nullptr;

            // 设置用户名
            BYTE* pbTemp = pbSerialization + sizeof(KERB_INTERACTIVE_UNLOCK_LOGON);
            pkiul->Logon.UserName.Length = (USHORT)cbUsername - sizeof(WCHAR);
            pkiul->Logon.UserName.MaximumLength = (USHORT)cbUsername;
            pkiul->Logon.UserName.Buffer = (PWSTR)pbTemp;
            CopyMemory(pbTemp, pszUsername, cbUsername);

            // 设置密码
            pbTemp += cbUsername;
            pkiul->Logon.Password.Length = (USHORT)cbPassword - sizeof(WCHAR);
            pkiul->Logon.Password.MaximumLength = (USHORT)cbPassword;
            pkiul->Logon.Password.Buffer = (PWSTR)pbTemp;
            CopyMemory(pbTemp, pszPassword, cbPassword);

            // 填充序列化结构
            pcpcs->clsidCredentialProvider = CLSID_WinUnlockProvider;
            pcpcs->rgbSerialization = pbSerialization;
            pcpcs->cbSerialization = cbSerialization;
            pcpcs->ulAuthenticationPackage = 0; // 使用默认认证包

            *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (pszUsername)
    {
        CoTaskMemFree(pszUsername);
    }
    if (pszPassword)
    {
        CoTaskMemFree(pszPassword);
    }

    if (FAILED(hr))
    {
        *pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
        if (ppszOptionalStatusText)
        {
            SHStrDupW(L"无法获取自动解锁凭据", ppszOptionalStatusText);
        }
        if (pcpsiOptionalStatusIcon)
        {
            *pcpsiOptionalStatusIcon = CPSI_ERROR;
        }
    }

    return hr;
}

IFACEMETHODIMP WinUnlockCredential::ReportResult(NTSTATUS ntsStatus, NTSTATUS ntsSubstatus, LPWSTR* ppszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    UNREFERENCED_PARAMETER(ntsStatus);
    UNREFERENCED_PARAMETER(ntsSubstatus);
    UNREFERENCED_PARAMETER(ppszOptionalStatusText);
    UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);
    return S_OK;
}

HRESULT WinUnlockCredential::CanAutoUnlock()
{
    PWSTR pszUsername = nullptr;
    PWSTR pszPassword = nullptr;
    HRESULT hr = _GetAutoUnlockCredentials(&pszUsername, &pszPassword);
    
    if (pszUsername)
    {
        CoTaskMemFree(pszUsername);
    }
    if (pszPassword)
    {
        CoTaskMemFree(pszPassword);
    }
    
    return hr;
}

// 获取自动解锁凭据
// 注意：这是一个示例实现，实际使用时应该从安全存储中读取凭据
HRESULT WinUnlockCredential::_GetAutoUnlockCredentials(PWSTR* ppszUsername, PWSTR* ppszPassword)
{
    HRESULT hr = E_FAIL;

    // 方法1: 从注册表读取（仅用于演示，实际应使用更安全的方法）
    HKEY hKey = nullptr;
    LONG lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WinUnlock", 0, KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS)
    {
        WCHAR szUsername[256] = { 0 };
        WCHAR szPassword[256] = { 0 };
        DWORD dwType = REG_SZ;
        DWORD dwSize = sizeof(szUsername);

        if (RegQueryValueExW(hKey, L"Username", nullptr, &dwType, (LPBYTE)szUsername, &dwSize) == ERROR_SUCCESS)
        {
            dwSize = sizeof(szPassword);
            if (RegQueryValueExW(hKey, L"Password", nullptr, &dwType, (LPBYTE)szPassword, &dwSize) == ERROR_SUCCESS)
            {
                hr = SHStrDupW(szUsername, ppszUsername);
                if (SUCCEEDED(hr))
                {
                    hr = SHStrDupW(szPassword, ppszPassword);
                }
            }
        }
        RegCloseKey(hKey);
    }

    // 方法2: 如果注册表中没有，尝试从当前登录用户获取（仅用于解锁场景）
    if (FAILED(hr) && (_cpus == CPUS_UNLOCK_WORKSTATION))
    {
        // 获取当前锁定的用户名
        WCHAR szCurrentUser[256] = { 0 };
        DWORD dwSize = sizeof(szCurrentUser) / sizeof(WCHAR);
        if (GetUserNameW(szCurrentUser, &dwSize))
        {
            // 这里应该从安全存储中读取密码
            // 为了演示，我们假设密码已配置
            // 实际实现应该使用 Windows Credential Manager 或 DPAPI 加密存储
            hr = SHStrDupW(szCurrentUser, ppszUsername);
            if (SUCCEEDED(hr))
            {
                // 注意：实际应用中，密码应该从加密存储中读取
                // 这里仅作为示例，实际不应硬编码
                hr = SHStrDupW(L"", ppszPassword); // 空密码仅用于演示
            }
        }
    }

    return hr;
}

