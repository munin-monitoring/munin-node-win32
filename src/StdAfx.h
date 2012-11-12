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

typedef struct
{
  DWORD dwUnknown1;
  ULONG uKeMaximumIncrement;
  ULONG uPageSize;
  ULONG uMmNumberOfPhysicalPages;
  ULONG uMmLowestPhysicalPage;
  ULONG uMmHighestPhysicalPage;
  ULONG uAllocationGranularity;
  PVOID pLowestUserAddress;
  PVOID pMmHighestUserAddress;
  ULONG uKeActiveProcessors;
  BYTE bKeNumberProcessors;
  BYTE bUnknown2;
  WORD wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
  LARGE_INTEGER liIdleTime;
  DWORD dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
  LARGE_INTEGER liKeBootTime;
  LARGE_INTEGER liKeSystemTime;
  LARGE_INTEGER liExpTimeZoneBias;
  ULONG uCurrentTimeZoneId;
  DWORD dwReserved;
} SYSTEM_TIME_INFORMATION;

extern "C" DWORD __stdcall NtQuerySystemInformation(DWORD, PVOID, ULONG, PULONG);
