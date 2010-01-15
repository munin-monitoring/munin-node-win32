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
#include "MuninNodePlugin.h"

bool MuninNodePlugin::IsThreadSafe() 
{
  return false; 
}

MuninNodePluginLockWrapper::MuninNodePluginLockWrapper(MuninNodePlugin *plugin) 
  : m_Plugin(plugin) 
{
  assert(m_Plugin != NULL); 
}

MuninNodePluginLockWrapper::~MuninNodePluginLockWrapper()
{
  delete m_Plugin;
}

const char *MuninNodePluginLockWrapper::GetName() 
{
  return m_Plugin->GetName(); 
}

int MuninNodePluginLockWrapper::GetConfig(char *buffer, int len) 
{
  JCAutoLockCritSec lock(&m_PluginCritSec);
  return m_Plugin->GetConfig(buffer, len); 
}

int MuninNodePluginLockWrapper::GetValues(char *buffer, int len) 
{
  JCAutoLockCritSec lock(&m_PluginCritSec);
  return m_Plugin->GetValues(buffer, len); 
}

bool MuninNodePluginLockWrapper::IsLoaded() 
{
  return m_Plugin->IsLoaded(); 
}

bool MuninNodePluginLockWrapper::IsThreadSafe() 
{
  return true; 
}
