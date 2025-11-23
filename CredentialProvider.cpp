#include "CredentialProvider.h"
#include <lm.h>
#include <wtsapi32.h>
#include <userenv.h>
#include <sddl.h>
#include <ntsecapi.h>
#include <subauth.h>
#include <new>

// 前向声明 DLL 引用计数函数
extern void DllAddRef();
extern void DllRelease();

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "userenv.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "advapi32.lib")

// ============================================================================
// CWinUnlockProviderFactory 实现
// ============================================================================

CWinUnlockProviderFactory::CWinUnlockProviderFactory() : _cRef(1)
{
    DllAddRef();
}

CWinUnlockProviderFactory::~CWinUnlockProviderFactory()
{
    DllRelease();
}

IFACEMETHODIMP CWinUnlockProviderFactory::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CWinUnlockProviderFactory, IClassFactory),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) CWinUnlockProviderFactory::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) CWinUnlockProviderFactory::Release()
{
    long cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
        delete this;
    return cRef;
}

IFACEMETHODIMP CWinUnlockProviderFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
    HRESULT hr;
    
    if (pUnkOuter != nullptr)
    {
        return CLASS_E_NOAGGREGATION;
    }

    CWinUnlockCredentialProvider* pProvider = new(std::nothrow) CWinUnlockCredentialProvider();
    if (pProvider)
    {
        hr = pProvider->QueryInterface(riid, ppv);
        pProvider->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

IFACEMETHODIMP CWinUnlockProviderFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        DllAddRef();
    }
    else
    {
        DllRelease();
    }
    return S_OK;
}

// ============================================================================
// CWinUnlockCredentialProvider 实现
// ============================================================================

CWinUnlockCredentialProvider::CWinUnlockCredentialProvider() :
    _cRef(1),
    _cpus(CPUS_INVALID),
    _pcpe(nullptr),
    _upAdviseContext(0),
    _dwFieldCount(0)
{
    ZeroMemory(_rgFieldDescriptors, sizeof(_rgFieldDescriptors));
    DllAddRef();
}

CWinUnlockCredentialProvider::~CWinUnlockCredentialProvider()
{
    if (_pcpe)
    {
        _pcpe->Release();
    }
    
    for (DWORD i = 0; i < _dwFieldCount; i++)
    {
        CoTaskMemFree(_rgFieldDescriptors[i].pszLabel);
    }
    
    DllRelease();
}

IFACEMETHODIMP CWinUnlockCredentialProvider::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CWinUnlockCredentialProvider, ICredentialProvider),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) CWinUnlockCredentialProvider::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) CWinUnlockCredentialProvider::Release()
{
    long cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
        delete this;
    return cRef;
}

IFACEMETHODIMP CWinUnlockCredentialProvider::SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags)
{
    HRESULT hr = E_INVALIDARG;

    // 只支持解锁场景
    if ((cpus == CPUS_UNLOCK_WORKSTATION) || (cpus == CPUS_LOGON))
    {
        _cpus = cpus;
        _dwFieldCount = 1;
        
        // 设置字段描述符
        _rgFieldDescriptors[0].dwFieldID = 0;
        _rgFieldDescriptors[0].cpft = CPFT_SUBMIT_BUTTON;
        hr = SHStrDupW(L"自动解锁", &_rgFieldDescriptors[0].pszLabel);
        
        if (SUCCEEDED(hr))
        {
            hr = S_OK;
        }
    }
    
    return hr;
}

IFACEMETHODIMP CWinUnlockCredentialProvider::SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs)
{
    UNREFERENCED_PARAMETER(pcpcs);
    return E_NOTIMPL;
}

IFACEMETHODIMP CWinUnlockCredentialProvider::Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext)
{
    if (_pcpe != nullptr)
    {
        _pcpe->Release();
    }
    
    _pcpe = pcpe;
    _upAdviseContext = upAdviseContext;
    
    if (_pcpe != nullptr)
    {
        _pcpe->AddRef();
    }
    
    return S_OK;
}

IFACEMETHODIMP CWinUnlockCredentialProvider::UnAdvise()
{
    if (_pcpe)
    {
        _pcpe->Release();
        _pcpe = nullptr;
    }
    _upAdviseContext = 0;
    return S_OK;
}

IFACEMETHODIMP CWinUnlockCredentialProvider::GetFieldDescriptorCount(DWORD* pdwCount)
{
    *pdwCount = _dwFieldCount;
    return S_OK;
}

