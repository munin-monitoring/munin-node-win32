#pragma once
#include "../../core/MuninNodePlugin.h"

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

// ntdll!NtQuerySystemInformation (NT specific!)
//
// The function copies the system information of the
// specified type into a buffer
//
// NTSYSAPI
// NTSTATUS
// NTAPI
// NtQuerySystemInformation(
// IN UINT SystemInformationClass, // information type
// OUT PVOID SystemInformation, // pointer to buffer
// IN ULONG SystemInformationLength, // buffer size in bytes
// OUT PULONG ReturnLength OPTIONAL // pointer to a 32-bit
// // variable that receives
// // the number of bytes
// // written to the buffer
// );
typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);

class CpuMuninNodePlugin : public MuninNodePlugin
{
public:
  CpuMuninNodePlugin();
  virtual ~CpuMuninNodePlugin();

  virtual const char *GetName() { return "cpu"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return NtQuerySystemInformation != NULL; }

private:
  void CalculateCpuLoad();

  PROCNTQSI NtQuerySystemInformation;
  double dbIdleTime;
  double dbSystemTime;  
  LARGE_INTEGER liOldIdleTime;
  LARGE_INTEGER liOldSystemTime;
};
