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
#include "MBMMuninNodePlugin.h"

MBMMuninNodePlugin::MBMMuninNodePlugin(mbm::sensor_t mode)
  : m_Mode(mode)
{
  switch (m_Mode) {
    case mbm::stTemperature:
      strcpy(m_Name, "mbm");
      break;
    case mbm::stVoltage:
      strcpy(m_Name, "mbm_volt");
      break;
    case mbm::stFan:
      strcpy(m_Name, "mbm_fan");
      break;
    case mbm::stMhz:
      strcpy(m_Name, "mbm_cpu");
      break;
    default:
      assert(false);
  }  
}

MBMMuninNodePlugin::~MBMMuninNodePlugin()
{
}

int MBMMuninNodePlugin::GetConfig(char *buffer, int len)
{
  const char *header = "";  
  switch (m_Mode) {
    case mbm::stTemperature:
      header = 
        "graph_title System temperature\n"
        "graph_args --base 1000 -l 0\n"
        "graph_vlabel temp in C\n"
        "graph_category sensors\n"
        "graph_info This graph shows the temperature in degrees Celsius.\n";
      break;
    case mbm::stVoltage:
      header = 
        "graph_title System voltages\n"
        "graph_args --base 1000 -l 0\n"
        "graph_vlabel voltage\n"
        "graph_category sensors\n"
        "graph_info This graph shows the voltages supplied by the PSU.\n";
      break;
    case mbm::stFan:
      header = 
        "graph_title Fan\n"
        "graph_args --base 1000 -l 0\n"
        "graph_vlabel fan speed in RPM\n"
        "graph_category sensors\n"
        "graph_info This graph shows the fan speeds.\n";
      break;
    case mbm::stMhz:
      header = 
        "graph_title CPU Speed\n"
        "graph_args --base 1000 -l 0\n"
        "graph_vlabel CPU speed in Mhz\n"
        "graph_category sensors\n"
        "graph_info This graph shows the cpu speed in Mhz.\n";
      break;
    default:
      assert(false);
  }
  int header_LEN = (int)strlen(header);    
    strncpy(buffer, header, len);
  buffer = buffer + header_LEN;
  len -= header_LEN;  
  if (m_MBM.open()) {
    mbm::sensor *sensors = m_MBM.sensor();
    if (sensors != NULL) {      
      for (int i = 0; i < SENSOR_INFO_LENGTH; i++) {
        mbm::sensor &curSensor = sensors[i];
        LPCSTR szName = curSensor.name();
        double val = curSensor.current();        
        int count;
        if (curSensor.type() == m_Mode && val != 255 && val != 0) {
          if (szName != NULL) {
            count = _snprintf(buffer, len, "sensor%i.label %s\n", i, szName);
          } else {
            count = _snprintf(buffer, len, "sensor%i.label Sensor %i\n", i, i);
          }
          buffer = buffer + count;
          len -= count;
        }
      }
    }
    strncat(buffer, ".\n", len);
  } else {
    // MBM isn't running?
    strncpy(buffer, ".\n", len);
  }
  return 0;
};

int MBMMuninNodePlugin::GetValues(char *buffer, int len) 
{ 
  if (m_MBM.open()) {
    mbm::sensor *sensors = m_MBM.sensor();
    if (sensors != NULL) {      
      for (int i = 0; i < SENSOR_INFO_LENGTH; i++) {
        mbm::sensor &curSensor = sensors[i];
        double val = curSensor.current();
        int count;
        if (curSensor.type() == m_Mode && val != 255 && val != 0) {
          count = _snprintf(buffer, len, "sensor%i.value %.2f\n", i, val);
          buffer = buffer + count;
          len -= count;
        }
      }
    }
  }
  strncat(buffer, ".\n", len);
  return 0;
};

bool MBMMuninNodePlugin::IsLoaded()
{ 
  char buffer[512]; 
  GetConfig(buffer, 512); 
  return m_MBM.opened(); 
};
