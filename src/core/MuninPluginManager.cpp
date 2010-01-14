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
#include "MuninPluginManager.h"
#include "Service.h"

#include "../plugins/cpu/CpuMuninNodePlugin.h"
#include "../plugins/disk/DiskMuninNodePlugin.h"
#include "../plugins/memory/MemoryMuninNodePlugin.h"
#include "../plugins/network/NetworkMuninNodePlugin.h"
#include "../plugins/process/ProcessesMuninNodePlugin.h"
#include "../plugins/mbm/MBMMuninNodePlugin.h"
#include "../plugins/disk/HDMuninNodePlugin.h"
#include "../plugins/disk/SMARTMuninNodePlugin.h"
#include "../plugins/speedfan/SpeedFanNodePlugin.h"
#include "../plugins/PerfCounterMuninNodePlugin.h"
#include "../plugins/external/ExternalMuninNodePlugin.h"

#ifdef _DEBUG
class MuninPluginManagerTestThread : public JCThread {
  MuninPluginManager *m_Manager;
public:
  MuninPluginManagerTestThread(MuninPluginManager *manager) : m_Manager(manager) {};

  virtual void *Entry() {
    while (!TestDestroy()) {
      m_Manager->TestPlugins();
      for (int i = 0; i < 50 && !TestDestroy(); i++)
        Sleep(100);
    }
    return NULL;
  };
};
MuninPluginManagerTestThread *t = NULL;
#endif

MuninPluginManager::MuninPluginManager()
{
  if (g_Config.GetValueB("Plugins", "Disk", true))
    AddPlugin(new DiskMuninNodePlugin());
  if (g_Config.GetValueB("Plugins", "Memory", true))
    AddPlugin(new MemoryMuninNodePlugin());
 if (g_Config.GetValueB("Plugins", "Processes", true))
    AddPlugin(new ProcessesMuninNodePlugin());
  if (g_Config.GetValueB("Plugins", "Network", true))
    AddPlugin(new NetworkMuninNodePlugin());
  
  if (g_Config.GetValueB("Plugins", "MbmTemp", true))
    AddPlugin(new MBMMuninNodePlugin(mbm::stTemperature));
  if (g_Config.GetValueB("Plugins", "MbmVoltage", true))
    AddPlugin(new MBMMuninNodePlugin(mbm::stVoltage));
  if (g_Config.GetValueB("Plugins", "MbmFan", true))
    AddPlugin(new MBMMuninNodePlugin(mbm::stFan));
  if (g_Config.GetValueB("Plugins", "MbmMhz", true))
    AddPlugin(new MBMMuninNodePlugin(mbm::stMhz));
  
  if (g_Config.GetValueB("Plugins", "Cpu", true))
    AddPlugin(new CpuMuninNodePlugin());
  if (g_Config.GetValueB("Plugins", "HD", true))
    AddPlugin(new HDMuninNodePlugin());

  if (g_Config.GetValueB("Plugins", "SMART", false))
    AddPlugin(new SMARTMuninNodePlugin());
  
  if (g_Config.GetValueB("Plugins", "SpeedFan", false))
    AddPlugin(new SpeedFanNodePlugin());
  
  if (g_Config.GetValueB("Plugins", "External", true)) {
    int externalCount = g_Config.NumValues("ExternalPlugin");
    for (int i = 0; i < externalCount; i++) {
      std::string valueName = g_Config.GetValueName("ExternalPlugin", i); 
      std::string filename = g_Config.GetValue("ExternalPlugin", valueName);
      ExternalMuninNodePlugin *plugin = new ExternalMuninNodePlugin(filename);
      if (plugin->IsLoaded()) {
        AddPlugin(plugin);
      } else {
        _Module.LogEvent("Failed to load External plugin: %s", filename.c_str());
        delete plugin;
      }
    }
  }

  const char *perfPrefix = PerfCounterMuninNodePlugin::SectionPrefix;
  size_t perfPrefixLen = strlen(perfPrefix);
  for (size_t i = 0; i < g_Config.GetNumKeys(); i++) {
    std::string keyName = g_Config.GetKeyName(i);
    if (keyName.compare(0, perfPrefixLen, perfPrefix) == 0) {
      PerfCounterMuninNodePlugin *plugin = new PerfCounterMuninNodePlugin(keyName);
      if (plugin->IsLoaded()) {
        AddPlugin(plugin);
      } else {
        _Module.LogEvent("Failed to load PerfCounter plugin: [%s]", keyName.c_str());
        delete plugin;
      }
    }
  }
  
#ifdef _DEBUG
  t = new MuninPluginManagerTestThread(this);
  t->JCThread_AddRef();
  t->Run();  
#endif
}

MuninPluginManager::~MuninPluginManager()
{
#ifdef _DEBUG
  t->Stop();
  Sleep(150);
  t->JCThread_RemoveRef();
#endif
  for (size_t i = 0; i < m_Plugins.size(); i++) {
    delete m_Plugins[i];
  }
}

void MuninPluginManager::AddPlugin(MuninNodePlugin *plugin)
{
  _Module.LogEvent("Loaded plugin [%s]", typeid(*plugin).name());
  if (!plugin->IsThreadSafe())
    plugin = new MuninNodePluginLockWrapper(plugin);
  m_Plugins.push_back(plugin);
}

MuninNodePlugin *MuninPluginManager::LookupPlugin(const char *name) 
{
  MuninNodePlugin *plugin = NULL;
  name = strstr(name, " ");
  if (name != NULL) {
    name++;
  } else {
    return NULL;
  }
  for (size_t i = 0; i < m_Plugins.size(); i++) {
    if (!strcmp(name, m_Plugins[i]->GetName())) {
      return m_Plugins[i];
    }
  }
  return NULL;
}

void MuninPluginManager::FillPluginList(char *buffer, int len) 
{
  buffer[0] = NULL;
  for (size_t i = 0; i < m_Plugins.size(); i++) {
    if (m_Plugins[i]->IsLoaded()) {
      strncat(buffer, m_Plugins[i]->GetName(), len);
      strncat(buffer, " ", len);
    }
  }
  strncat(buffer, "\n", len);
}

void MuninPluginManager::TestPlugins()
{
  // Test Plugins
  char buffer[8096];
  for (size_t i = 0; i < m_Plugins.size(); i++) {
    _Module.LogEvent("Name: %s", m_Plugins[i]->GetName());
    
    buffer[0] = NULL;
    m_Plugins[i]->GetConfig(buffer,  sizeof(buffer));
    _Module.LogEvent("Config:\n%s", buffer);
    
    buffer[0] = NULL;
    m_Plugins[i]->GetValues(buffer, sizeof(buffer));
    _Module.LogEvent("Value:\n%s", buffer);
  }
}