IFACEMETHODIMP CWinUnlockCredentialProvider::GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
{
    HRESULT hr = E_INVALIDARG;
    
    if ((dwIndex < _dwFieldCount) && (ppcpfd != nullptr))
    {
        hr = FieldDescriptorCoAllocCopy(&_rgFieldDescriptors[dwIndex], ppcpfd);
    }
    
    return hr;
}

IFACEMETHODIMP CWinUnlockCredentialProvider::GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault)
{
    *pdwCount = 1;
    *pdwDefault = 0;
    *pbAutoLogonWithDefault = TRUE;  // 启用自动登录
    
    return S_OK;
}

IFACEMETHODIMP CWinUnlockCredentialProvider::GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc)
{
    HRESULT hr = E_INVALIDARG;
    
    if ((dwIndex == 0) && (ppcpc != nullptr))
    {
        CWinUnlockCredential* pCredential = new(std::nothrow) CWinUnlockCredential();
        if (pCredential)
        {
            FIELD_STATE_PAIR rgfsp[1];
            rgfsp[0].cpfs = CPFS_DISPLAY_IN_SELECTED_TILE;
            rgfsp[0].cpfis = CPFIS_NONE;
            
            hr = pCredential->Initialize(_cpus, _rgFieldDescriptors, rgfsp);
            if (SUCCEEDED(hr))
            {
                hr = pCredential->QueryInterface(IID_PPV_ARGS(ppcpc));
            }
            pCredential->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    return hr;
}

// ============================================================================
// CWinUnlockCredential 实现
// ============================================================================

CWinUnlockCredential::CWinUnlockCredential() :
    _cRef(1),
    _cpus(CPUS_INVALID),
    _pcpce(nullptr),
    _bAutoLogon(FALSE)
{
    ZeroMemory(_rgFieldDescriptors, sizeof(_rgFieldDescriptors));
    ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
    DllAddRef();
}

CWinUnlockCredential::~CWinUnlockCredential()
{
    if (_pcpce)
    {
        _pcpce->Release();
    }
    
    for (DWORD i = 0; i < ARRAYSIZE(_rgFieldDescriptors); i++)
    {
        CoTaskMemFree(_rgFieldDescriptors[i].pszLabel);
    }
    
    DllRelease();
}

IFACEMETHODIMP CWinUnlockCredential::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CWinUnlockCredential, ICredentialProviderCredential),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) CWinUnlockCredential::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) CWinUnlockCredential::Release()
{
    long cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
        delete this;
    return cRef;
}

HRESULT CWinUnlockCredential::Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgFieldDescriptors, const FIELD_STATE_PAIR* rgFieldStatePairs)
{
    HRESULT hr = S_OK;
    
    _cpus = cpus;
    
    for (DWORD i = 0; i < ARRAYSIZE(_rgFieldDescriptors); i++)
    {
        SHStrDupW(rgFieldDescriptors[i].pszLabel, &_rgFieldDescriptors[i].pszLabel);
        _rgFieldDescriptors[i].cpft = rgFieldDescriptors[i].cpft;
        _rgFieldDescriptors[i].dwFieldID = rgFieldDescriptors[i].dwFieldID;
        _rgFieldDescriptors[i].guidFieldType = rgFieldDescriptors[i].guidFieldType;
        
        _rgFieldStatePairs[i] = rgFieldStatePairs[i];
    }
    
    return hr;
}

IFACEMETHODIMP CWinUnlockCredential::Advise(ICredentialProviderCredentialEvents* pcpc)
{
    if (_pcpce != nullptr)
    {
        _pcpce->Release();
    }
    
    _pcpce = pcpc;
    
    if (_pcpce != nullptr)
    {
        _pcpce->AddRef();
    }
    
    return S_OK;
}

IFACEMETHODIMP CWinUnlockCredential::UnAdvise()
{
    if (_pcpce)
    {
        _pcpce->Release();
        _pcpce = nullptr;
    }
    return S_OK;
}

