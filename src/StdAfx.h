// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER		0x0500
//#define _WIN32_WINNT	0x0400
#define _WIN32_IE	0x0400

// Memory debugging includes
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>
#endif

// C Includes
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Windows Includes
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <tchar.h>
#include <PDHMsg.h>
#include <netfw.h>
#include <msi.h>
#include <Shlwapi.h>

// C++ Includes
#include <string>
#include <vector>
#include <map>

// Includes for plugins
#include <Pdh.h>
#include <Tlhelp32.h>
#include <Iphlpapi.h>
#include <DelayImp.h>
 