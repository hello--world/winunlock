#pragma once

// 确保包含必要的 Windows 头文件
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <unknwn.h>

// 包含凭据提供程序头文件（需要 Windows SDK 8.0+）
#include <credentialprovider.h>

#include <shlguid.h>
#include <shlobj.h>
#include <strsafe.h>
#include <wincrypt.h>
#include <comdef.h>
#include <ntsecapi.h>
#include <wincred.h>
#include <new>
#include <shlwapi.h>

// {A1B2C3D4-E5F6-4A5B-8C9D-0E1F2A3B4C5D}
DEFINE_GUID(CLSID_WinUnlockCredentialProvider,
    0xa1b2c3d4, 0xe5f6, 0x4a5b, 0x8c, 0x9d, 0xe, 0x1f, 0x2a, 0x3b, 0x4c, 0x5d);

// 凭据提供程序工厂类
class CWinUnlockProviderFactory : public IClassFactory
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv);
    IFACEMETHODIMP LockServer(BOOL fLock);

    CWinUnlockProviderFactory();
    ~CWinUnlockProviderFactory();

private:
    long _cRef;
};

// 凭据提供程序类
class CWinUnlockCredentialProvider : public ICredentialProvider
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // ICredentialProvider
    IFACEMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);
    IFACEMETHODIMP Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();
    IFACEMETHODIMP GetFieldDescriptorCount(DWORD* pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);
    IFACEMETHODIMP GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc);

    CWinUnlockCredentialProvider();
    ~CWinUnlockCredentialProvider();

private:
    long _cRef;
    CREDENTIAL_PROVIDER_USAGE_SCENARIO _cpus;
    ICredentialProviderEvents* _pcpe;
    UINT_PTR _upAdviseContext;
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR _rgFieldDescriptors[1];
    DWORD _dwFieldCount;
};

// 凭据类
class CWinUnlockCredential : public ICredentialProviderCredential
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // ICredentialProviderCredential
    IFACEMETHODIMP Advise(ICredentialProviderCredentialEvents* pcpc);
    IFACEMETHODIMP UnAdvise();
    IFACEMETHODIMP SetSelected(BOOL* pbAutoLogon);
    IFACEMETHODIMP SetDeselected();
    IFACEMETHODIMP GetFieldState(DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);
    IFACEMETHODIMP GetStringValue(DWORD dwFieldID, LPWSTR* ppsz);
    IFACEMETHODIMP GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp);
    IFACEMETHODIMP GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, LPWSTR* ppszLabel);
    IFACEMETHODIMP GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo);
    IFACEMETHODIMP SetStringValue(DWORD dwFieldID, LPCWSTR psz);
    IFACEMETHODIMP SetCheckboxValue(DWORD dwFieldID, BOOL bChecked);
    IFACEMETHODIMP CommandLinkClicked(DWORD dwFieldID);
    IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, LPWSTR* ppszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
    IFACEMETHODIMP ReportResult(NTSTATUS ntsStatus, NTSTATUS ntsSubstatus, LPWSTR* ppszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

    CWinUnlockCredential();
    ~CWinUnlockCredential();

    HRESULT Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgFieldDescriptors, const FIELD_STATE_PAIR* rgFieldStatePairs);

private:
    long _cRef;
    CREDENTIAL_PROVIDER_USAGE_SCENARIO _cpus;
    ICredentialProviderCredentialEvents* _pcpce;
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR _rgFieldDescriptors[1];
    FIELD_STATE_PAIR _rgFieldStatePairs[1];
    BOOL _bAutoLogon;
    
    // 自动解锁相关
    HRESULT GetStoredPassword(LPWSTR* ppszPassword);
    HRESULT PerformAutoUnlock();
};

// 辅助函数
HRESULT FieldDescriptorCoAllocCopy(const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rcpfd, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);
void FieldDescriptorFree(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd);