IFACEMETHODIMP CWinUnlockCredential::SetSelected(BOOL* pbAutoLogon)
{
    // 检查是否有来自服务的解锁请求
    BOOL bUnlockRequested = CheckUnlockRequestFlag();
    
    if (bUnlockRequested)
    {
        // 如果服务请求解锁，启用自动登录
        *pbAutoLogon = TRUE;
        _bAutoLogon = TRUE;
        
        // 清除解锁请求标志
        ClearUnlockRequestFlag();
    }
    else
    {
        // 正常情况下的自动解锁
        *pbAutoLogon = TRUE;
        _bAutoLogon = TRUE;
    }
    
    // 尝试自动解锁
    PerformAutoUnlock();
    
    return S_OK;
}

IFACEMETHODIMP CWinUnlockCredential::SetDeselected()
{
    _bAutoLogon = FALSE;
    return S_OK;
}

IFACEMETHODIMP CWinUnlockCredential::GetFieldState(DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis)
{
    HRESULT hr = E_INVALIDARG;
    
    if (dwFieldID < ARRAYSIZE(_rgFieldStatePairs))
    {
        *pcpfs = _rgFieldStatePairs[dwFieldID].cpfs;
        *pcpfis = _rgFieldStatePairs[dwFieldID].cpfis;
        hr = S_OK;
    }
    
    return hr;
}

IFACEMETHODIMP CWinUnlockCredential::GetStringValue(DWORD dwFieldID, LPWSTR* ppsz)
{
    HRESULT hr = E_INVALIDARG;
    
    if (dwFieldID < ARRAYSIZE(_rgFieldDescriptors))
    {
        hr = SHStrDupW(_rgFieldDescriptors[dwFieldID].pszLabel, ppsz);
    }
    
    return hr;
}

IFACEMETHODIMP CWinUnlockCredential::GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp)
{
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(phbmp);
    return E_NOTIMPL;
}

IFACEMETHODIMP CWinUnlockCredential::GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, LPWSTR* ppszLabel)
{
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(pbChecked);
    UNREFERENCED_PARAMETER(pszLabel);
    return E_NOTIMPL;
}

IFACEMETHODIMP CWinUnlockCredential::GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo)
{
    HRESULT hr = E_INVALIDARG;
    
    if (dwFieldID == 0)
    {
        *pdwAdjacentTo = 0;
        hr = S_OK;
    }
    
    return hr;
}

IFACEMETHODIMP CWinUnlockCredential::SetStringValue(DWORD dwFieldID, LPCWSTR psz)
{
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(psz);
    return E_NOTIMPL;
}

IFACEMETHODIMP CWinUnlockCredential::SetCheckboxValue(DWORD dwFieldID, BOOL bChecked)
{
    UNREFERENCED_PARAMETER(dwFieldID);
    UNREFERENCED_PARAMETER(bChecked);
    return E_NOTIMPL;
}

IFACEMETHODIMP CWinUnlockCredential::CommandLinkClicked(DWORD dwFieldID)
{
    UNREFERENCED_PARAMETER(dwFieldID);
    return E_NOTIMPL;
}

