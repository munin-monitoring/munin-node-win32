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
#include "HDMuninNodePlugin.h"

HDMuninNodePlugin::HDMuninNodePlugin() 
{
  
}

HDMuninNodePlugin::~HDMuninNodePlugin() 
{

}

int HDMuninNodePlugin::GetConfig(char *buffer, int len) 
{
  int ret = 0;
  int index = 0;

  ret = _snprintf(buffer, len, "graph_title HDD temperature\n"
    "graph_args --base 1000 -l 0\n"
    "graph_vlabel temp in C\n"
    "graph_category sensors\n"
    "graph_info This graph shows the temperature in degrees Celsius of the hard drives in the machine.\n");
  buffer += ret;
  len -= ret;

  JCAutoLockCritSec lock(&g_SmartReaderCritSec);
  g_SmartReader.UpdateSMART();
  for (index = 0; index < g_SmartReader.m_ucDrivesWithInfo; index++) {
    ret = _snprintf(buffer, len, "_hdd_%i_.label %s\n", index, 
      (PCHAR)g_SmartReader.m_stDrivesInfo[index].m_stInfo.sModelNumber);
    len -= ret;
    buffer += ret;
  }

  strncat(buffer, ".\n", len);

  return 0;
}

int HDMuninNodePlugin::GetValues(char *buffer, int len) 
{ 
  int index = 0;
  int ret;    

  JCAutoLockCritSec lock(&g_SmartReaderCritSec);
  g_SmartReader.UpdateSMART();
  for (index = 0; index < g_SmartReader.m_ucDrivesWithInfo; index++) {
    ST_DRIVE_INFO *pDriveInfo = g_SmartReader.GetDriveInfo(index);
    if (!pDriveInfo)
      continue;

    ST_SMART_INFO *pSmartInfo = g_SmartReader.GetSMARTValue(pDriveInfo->m_ucDriveIndex, SMART_ATTRIB_TEMPERATURE);
    if (!pSmartInfo)
      continue;

    ret = _snprintf(buffer, len, "_hdd_%i_.value %i\n", index, pSmartInfo->m_dwAttribValue);
    len -= ret;
    buffer += ret;
  }

  strncat(buffer, ".\n", len);
  return 0;
}
