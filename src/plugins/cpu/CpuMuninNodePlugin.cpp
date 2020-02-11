/* This file is part of munin-node-win32
* Copyright (C) 2006-2008 Jory Stone (jcsston@jory.info)
* Copyright (C) 2007 Steve Schnepp <steve.schnepp@gmail.com> 
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "StdAfx.h"
#include "CpuMuninNodePlugin.h"
#include <Windows.h>
#include <stdio.h>
#include <atlstr.h>
#include <iostream>

#define SystemBasicInformation 0
#define SystemTimeInformation 3

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

// Initialisation
CpuMuninNodePlugin::CpuMuninNodePlugin()
{ 
  dbCpuPercent = 0;
  dbCpuHeadroomPercent = 0;
  liOldSystemTime = 0;
  liOldIdleTime = 0;
  NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle(_T("ntdll")), "NtQuerySystemInformation");
  GetSystemTimes = (pfnGetSystemTimes)GetProcAddress(GetModuleHandle(_T("kernel32")), "GetSystemTimes");

  // Setup first call
  CalculateCpuLoad();
}

CpuMuninNodePlugin::~CpuMuninNodePlugin()
{

}

void CpuMuninNodePlugin::CalculateCpuLoad()
{
    CString  ProcessorHeadroomPercentage;

    FILETIME IdleTime, KernelTime, UserTime;
    static unsigned long long PrevTotal = 0;
    static unsigned long long PrevIdle = 0;
    static unsigned long long PrevUser = 0;
    unsigned long long ThisTotal;
    unsigned long long ThisIdle, ThisKernel, ThisUser;
    unsigned long long TotalSinceLast, IdleSinceLast, UserSinceLast;


    // GET THE KERNEL / USER / IDLE times.  
    // And oh, BTW, kernel time includes idle time
    GetSystemTimes(&IdleTime, &KernelTime, &UserTime);

    ThisIdle = FileTimeToInt64(IdleTime);
    ThisKernel = FileTimeToInt64(KernelTime);
    ThisUser = FileTimeToInt64(UserTime);

    ThisTotal = ThisKernel + ThisUser;
    TotalSinceLast = ThisTotal - PrevTotal;
    IdleSinceLast = ThisIdle - PrevIdle;
    UserSinceLast = ThisUser - PrevUser;
    double Headroom;
    Headroom = (double)IdleSinceLast / (double)TotalSinceLast;
    double Load;
    Load = 1.0 - Headroom;
    Headroom *= 100.0;  // to make it percent
    Load *= 100.0;  // percent

    PrevTotal = ThisTotal;
    PrevIdle = ThisIdle;
    PrevUser = ThisUser;

    dbCpuPercent = Load;
    dbCpuHeadroomPercent = Headroom;

  /*if (NtQuerySystemInformation != NULL && GetSystemTimes != NULL) {
    LONG status;
    SYSTEM_TIME_INFORMATION SysTimeInfo;
    SYSTEM_BASIC_INFORMATION SysBaseInfo;

    // get number of processors in the system
    status = NtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL);
    if (status != NO_ERROR) {
      printf("Querying SystemBasicInformation failed: 0x%x\n", status);
      return;
    }

    // get new system time
    status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), NULL);
    if (status!=NO_ERROR) {
      printf("Querying SystemTimeInformation failed: 0x%x\n", status);
      return;
    }

    // get new CPU times
    // http://www.codeproject.com/Articles/9113/Get-CPU-Usage-with-GetSystemTimes
    FILETIME ftIdleTime;
    FILETIME ftKernelTime;
    FILETIME ftUserTime;
    BOOL result = GetSystemTimes((LPFILETIME)&ftIdleTime, (LPFILETIME)&ftKernelTime, (LPFILETIME)&ftUserTime);
    if (result == FALSE) {
      printf("GetSystemTimes failed\n");
      return;
    }
    unsigned long long systemTime = FileTimeToInt64(ftKernelTime) + FileTimeToInt64(ftUserTime);
    unsigned long long idleTime = FileTimeToInt64(ftIdleTime);

    // if it's a first call - skip it
    if (liOldIdleTime != 0)
    {
      // CurrentValue = NewValue - OldValue
      __int64 diffIdleTime = idleTime - liOldIdleTime;
      __int64 diffSystemTime = systemTime - liOldSystemTime;

      dbCpuTimePercent = (1.0f - ((diffSystemTime > 0) ? ((float)diffIdleTime) / diffSystemTime : 0)) * 100;
    }

    // store new times
    liOldIdleTime = idleTime;
    liOldSystemTime = systemTime;
  }
  else {
    printf("NtQuerySystemInformation or GetSystemTimes functions not available\n");
  }*/
}

unsigned long long CpuMuninNodePlugin::FileTimeToInt64(const FILETIME& ft) {
    return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}

int CpuMuninNodePlugin::GetValues(char *buffer, int len) 
{
  CalculateCpuLoad();

  _snprintf(buffer, len, 
    "cpu_load.value %f\n"
    "cpu_headroom.value %f\n"
    ".\n", this->dbCpuPercent, this->dbCpuHeadroomPercent);
  return 0;
}

int CpuMuninNodePlugin::GetConfig(char *buffer, int len) 
{
  strncpy(buffer, 
    "graph_args -l 0 --vertical-label percent --upper-limit 100\n"
    "graph_title Cpu usage\n"
    "graph_category system\n"
    "graph_info This graph shows what the machine uses its cpu for.\n"
    "graph_order cpu_user\n"
    "graph_order cpu_total cpu_system cpu_user\n"
    "cpu_load.label CPU Load\n"
    "cpu_load.draw LINE2\n"
    "cpu_load.info CPU Usage.\n"
    "cpu_headroom.label CPU Headroom\n"
    "cpu_headroom.draw LINE2\n"
    "cpu_headroom.info CPU Headroom.\n"
    ".\n", len);
  return 0;
}
