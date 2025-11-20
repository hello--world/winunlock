#pragma once

#include "pch.h"

// 字段索引定义
enum FIELDID
{
    SFI_TILEIMAGE = 0,
    SFI_LARGE_TEXT,
    SFI_SMALL_TEXT,
    SFI_SUBMIT_BUTTON,
    SFI_NUM_FIELDS
};

class WinUnlockCredential : public ICredentialProviderCredential
{
public:
    // IUnknown
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);

    // ICredentialProviderCredential
    IFACEMETHODIMP Advise(ICredentialProviderCredentialEvents* pcpce);
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
    IFACEMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, LPWSTR* ppszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
    IFACEMETHODIMP ReportResult(NTSTATUS ntsStatus, NTSTATUS ntsSubstatus, LPWSTR* ppszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

    WinUnlockCredential();
    ~WinUnlockCredential();

    HRESULT Initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgFieldDescriptors, const FIELDID* rgFieldIDs);
    HRESULT CanAutoUnlock();

protected:
    LONG _cRef;
    CREDENTIAL_PROVIDER_USAGE_SCENARIO _cpus;
    ICredentialProviderCredentialEvents* _pcpce;
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR _rgFieldDescriptors[SFI_NUM_FIELDS];
    FIELDID _rgFieldIDs[SFI_NUM_FIELDS];
    PWSTR _pszUserSid;
    PWSTR _pszQualifiedUserName;
    bool _bAutoSubmit;
    
    HRESULT _GetAutoUnlockCredentials(PWSTR* ppszUsername, PWSTR* ppszPassword);
};

