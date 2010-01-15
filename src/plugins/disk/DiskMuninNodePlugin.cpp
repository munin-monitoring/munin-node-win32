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
#include "DiskMuninNodePlugin.h"

DiskMuninNodePlugin::DiskMuninNodePlugin() 
{
  int i;
  for (i = 0; i < 32; i++) {
    drives[i][0] = NULL;
  }
  i = 0;
  for (int d = 'A'; d <= 'Z'; d++) {
    drives[i][0] = d;
    drives[i][1] = ':';
    drives[i][2] = NULL;//'\\';
    drives[i][3] = NULL;
    if (GetDriveTypeA(drives[i]) != DRIVE_FIXED) {
      // Remove it
      drives[i][0] = NULL;
    } else {
      i++;
    }
  }
}

DiskMuninNodePlugin::~DiskMuninNodePlugin() 
{

}

int DiskMuninNodePlugin::GetConfig(char *buffer, int len) 
{
  int ret = 0;
  int index = 0;

  ret = _snprintf(buffer, len, "graph_title Filesystem usage (in %%)\n"
    "graph_category disk\n"
    "graph_info This graph shows disk usage on the machine.\n"
    "graph_args --upper-limit 100 -l 0\n"
    "graph_vlabel %%\n");
  buffer += ret;
  len -= ret;

  int warning = g_Config.GetValueI("DiskPlugin", "Warning", 92);
  int critical = g_Config.GetValueI("DiskPlugin", "Critical", 98);
  while (drives[index][0] != NULL) {
    ret = _snprintf(buffer, len, "_dev_%i_.label %s\n"
      "_dev_%i_.warning %i\n"
      "_dev_%i_.critical %i\n", index, drives[index], index, warning, index, critical);
    len -= ret;
    buffer += ret;
    index++;
  }

  strncat(buffer, ".\n", len);

  return 0;
}

int DiskMuninNodePlugin::GetValues(char *buffer, int len)
{ 
  ULARGE_INTEGER uliTotal = { 0 };
  ULARGE_INTEGER uliFree = { 0 };
  int index = 0;
  int ret;

  while (drives[index][0] != NULL) {
    ret = GetDiskFreeSpaceExA(drives[index], NULL, &uliTotal, &uliFree);
    ret = _snprintf(buffer, len, "_dev_%i_.value %i\n", index, 100-(int)(100.0f / uliTotal.QuadPart * uliFree.QuadPart));
    len -= ret;
    buffer += ret;
    index++;
  }
  strncat(buffer, ".\n", len);
  return 0;
}
