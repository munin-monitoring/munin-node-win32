/* This file is part of munin-node-win32
 * Copyright (C) 2006-2008 Jory Stone (jcsston@jory.info)
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
#include "DiskTimeMuninNodePlugin.h"

DiskTimeMuninNodePlugin::DiskTimeMuninNodePlugin()
{
  m_PerfQuery = NULL;
  m_Loaded = OpenCounter();
}

DiskTimeMuninNodePlugin::~DiskTimeMuninNodePlugin()
{
  for (size_t i = 0; i < m_DiskTimeCounters.size(); i++) {
    // Close the counters
    PdhRemoveCounter(m_DiskTimeCounters[i]);
  }

  if (m_PerfQuery != NULL) {
    // Close the query
    PdhCloseQuery(&m_PerfQuery);
  }
}

bool DiskTimeMuninNodePlugin::OpenCounter()
{
  PDH_STATUS status;  

  OSVERSIONINFO osvi;    
  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (!GetVersionEx(&osvi) || (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT))
    return false; //unknown OS or not NT based

  // Create a PDH query
  status = PdhOpenQuery(NULL, 0, &m_PerfQuery);
  if (status != ERROR_SUCCESS)
    return false;

  TString logicalDiskCounterName = A2TConvert(g_Config.GetValue("DiskTimePlugin", "CounterNameLogicalDisk", "LogicalDisk"));
  TString diskTimeCounterName = A2TConvert(g_Config.GetValue("DiskTimePlugin", "CounterNameDiskTime", "% Disk Time"));
  
  DWORD counterListLength = 0;  
  DWORD instanceListLength = 0;
  status = PdhEnumObjectItems(NULL, NULL, logicalDiskCounterName.c_str(), NULL, &counterListLength, NULL, &instanceListLength, PERF_DETAIL_EXPERT, 0);
  if (status != PDH_MORE_DATA)
    return false;

  TCHAR *counterList = new TCHAR[counterListLength];
  TCHAR *instanceList = new TCHAR[instanceListLength];
  counterList[0] = NULL;
  instanceList[0] = NULL;

  status = PdhEnumObjectItems(NULL, NULL, logicalDiskCounterName.c_str(), counterList, &counterListLength, instanceList, &instanceListLength, PERF_DETAIL_EXPERT, 0);
  if (status != ERROR_SUCCESS) {
    delete [] counterList;
    delete [] instanceList;
    return false;  
  }

  int pos = 0;
  TCHAR *instanceName = instanceList;
  while (instanceName[0] != NULL && instanceName[1] != NULL) {
    std::string diskName = T2AConvert(instanceName);
    m_DiskTimeNames.push_back(diskName);
    while (instanceName[0] != NULL)
      instanceName++;
    instanceName++;
  }
  delete [] counterList;
  delete [] instanceList;

  // We drop the last instance name as it is _Total
  m_DiskTimeNames.pop_back();

  TCHAR diskTimeCounterPath[MAX_PATH] = {0};
  HCOUNTER diskTimeCounter;
  for (size_t i = 0; i < m_DiskTimeNames.size(); i++) {
    _sntprintf(diskTimeCounterPath, MAX_PATH, _T("\\%s(%s)\\%s"), logicalDiskCounterName.c_str(), m_DiskTimeNames[i].c_str(), diskTimeCounterName.c_str());
    // Associate the uptime counter with the query
    status = PdhAddCounter(m_PerfQuery, diskTimeCounterPath, 0, &diskTimeCounter);
    if (status != ERROR_SUCCESS)
      return false;
    
    m_DiskTimeCounters.push_back(diskTimeCounter);
  }
  
  // Collect init data
  status = PdhCollectQueryData(m_PerfQuery);
  if (status != ERROR_SUCCESS)
    return false;

  return true;
}

int DiskTimeMuninNodePlugin::GetConfig(char *buffer, int len) 
{  
  if (!m_DiskTimeCounters.empty()) {
    PDH_STATUS status;  
    DWORD infoSize = 0;
    status = PdhGetCounterInfo(m_DiskTimeCounters[0], TRUE, &infoSize, NULL);
    if (status != PDH_MORE_DATA)
      return -1;

    PDH_COUNTER_INFO *info = (PDH_COUNTER_INFO *)malloc(infoSize);
    status = PdhGetCounterInfo(m_DiskTimeCounters[0], TRUE, &infoSize, info);
    if (status != ERROR_SUCCESS)
      return -1;

    int printCount;
    printCount = _snprintf(buffer, len, "graph_title Disk Time\n"
      "graph_category system\n"
      "graph_args --base 1000 -l 0\n"
      "graph_info %s\n"
      "graph_vlabel %s\n", info->szExplainText, info->szCounterName);
    len -= printCount;
    buffer += printCount;

    free(info);

    assert(m_DiskTimeNames.size() == m_DiskTimeCounters.size());
    for (size_t i = 0; i < m_DiskTimeNames.size(); i++) {
      printCount = _snprintf(buffer, len, "disktime_%i_.label %s\n", i, m_DiskTimeNames[i].c_str());
      len -= printCount;
      buffer += printCount;
    }
  }

  strncat(buffer, ".\n", len);
  return 0;
}

int DiskTimeMuninNodePlugin::GetValues(char *buffer, int len)
{
  PDH_STATUS status;
  PDH_FMT_COUNTERVALUE diskTimeValue;
  int printCount;

  status = PdhCollectQueryData(m_PerfQuery);
  if (status != ERROR_SUCCESS)
    return -1;
  
  for (size_t i = 0; i < m_DiskTimeCounters.size(); i++) {
    // Get the formatted counter value    
    status = PdhGetFormattedCounterValue(m_DiskTimeCounters[i], PDH_FMT_DOUBLE, NULL, &diskTimeValue);
    if (status != ERROR_SUCCESS)
      return -1;
    printCount = _snprintf(buffer, len, "disktime_%i_.value %.2f\n", i, diskTimeValue.doubleValue);
    len -= printCount;
    buffer += printCount;
  }
  strncat(buffer, ".\n", len);
  return 0;
}
