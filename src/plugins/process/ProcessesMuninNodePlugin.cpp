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
#include "ProcessesMuninNodePlugin.h"

ProcessesMuninNodePlugin::ProcessesMuninNodePlugin()
{
}

ProcessesMuninNodePlugin::~ProcessesMuninNodePlugin()
{
}

int ProcessesMuninNodePlugin::GetConfig(char *buffer, int len) 
{
  strncpy(buffer, "graph_title Number of Processes\n"
    "graph_args --base 1000 -l 0\n"
    "graph_vlabel number of processes\n"
    "graph_category processes\n"
    "graph_info This graph shows the number of processes and threads in the system.\n"
    "processes.label processes\n"
    "processes.draw LINE2\n"
    "processes.info The current number of processes.\n"
    "threads.label threads\n"
    "threads.draw LINE1\n"
    "threads.info The current number of threads.\n"
    ".\n", len);

  return 0;
}

int ProcessesMuninNodePlugin::GetValues(char *buffer, int len) 
{ 
  int processes = 0;
  int threads = 0;

  HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapShot != INVALID_HANDLE_VALUE) {
    PROCESSENTRY32 entry;
    memset(&entry, 0, sizeof(PROCESSENTRY32));
    entry.dwSize = sizeof(PROCESSENTRY32);
    Process32First(hSnapShot, &entry);
    do {
      processes++;
      threads += entry.cntThreads;
    } while (Process32Next(hSnapShot, &entry));

    CloseHandle(hSnapShot);
  }

  _snprintf(buffer, len, "processes.value %i\nthreads.value %i\n.\n", processes, threads);
  return 0;
}
