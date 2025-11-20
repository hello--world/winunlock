// Local copy of credentialprovider.h for GitHub Actions compatibility
// This file contains the necessary interface definitions for Credential Provider

#pragma once

#include <windows.h>
#include <objbase.h>
#include <unknwn.h>
#include <winternl.h>

// NTSTATUS definition
#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

// Forward declarations
interface ICredentialProvider;
interface ICredentialProviderCredential;
interface ICredentialProviderEvents;
interface ICredentialProviderCredentialEvents;

// Enumerations
typedef enum _CREDENTIAL_PROVIDER_USAGE_SCENARIO
{
    CPUS_INVALID = 0,
    CPUS_LOGON = 1,
    CPUS_UNLOCK_WORKSTATION = 2,
    CPUS_CHANGE_PASSWORD = 3,
    CPUS_CREDUI = 4,
    CPUS_PLAP = 5
} CREDENTIAL_PROVIDER_USAGE_SCENARIO;

typedef enum _CREDENTIAL_PROVIDER_FIELD_TYPE
{
    CPFT_INVALID = 0,
    CPFT_LARGE_TEXT = 1,
    CPFT_SMALL_TEXT = 2,
    CPFT_COMMAND_LINK = 3,
    CPFT_EDIT_TEXT = 4,
    CPFT_PASSWORD_TEXT = 5,
    CPFT_TILE_IMAGE = 6,
    CPFT_CHECKBOX = 7,
    CPFT_SUBMIT_BUTTON = 8,
    CPFT_COMBOBOX = 9,
    CPFT_RADIOBUTTON = 10,
    CPFT_EDIT_TEXT_PASSWORD = 11
} CREDENTIAL_PROVIDER_FIELD_TYPE;

typedef enum _CREDENTIAL_PROVIDER_FIELD_STATE
{
    CPFS_HIDDEN = 0,
    CPFS_DISPLAY_IN_SELECTED_TILE = 1,
    CPFS_DISPLAY_IN_DESELECTED_TILE = 2,
    CPFS_DISPLAY_IN_BOTH = 3
} CREDENTIAL_PROVIDER_FIELD_STATE;

typedef enum _CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE
{
    CPFIS_NONE = 0,
    CPFIS_READONLY = 1,
    CPFIS_DISABLED = 2,
    CPFIS_FOCUSED = 3
} CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE;

typedef enum _CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE
{
    CPGSR_NO_CREDENTIAL_NOT_FINISHED = 0,
    CPGSR_NO_CREDENTIAL_FINISHED = 1,
    CPGSR_RETURN_CREDENTIAL_FINISHED = 2,
    CPGSR_RETURN_NO_CREDENTIAL_FINISHED = 3
} CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE;

typedef enum _CREDENTIAL_PROVIDER_STATUS_ICON
{
    CPSI_NONE = 0,
    CPSI_ERROR = 1,
    CPSI_WARNING = 2,
    CPSI_SUCCESS = 3
} CREDENTIAL_PROVIDER_STATUS_ICON;

// Constants
#define CREDENTIAL_PROVIDER_NO_DEFAULT ((DWORD)-1)

// GUIDs
DEFINE_GUID(GUID_TILE_IMAGE, 0x2d837775, 0x8de1, 0x4e3e, 0xb2, 0x98, 0x55, 0xfc, 0x6e, 0xec, 0x0c, 0x0a);
DEFINE_GUID(GUID_LARGE_TEXT, 0xe7179c38, 0xbd07, 0x4708, 0x8c, 0x8c, 0x8b, 0xcb, 0x40, 0xc3, 0x97, 0x50);
DEFINE_GUID(GUID_SMALL_TEXT, 0x6f45dc1e, 0x5384, 0x457a, 0xbc, 0xc7, 0x4f, 0x95, 0x0f, 0x0e, 0x3e, 0xfb);
DEFINE_GUID(GUID_SUBMIT_BUTTON, 0x30574de6, 0x3532, 0x4830, 0xbd, 0x23, 0x08, 0x34, 0x72, 0xbd, 0x58, 0x93);

// Structures
typedef struct _CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR
{
    DWORD dwFieldID;
    CREDENTIAL_PROVIDER_FIELD_TYPE cpft;
    LPWSTR pszLabel;
    GUID guidFieldType;
} CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR;

typedef struct _CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION
{
    ULONG ulAuthenticationPackage;
    GUID clsidCredentialProvider;
    ULONG cbSerialization;
    BYTE* rgbSerialization;
} CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION;

