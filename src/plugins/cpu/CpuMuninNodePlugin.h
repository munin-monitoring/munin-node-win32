#pragma once
#include "../../core/MuninNodePlugin.h"

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
typedef BOOL (WINAPI *pfnGetSystemTimes)(LPFILETIME lpIdleTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime );

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
  unsigned long long FileTimeToInt64(const FILETIME & ft);


  PROCNTQSI NtQuerySystemInformation;
  pfnGetSystemTimes GetSystemTimes;
  double dbCpuTimePercent;
  unsigned long long liOldIdleTime;
  unsigned long long liOldSystemTime;
};
