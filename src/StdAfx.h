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

typedef struct _SYSTEM_PERFORMANCE_INFORMATION
{
  LARGE_INTEGER IdleTime;
  LARGE_INTEGER ReadTransferCount;
  LARGE_INTEGER WriteTransferCount;
  LARGE_INTEGER OtherTransferCount;
  ULONG ReadOperationCount;
  ULONG WriteOperationCount;
  ULONG OtherOperationCount;
  ULONG AvailablePages;
  ULONG TotalCommittedPages;
  ULONG TotalCommitLimit;
  ULONG PeakCommitment;
  ULONG PageFaults;
  ULONG WriteCopyFaults;
  ULONG TransitionFaults;
  ULONG Reserved1;
  ULONG DemandZeroFaults;
  ULONG PagesRead;
  ULONG PageReadIos;
  ULONG CacheReadCount;
  ULONG CacheIoCount;
  ULONG PagefilePagesWritten;
  ULONG PagefilePageWriteIos;
  ULONG MappedFilePagesWritten;
  ULONG MappedFilePageWriteIos;
  ULONG PagedPoolUsage;
  ULONG NonPagedPoolUsage;
  ULONG PagedPoolAllocs;
  ULONG PagedPoolFrees;
  ULONG NonPagedPoolAllocs;
  ULONG NonPagedPoolFrees;
  ULONG TotalFreeSystemPtes;
  ULONG SystemCodePage;
  ULONG TotalSystemDriverPages;
  ULONG TotalSystemCodePages;
  ULONG SmallNonPagedLookasideListAllocateHits;
  ULONG SmallPagedLookasideListAllocateHits;
  ULONG Reserved3;
  ULONG MmSystemCachePage;
  ULONG PagedPoolPage;
  ULONG SystemDriverPage;
  ULONG FastReadNoWait;
  ULONG FastReadWait;
  ULONG FastReadResourceMiss;
  ULONG FastReadNotPossible;
  ULONG FastMdlReadNoWait;
  ULONG FastMdlReadWait;
  ULONG FastMdlReadResourceMiss;
  ULONG FastMdlReadNotPossible;
  ULONG MapDataNoWait;
  ULONG MapDataWait;
  ULONG MapDataNoWaitMiss;
  ULONG MapDataWaitMiss;
  ULONG PinMappedDataCount;
  ULONG PinReadNoWait;
  ULONG PinReadWait;
  ULONG PinReadNoWaitMiss;
  ULONG PinReadWaitMiss;
  ULONG CopyReadNoWait;
  ULONG CopyReadWait;
  ULONG CopyReadNoWaitMiss;
  ULONG CopyReadWaitMiss;
  ULONG MdlReadNoWait;
  ULONG MdlReadWait;
  ULONG MdlReadNoWaitMiss;
  ULONG MdlReadWaitMiss;
  ULONG ReadAheadIos;
  ULONG LazyWriteIos;
  ULONG LazyWritePages;
  ULONG DataFlushes;
  ULONG DataPages;
  ULONG ContextSwitches;
  ULONG FirstLevelTbFills;
  ULONG SecondLevelTbFills;
  ULONG SystemCalls;
} SYSTEM_PERFORMANCE_INFORMATION, *PSYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
  LARGE_INTEGER liKeBootTime;
  LARGE_INTEGER liKeSystemTime;
  LARGE_INTEGER liExpTimeZoneBias;
  ULONG uCurrentTimeZoneId;
  DWORD dwReserved;
} SYSTEM_TIME_INFORMATION;

extern "C" DWORD __stdcall NtQuerySystemInformation(DWORD, PVOID, ULONG, PULONG);
