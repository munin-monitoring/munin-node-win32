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

typedef struct _SYSTEM_BASIC_INFORMATION {
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
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

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

typedef struct _SYSTEM_TIMEOFDAY_INFORMATION {
  LARGE_INTEGER liKeBootTime;
  LARGE_INTEGER liKeSystemTime;
  LARGE_INTEGER liExpTimeZoneBias;
  ULONG uCurrentTimeZoneId;
  DWORD dwReserved;
} SYSTEM_TIMEOFDAY_INFORMATION, *PSYSTEM_TIMEOFDAY_INFORMATION; /* was SYSTEM_TIME_INFORMATION */;

typedef struct _SYSTEM_INTERRUPT_INFORMATION {
  ULONG ContextSwitches;
  ULONG DpcCount; /* FIXME */
  ULONG DpcRate; /* FIXME */
  ULONG TimeIncrement;
  ULONG DpcBypassCount; /* FIXME */
  ULONG ApcBypassCount; /* FIXME */
} SYSTEM_INTERRUPT_INFORMATION, *PSYSTEM_INTERRUPT_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
  SystemBasicInformation = 0,
  SystemPerformanceInformation = 2,
  SystemTimeOfDayInformation = 3,
  SystemProcessInformation = 5,
  SystemProcessorPerformanceInformation = 8,
  SystemModuleInformation = 11,
  SystemHandleInformation = 16,
  SystemPageFileInformation = 18,
  SystemCacheInformation = 21,
  SystemInterruptInformation = 23,
  SystemDpcBehaviourInformation = 24,
  SystemFullMemoryInformation = 25,
  SystemNotImplemented6 = 25,
  SystemLoadImage = 26,
  SystemUnloadImage = 27,
  SystemTimeAdjustmentInformation = 28,
  SystemTimeAdjustment = 28,
  SystemSummaryMemoryInformation = 29,
  SystemNotImplemented7 = 29,
  SystemNextEventIdInformation = 30,
  SystemEventIdsInformation = 31,
  SystemCrashDumpInformation = 32,
  SystemExceptionInformation = 33,
  SystemCrashDumpStateInformation = 34,
  SystemKernelDebuggerInformation = 35,
  SystemContextSwitchInformation = 36,
  SystemRegistryQuotaInformation = 37,
  SystemCurrentTimeZoneInformation = 44,
  SystemLookasideInformation = 45,
  SystemSetTimeSlipEvent = 46,
  SystemCreateSession = 47,
  SystemDeleteSession = 48,
  SystemInvalidInfoClass4 = 49,
  SystemRangeStartInformation = 50,
  SystemVerifierInformation = 51,
  SystemAddVerifier = 52,
  SystemSessionProcessesInformation = 53,
  SystemLogicalProcessorInformation = 73,
  SystemInformationClassMax
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
  LARGE_INTEGER IdleTime;
  LARGE_INTEGER KernelTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER DpcTime;
  LARGE_INTEGER InterruptTime;
  ULONG InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

typedef LONG NTSTATUS;

extern "C" DWORD __stdcall NtQuerySystemInformation(DWORD, PVOID, ULONG, PULONG);
