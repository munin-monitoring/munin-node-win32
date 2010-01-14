//--------------------------------------------------------------------------------------
// File: FirewallInstallHelper.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// Uncomment to get a debug messagebox
//#define SHOW_DEBUG_MSGBOXES

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
STDAPI AddApplicationToExceptionListW( WCHAR* strGameExeFullPath, WCHAR* strFriendlyAppName );
STDAPI RemoveApplicationFromExceptionListW( WCHAR* strGameExeFullPath );
STDAPI AddApplicationToExceptionListA( CHAR* strGameExeFullPath, CHAR* strFriendlyAppName );
STDAPI RemoveApplicationFromExceptionListA( CHAR* strGameExeFullPath );
LPWSTR GetPropertyFromMSI( MSIHANDLE hMSI, LPCWSTR szPropName );
INetFwProfile* GetFirewallProfile();

#ifdef _UNICODE
#define AddApplicationToExceptionList AddApplicationToExceptionListW
#else
#define AddApplicationToExceptionList AddApplicationToExceptionListA
#endif


#ifdef _UNICODE
#define RemoveApplicationFromExceptionList RemoveApplicationFromExceptionListW
#else
#define RemoveApplicationFromExceptionList RemoveApplicationFromExceptionListA
#endif

LPWSTR GetPropertyFromMSI( MSIHANDLE hMSI, LPCWSTR szPropName );
INetFwProfile* GetFirewallProfile();