IFACEMETHODIMP CWinUnlockCredential::GetSerialization(CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, LPWSTR* ppszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    HRESULT hr = E_INVALIDARG;
    LPWSTR pszPassword = nullptr;
    
    // 获取存储的密码
    hr = GetStoredPassword(&pszPassword);
    
    if (SUCCEEDED(hr) && pszPassword)
    {
        // 获取当前用户名
        WCHAR szUsername[UNLEN + 1] = {0};
        DWORD dwSize = UNLEN + 1;
        
        if (GetUserNameW(szUsername, &dwSize))
        {
            // 准备序列化凭据
            DWORD cbSerialization = 0;
            KERB_INTERACTIVE_UNLOCK_LOGON kiul;
            
            ZeroMemory(&kiul, sizeof(kiul));
            
            // 设置 Kerberos 交互式解锁登录结构
            kiul.Logon.MessageType = KerbInteractiveUnlockLogon;
            
            // 复制用户名
            hr = StringCbCopyW(kiul.Logon.UserName.Buffer, sizeof(kiul.Logon.UserName.Buffer), szUsername);
            if (SUCCEEDED(hr))
            {
                kiul.Logon.UserName.Length = (USHORT)(wcslen(szUsername) * sizeof(WCHAR));
                kiul.Logon.UserName.MaximumLength = kiul.Logon.UserName.Length + sizeof(WCHAR);
            }
            
            // 复制密码
            if (SUCCEEDED(hr))
            {
                hr = StringCbCopyW(kiul.Logon.Password.Buffer, sizeof(kiul.Logon.Password.Buffer), pszPassword);
                if (SUCCEEDED(hr))
                {
                    kiul.Logon.Password.Length = (USHORT)(wcslen(pszPassword) * sizeof(WCHAR));
                    kiul.Logon.Password.MaximumLength = kiul.Logon.Password.Length + sizeof(WCHAR);
                }
            }
            
            // 设置域名（本地账户使用计算机名）
            if (SUCCEEDED(hr))
            {
                WCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
                DWORD dwComputerNameSize = MAX_COMPUTERNAME_LENGTH + 1;
                
                if (GetComputerNameW(szComputerName, &dwComputerNameSize))
                {
                    hr = StringCbCopyW(kiul.Logon.LogonDomainName.Buffer, sizeof(kiul.Logon.LogonDomainName.Buffer), szComputerName);
                    if (SUCCEEDED(hr))
                    {
                        kiul.Logon.LogonDomainName.Length = (USHORT)(wcslen(szComputerName) * sizeof(WCHAR));
                        kiul.Logon.LogonDomainName.MaximumLength = kiul.Logon.LogonDomainName.Length + sizeof(WCHAR);
                    }
                }
            }
            
            // 序列化
            if (SUCCEEDED(hr))
            {
                hr = KerbInteractiveUnlockLogonPack(kiul, &pcpcs->rgbSerialization, &pcpcs->cbSerialization);
            }
            
            if (SUCCEEDED(hr))
            {
                pcpcs->clsidCredentialProvider = CLSID_WinUnlockCredentialProvider;
                
                // 获取 MSV1_0 认证包 ID（用于本地账户）
                ULONG ulAuthPackage = 0;
                LSA_STRING packageName;
                packageName.Buffer = const_cast<PCHAR>(MSV1_0_PACKAGE_NAME);
                packageName.Length = (USHORT)strlen(MSV1_0_PACKAGE_NAME);
                packageName.MaximumLength = packageName.Length + 1;
                
                HANDLE hLsa = nullptr;
                NTSTATUS status = LsaConnectUntrusted(&hLsa);
                if (NT_SUCCESS(status))
                {
                    status = LsaLookupAuthenticationPackage(hLsa, &packageName, &ulAuthPackage);
                    if (hLsa)
                    {
                        LsaDeregisterLogonProcess(hLsa);
                    }
                }
                
                if (NT_SUCCESS(status) && ulAuthPackage != 0)
                {
                    pcpcs->ulAuthenticationPackage = ulAuthPackage;
                }
                else
                {
                    // 如果获取失败，使用 MSV1_0 的默认值（通常是 2）
                    // 注意：这可能需要根据系统调整
                    pcpcs->ulAuthenticationPackage = 0;
                }
                
                *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
            }
        }
        
        SecureZeroMemory(pszPassword, wcslen(pszPassword) * sizeof(WCHAR));
        CoTaskMemFree(pszPassword);
    }
    
    if (FAILED(hr))
    {
        *pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
    }
    
    return hr;
}

IFACEMETHODIMP CWinUnlockCredential::ReportResult(NTSTATUS ntsStatus, NTSTATUS ntsSubstatus, LPWSTR* ppszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    UNREFERENCED_PARAMETER(ntsStatus);
    UNREFERENCED_PARAMETER(ntsSubstatus);
    UNREFERENCED_PARAMETER(ppszOptionalStatusText);
    UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);
    return S_OK;
}

