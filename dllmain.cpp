#include "pch.h"
#include "CredentialProvider.h"

// DLL 引用计数
static LONG g_cRef = 0;

void DllAddRef()
{
    InterlockedIncrement(&g_cRef);
}

void DllRelease()
{
    InterlockedDecrement(&g_cRef);
}

class CWinUnlockProviderClassFactory : public IClassFactory
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CWinUnlockProviderClassFactory, IClassFactory),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
            delete this;
        return cRef;
    }

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
    {
        HRESULT hr = CLASS_E_NOAGGREGATION;
        if (pUnkOuter == nullptr)
        {
            hr = E_OUTOFMEMORY;
            WinUnlockProvider* pProvider = new(std::nothrow) WinUnlockProvider();
            if (pProvider)
            {
                hr = pProvider->QueryInterface(riid, ppv);
                pProvider->Release();
            }
        }
        return hr;
    }

    IFACEMETHODIMP LockServer(BOOL fLock)
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

    CWinUnlockProviderClassFactory() : _cRef(1)
    {
        DllAddRef();
    }

private:
    LONG _cRef;
};

STDAPI DllCanUnloadNow()
{
    return (g_cRef == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
    if (IsEqualGUID(rclsid, CLSID_WinUnlockProvider))
    {
        hr = E_OUTOFMEMORY;
        CWinUnlockProviderClassFactory* pClassFactory = new(std::nothrow) CWinUnlockProviderClassFactory();
        if (pClassFactory)
        {
            hr = pClassFactory->QueryInterface(riid, ppv);
            pClassFactory->Release();
        }
    }
    return hr;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