// Interface definitions
#undef INTERFACE
#define INTERFACE ICredentialProviderEvents
DECLARE_INTERFACE_(ICredentialProviderEvents, IUnknown)
{
    STDMETHOD(OnCredentialsChanged)(THIS_ UINT_PTR upAdviseContext) PURE;
};
#undef INTERFACE

#undef INTERFACE
#define INTERFACE ICredentialProviderCredentialEvents
DECLARE_INTERFACE_(ICredentialProviderCredentialEvents, IUnknown)
{
    STDMETHOD(SetFieldState)(THIS_ DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE cpfs) PURE;
    STDMETHOD(SetFieldInteractiveState)(THIS_ DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE cpfis) PURE;
    STDMETHOD(SetFieldString)(THIS_ DWORD dwFieldID, LPCWSTR psz) PURE;
    STDMETHOD(SetFieldCheckbox)(THIS_ DWORD dwFieldID, BOOL bChecked, LPCWSTR pszLabel) PURE;
    STDMETHOD(SetFieldBitmap)(THIS_ DWORD dwFieldID, HBITMAP hbmp) PURE;
    STDMETHOD(SetFieldComboBoxSelectedItem)(THIS_ DWORD dwFieldID, DWORD dwSelectedItem) PURE;
    STDMETHOD(DeleteFieldComboBoxItem)(THIS_ DWORD dwFieldID, DWORD dwItem) PURE;
    STDMETHOD(AppendFieldComboBoxItem)(THIS_ DWORD dwFieldID, LPCWSTR pszItem) PURE;
    STDMETHOD(SetFieldSubmitButton)(THIS_ DWORD dwFieldID, DWORD dwAdjacentTo) PURE;
    STDMETHOD(OnCreatingWindow)(THIS_ HWND* phwndOwner) PURE;
};
#undef INTERFACE

#undef INTERFACE
#define INTERFACE ICredentialProviderCredential
DECLARE_INTERFACE_(ICredentialProviderCredential, IUnknown)
{
    STDMETHOD(Advise)(THIS_ ICredentialProviderCredentialEvents* pcpce) PURE;
    STDMETHOD(UnAdvise)(THIS) PURE;
    STDMETHOD(SetSelected)(THIS_ BOOL* pbAutoLogon) PURE;
    STDMETHOD(SetDeselected)(THIS) PURE;
    STDMETHOD(GetFieldState)(THIS_ DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis) PURE;
    STDMETHOD(GetStringValue)(THIS_ DWORD dwFieldID, LPWSTR* ppsz) PURE;
    STDMETHOD(GetBitmapValue)(THIS_ DWORD dwFieldID, HBITMAP* phbmp) PURE;
    STDMETHOD(GetCheckboxValue)(THIS_ DWORD dwFieldID, BOOL* pbChecked, LPWSTR* ppszLabel) PURE;
    STDMETHOD(GetSubmitButtonValue)(THIS_ DWORD dwFieldID, DWORD* pdwAdjacentTo) PURE;
    STDMETHOD(SetStringValue)(THIS_ DWORD dwFieldID, LPCWSTR psz) PURE;
    STDMETHOD(SetCheckboxValue)(THIS_ DWORD dwFieldID, BOOL bChecked) PURE;
    STDMETHOD(CommandLinkClicked)(THIS_ DWORD dwFieldID) PURE;
    STDMETHOD(GetSerialization)(THIS_ CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, LPWSTR* ppszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon) PURE;
    STDMETHOD(ReportResult)(THIS_ NTSTATUS ntsStatus, NTSTATUS ntsSubstatus, LPWSTR* ppszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon) PURE;
};
#undef INTERFACE

#undef INTERFACE
#define INTERFACE ICredentialProvider
DECLARE_INTERFACE_(ICredentialProvider, IUnknown)
{
    STDMETHOD(SetUsageScenario)(THIS_ CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags) PURE;
    STDMETHOD(SetSerialization)(THIS_ const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs) PURE;
    STDMETHOD(Advise)(THIS_ ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext) PURE;
    STDMETHOD(UnAdvise)(THIS) PURE;
    STDMETHOD(GetFieldDescriptorCount)(THIS_ DWORD* pdwCount) PURE;
    STDMETHOD(GetFieldDescriptorAt)(THIS_ DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd) PURE;
    STDMETHOD(GetCredentialCount)(THIS_ DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault) PURE;
    STDMETHOD(GetCredentialAt)(THIS_ DWORD dwIndex, ICredentialProviderCredential** ppcpc) PURE;
};
#undef INTERFACE
