#include "pch.h"
#include "CredentialProvider.h"

WinUnlockProvider::WinUnlockProvider() :
    _cRef(1),
    _cpus(CPUS_INVALID),
    _pcpe(nullptr),
    _upAdviseContext(0),
    _pCredential(nullptr),
    _dwFieldIDToSetFocus(0),
    _dwSetSerializationCred(CREDENTIAL_PROVIDER_NO_DEFAULT),
    _bAutoSubmit(false)
{
    DllAddRef();
    ZeroMemory(_rgFieldDescriptors, sizeof(_rgFieldDescriptors));
}

WinUnlockProvider::~WinUnlockProvider()
{
    if (_pCredential)
    {
        _pCredential->Release();
        _pCredential = nullptr;
    }
    DllRelease();
}

// IUnknown
IFACEMETHODIMP_(ULONG) WinUnlockProvider::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) WinUnlockProvider::Release()
{
    LONG cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
        delete this;
    return cRef;
}

IFACEMETHODIMP WinUnlockProvider::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(WinUnlockProvider, ICredentialProvider),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

// ICredentialProvider
IFACEMETHODIMP WinUnlockProvider::SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags)
{
    HRESULT hr = E_INVALIDARG;

    if ((cpus == CPUS_LOGON) || (cpus == CPUS_UNLOCK_WORKSTATION))
    {
        _cpus = cpus;
        hr = S_OK;
    }

    return hr;
}

IFACEMETHODIMP WinUnlockProvider::SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs)
{
    UNREFERENCED_PARAMETER(pcpcs);
    return E_NOTIMPL;
}

IFACEMETHODIMP WinUnlockProvider::Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext)
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

IFACEMETHODIMP WinUnlockProvider::UnAdvise()
{
    if (_pcpe)
    {
        _pcpe->Release();
        _pcpe = nullptr;
    }
    _upAdviseContext = 0;
    return S_OK;
}

IFACEMETHODIMP WinUnlockProvider::GetFieldDescriptorCount(DWORD* pdwCount)
{
    *pdwCount = SFI_NUM_FIELDS;
    return S_OK;
}

IFACEMETHODIMP WinUnlockProvider::GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
{
    HRESULT hr = E_INVALIDARG;
    if (ppcpfd && (dwIndex < SFI_NUM_FIELDS))
    {
        *ppcpfd = &_rgFieldDescriptors[dwIndex];
        hr = S_OK;
    }
    return hr;
}

IFACEMETHODIMP WinUnlockProvider::GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault)
{
    HRESULT hr = S_OK;

    if (!pdwCount || !pdwDefault || !pbAutoLogonWithDefault)
    {
        return E_INVALIDARG;
    }

    *pdwCount = 1;
    *pdwDefault = 0;
    *pbAutoLogonWithDefault = FALSE;

    // 创建凭据对象
    if (!_pCredential)
    {
        _pCredential = new(std::nothrow) WinUnlockCredential();
        if (_pCredential)
        {
            hr = _pCredential->Initialize(_cpus, _rgFieldDescriptors, nullptr);
            if (SUCCEEDED(hr))
            {
                // 检查是否可以自动解锁
                if (SUCCEEDED(_pCredential->CanAutoUnlock()))
                {
                    *pbAutoLogonWithDefault = TRUE;
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

IFACEMETHODIMP WinUnlockProvider::GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc)
{
    HRESULT hr = E_INVALIDARG;
    if ((dwIndex == 0) && ppcpc)
    {
        if (_pCredential)
        {
            hr = _pCredential->QueryInterface(IID_PPV_ARGS(ppcpc));
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    return hr;
}

