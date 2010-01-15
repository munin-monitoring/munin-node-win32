/* This file is part of munin-node-win32
 * Copyright (C) 2006-2007 Jory Stone (jcsston@jory.info)
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
#include "UptimeMuninNodePlugin.h"

UptimeMuninNodePlugin::UptimeMuninNodePlugin()
{
  PDH_STATUS status;
  //char uptimeCounterPath[] = "\\Sistema\\Tiempo de actividad del sistema";
  TString uptimeCounterPath = A2TConvert(g_Config.GetValue("UptimePlugin", "CounterPath", "\\System\\System Up Time"));
  OSVERSIONINFO osvi;    
  
  m_PerfQuery = NULL;
  m_UptimeCounter = NULL;

  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (!GetVersionEx(&osvi) || (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT))
    return; //unknown OS or not NT based

  // Create a PDH query
  if (PdhOpenQuery(NULL, 0, &m_PerfQuery) != ERROR_SUCCESS)
    return;

  // Associate the uptime counter with the query
  status = PdhAddCounter(m_PerfQuery, uptimeCounterPath.c_str(), 0, &m_UptimeCounter);
  if (status != ERROR_SUCCESS)
    return;
}

UptimeMuninNodePlugin::~UptimeMuninNodePlugin()
{
  // Close the counter
  PdhRemoveCounter(m_UptimeCounter);
  // Close the query
  PdhCloseQuery(&m_PerfQuery);
}

int UptimeMuninNodePlugin::GetConfig(char *buffer, int len) 
{
  strncpy(buffer, "graph_title Uptime\n"
    "graph_category system\n"
    "graph_args --base 1000 -l 0\n"
    "graph_vlabel uptime in days\n"
    "uptime.label uptime\n"
    "uptime.draw AREA\n"
    ".\n", len);

  return 0;
}

int UptimeMuninNodePlugin::GetValues(char *buffer, int len)
{ 
  PDH_STATUS status;
  PDH_FMT_COUNTERVALUE uptimeValue;

  // Collect the uptime value
  status = PdhCollectQueryData(m_PerfQuery);
  if (status != ERROR_SUCCESS)
  {
    _snprintf(buffer, len, ".\n");
     return -1;
  }

  // Get the formatted counter value
  status = PdhGetFormattedCounterValue(m_UptimeCounter, PDH_FMT_LARGE, NULL, &uptimeValue);
  if (status != ERROR_SUCCESS)
  {
    _snprintf(buffer, len, ".\n");
     return -1;
  }

  _snprintf(buffer, len, "uptime.value %.2f\n.\n", (uptimeValue.largeValue / 86400.0f));

  return 0;
}
