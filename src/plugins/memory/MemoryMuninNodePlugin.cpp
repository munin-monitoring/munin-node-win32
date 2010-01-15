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
#include "MemoryMuninNodePlugin.h"

MemoryMuninNodePlugin::MemoryMuninNodePlugin() 
{

}

MemoryMuninNodePlugin::~MemoryMuninNodePlugin()
{

}

int MemoryMuninNodePlugin::GetConfig(char *buffer, int len) 
{
  strncpy(buffer, 
    "graph_args --base 1024 -l 0 --vertical-label Bytes --upper-limit 329342976\n"
    "graph_title Memory usage\n"
    "graph_category system\n"
    "graph_info This graph shows what the machine uses its memory for.\n"
    "graph_order apps free swap\n"
    "apps.label apps\n"
    "apps.draw AREA\n"
    "apps.info Memory used by user-space applications.\n"
    "swap.label swap\n"
    "swap.draw STACK\n"
    "swap.info Swap space used.\n"
    "free.label unused\n"
    "free.draw STACK\n"
    "free.info Wasted memory. Memory that is not used for anything at all.\n"
    //"committed.label committed\n"
    //"committed.draw LINE2\n"
    //"committed.warn 625410048\n"
    //"committed.info The amount of memory that would be used if all the memory that's been allocated were to be used.\n"
    ".\n", len);
  return 0;
}

int MemoryMuninNodePlugin::GetValues(char *buffer, int len) 
{
  MEMORYSTATUSEX mem;
  mem.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&mem);
  _snprintf(buffer, len, "apps.value %llu\n"
    "swap.value %llu\n"
    "free.value %llu\n"
    //"committed.value %u\n"
    ".\n", mem.ullTotalPhys-mem.ullAvailPhys, mem.ullTotalPageFile-mem.ullAvailPageFile, mem.ullAvailPhys);
  return 0;
}