// 从注册表或安全存储获取密码
HRESULT CWinUnlockCredential::GetStoredPassword(LPWSTR* ppszPassword)
{
    HRESULT hr = E_FAIL;
    HKEY hKey = nullptr;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    LPBYTE pData = nullptr;
    
    // 尝试从注册表读取加密的密码
    // 注意：实际应用中应该使用 Windows 凭据管理器或 DPAPI 加密存储
    LONG lResult = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\WinUnlock",
        0,
        KEY_READ,
        &hKey
    );
    
    if (lResult == ERROR_SUCCESS)
    {
        lResult = RegQueryValueExW(
            hKey,
            L"EncryptedPassword",
            nullptr,
            &dwType,
            nullptr,
            &dwSize
        );
        
        if (lResult == ERROR_SUCCESS && dwSize > 0)
        {
            pData = (LPBYTE)CoTaskMemAlloc(dwSize);
            if (pData)
            {
                lResult = RegQueryValueExW(
                    hKey,
                    L"EncryptedPassword",
                    nullptr,
                    &dwType,
                    pData,
                    &dwSize
                );
                
                if (lResult == ERROR_SUCCESS)
                {
                    // 使用 DPAPI 解密
                    DATA_BLOB DataIn = {0};
                    DATA_BLOB DataOut = {0};
                    
                    DataIn.pbData = pData;
                    DataIn.cbData = dwSize;
                    
                    if (CryptUnprotectData(&DataIn, nullptr, nullptr, nullptr, nullptr, 0, &DataOut))
                    {
                        // 确保分配足够的空间（包括 null 终止符）
                        DWORD dwPasswordLen = DataOut.cbData / sizeof(WCHAR);
                        *ppszPassword = (LPWSTR)CoTaskMemAlloc((dwPasswordLen + 1) * sizeof(WCHAR));
                        if (*ppszPassword)
                        {
                            memcpy(*ppszPassword, DataOut.pbData, DataOut.cbData);
                            (*ppszPassword)[dwPasswordLen] = L'\0';  // 确保 null 终止
                            hr = S_OK;
                        }
                        
                        SecureZeroMemory(DataOut.pbData, DataOut.cbData);
                        LocalFree(DataOut.pbData);
                    }
                }
                
                CoTaskMemFree(pData);
            }
        }
        
        RegCloseKey(hKey);
    }
    
    // 如果注册表中没有，尝试从 Windows 凭据管理器获取
    if (FAILED(hr))
    {
        // 这里可以添加从 Windows 凭据管理器读取的逻辑
        // 使用 CredRead API
    }
    
    return hr;
}

// 执行自动解锁
HRESULT CWinUnlockCredential::PerformAutoUnlock()
{
    // 这个方法在 SetSelected 时被调用
    // 实际的解锁逻辑在 GetSerialization 中完成
    return S_OK;
}

// 检查解锁请求标志
BOOL CWinUnlockCredential::CheckUnlockRequestFlag()
{
    HKEY hKey = nullptr;
    DWORD dwValue = 0;
    DWORD dwSize = sizeof(dwValue);
    DWORD dwType = REG_DWORD;
    BOOL bResult = FALSE;

    LONG lResult = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\WinUnlock",
        0,
        KEY_READ,
        &hKey
    );

    if (lResult == ERROR_SUCCESS)
    {
        lResult = RegQueryValueExW(
            hKey,
            L"UnlockRequest",
            nullptr,
            &dwType,
            (LPBYTE)&dwValue,
            &dwSize
        );

        if (lResult == ERROR_SUCCESS && dwType == REG_DWORD && dwValue != 0)
        {
            bResult = TRUE;
        }

        RegCloseKey(hKey);
    }

    return bResult;
}

// 清除解锁请求标志
BOOL CWinUnlockCredential::ClearUnlockRequestFlag()
{
    HKEY hKey = nullptr;
    LONG lResult = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\WinUnlock",
        0,
        KEY_WRITE,
        &hKey
    );

    if (lResult == ERROR_SUCCESS)
    {
        lResult = RegDeleteValueW(hKey, L"UnlockRequest");
        RegCloseKey(hKey);
    }

    return (lResult == ERROR_SUCCESS || lResult == ERROR_FILE_NOT_FOUND);
}

// ============================================================================
// 辅助函数
// ============================================================================

HRESULT FieldDescriptorCoAllocCopy(const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rcpfd, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
{
    HRESULT hr = S_OK;
    DWORD cbStruct = sizeof(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR);
    
    *ppcpfd = (CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR*)CoTaskMemAlloc(cbStruct);
    if (*ppcpfd)
    {
        (*ppcpfd)->dwFieldID = rcpfd->dwFieldID;
        (*ppcpfd)->cpft = rcpfd->cpft;
        (*ppcpfd)->guidFieldType = rcpfd->guidFieldType;
        
        if (rcpfd->pszLabel)
        {
            hr = SHStrDupW(rcpfd->pszLabel, &((*ppcpfd)->pszLabel));
        }
        else
        {
            (*ppcpfd)->pszLabel = nullptr;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

void FieldDescriptorFree(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd)
{
    if (pcpfd)
    {
        CoTaskMemFree(pcpfd->pszLabel);
        CoTaskMemFree(pcpfd);
    }
}

