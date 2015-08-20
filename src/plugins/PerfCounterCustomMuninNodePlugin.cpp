/* This file is part of munin-node-win32
* Copyright (C) 2006-2011 Jory Stone (jcsston@jory.info)
* Support for DERIVE Permon counters by jossh.robb@gmail.com
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
#include "PerfCounterCustomMuninNodePlugin.h"

#include "core/Service.h"
#include "core/Tools.h"

static const std::string EOL("\r\n");

const char* PerfCounterCustomMuninNodePlugin::SectionPrefix = "PerfCounterCustomPlugin_";

PerfCounterCustomMuninNodePlugin::PerfCounterCustomMuninNodePlugin(const std::string &sectionName)
: m_SectionName(sectionName)
{
  m_PerfQuery = NULL;
  m_Loaded = OpenCounter();
}

PerfCounterCustomMuninNodePlugin::~PerfCounterCustomMuninNodePlugin()
{
  // Close counters
  for (auto i = m_Fields.begin(); i != m_Fields.end(); i++) {
    const  Field& field = i->second;
    PdhRemoveCounter(field.counter.handle);
  }

  // Close query
  if (m_PerfQuery != NULL) {
    PdhCloseQuery(&m_PerfQuery);
  }
}

bool PerfCounterCustomMuninNodePlugin::OpenCounter()
{
  PDH_STATUS status;

  m_Name = m_SectionName.substr(strlen(PerfCounterCustomMuninNodePlugin::SectionPrefix));

  OSVERSIONINFO osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (!GetVersionEx(&osvi) || (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT)) {
    _Module.LogError("PerfCounterCustom plugin: %s: unknown OS or not NT based", m_Name.c_str());
    return false; //unknown OS or not NT based
  }

  // Create a PDH query
  status = PdhOpenQuery(NULL, 0, &m_PerfQuery);
  if (status != ERROR_SUCCESS) {
    _Module.LogError("PerfCounterCustom plugin: %s: PdhOpenQuery error=%x", m_Name.c_str(), status);
    return false;
  }

  // Iterate on each Key contained
  for (size_t i = 0; i < g_Config.GetNumValues(m_SectionName); i++) {
    auto valueName = g_Config.GetValueName(m_SectionName, i);

    // Decompose Key
    auto first_sep = valueName.find_first_of(".");
    auto keyArg = valueName.substr(first_sep + 1);
    auto keyType = valueName.substr(0, first_sep);
    auto keyValue = g_Config.GetValue(m_SectionName, valueName);

    _Module.LogEvent("PerfCounterCustom plugin: %s: read ValueName: [%s] => [%s]", m_Name.c_str(), valueName.c_str(), keyValue.c_str());

    if (keyType == "Field") {
      // Adding Args to Field def
      auto fieldName = keyArg.substr(0, keyArg.find_last_of("."));

      auto currentFieldIterator = m_Fields.find(fieldName);
      if (currentFieldIterator == m_Fields.end()) {
        // Adding it...
        Field newField;
        newField.name = fieldName;
        newField.counter.format = PDH_FMT_DOUBLE;
        newField.counter.multiply = 1.0;
        m_Fields[fieldName] = newField;

        // ... and now we should find it
        currentFieldIterator = m_Fields.find(fieldName);
      }
      Field& currentField = currentFieldIterator->second;

      // Set the Path if 
      if (keyArg.substr(fieldName.length()) == ".CounterPath") {
        // If path set already ignore the key
        if (currentField.counter.path != "") continue;

        // Sets the permon counter path.
        currentField.counter.path = keyValue;

        // .. and add it to our Perf Query
        status = PdhAddCounter(m_PerfQuery, A2TConvert(keyValue).c_str(), 0, &currentField.counter.handle);
        if (status != ERROR_SUCCESS) {
          _Module.LogError("PerfCounterCustom plugin: %s: PDH add counter error=%x (%s)", m_Name.c_str(), status, Tools::getHumanReadableError(status));
          return false;
        }
      }
      else if (keyArg.substr(fieldName.length()) == ".CounterFormat") {
        // Get the Conter format type
        std::string counterFormatStr = keyValue;
        if (!counterFormatStr.compare("double") || !counterFormatStr.compare("float")) {
          currentField.counter.format = PDH_FMT_DOUBLE;
        }
        else if (!counterFormatStr.compare("int") || !counterFormatStr.compare("long")) {
          currentField.counter.format = PDH_FMT_LONG;
        }
        else if (!counterFormatStr.compare("int64") || !counterFormatStr.compare("longlong") || !counterFormatStr.compare("large")) {
          currentField.counter.format = PDH_FMT_LARGE;
        }
        else {
          _Module.LogError("PerfCounter plugin: %s: Unknown CounterFormat: %s", m_Name.c_str(), counterFormatStr.c_str());
          assert(!"Unknown CounterFormat!");
        }
      }
      else if (keyArg.substr(fieldName.length()) == ".type" && keyValue == "DERIVE") {
        // DERIVE asked, override to use an integer format
        currentField.counter.format = PDH_FMT_LARGE;
        // But still add it to munin args
        currentField.preParsedArgs += keyArg + " " + keyValue + EOL;
      }
      else {
        // Normal argument, pass it directly to munin
        currentField.preParsedArgs += keyArg + " " + keyValue + EOL;
      }
    }
    else if (keyType == "Graph") {
      // Adding straight to graph definition
      m_GraphParameters += keyArg + " " + keyValue + EOL;
    }
    else {
      _Module.LogEvent("In plugin [%s], found unknown fieldType: [%s]", valueName.c_str(), keyType.c_str());
    }
  }

  // Start collecting data
  status = PdhCollectQueryData(m_PerfQuery);
  if (status != ERROR_SUCCESS) {
    if (status == PDH_INVALID_HANDLE) {
      _Module.LogError("PerfCounter plugin: %s: PDH collect data: PDH_INVALID_HANDLE", m_Name.c_str());
      return false;
    }
    if (status == PDH_NO_DATA) {
      _Module.LogError("PerfCounter plugin: %s: PDH collect data: PDH_NO_DATA", m_Name.c_str());
      return false;
    }

    _Module.LogError("PerfCounter plugin: %s: PDH collect data error: %x", m_Name.c_str(), status);
    return false;
  }

  return true;
}

int PerfCounterCustomMuninNodePlugin::GetConfig(char *buffer, int len)
{
  if (m_Fields.empty()) {
    // No fields defined.
    return -1;
  }

  std::string strOutConfig = m_GraphParameters;

  // Adding fields config
  for (auto i = m_Fields.begin(); i != m_Fields.end(); i++) {
    const  Field& field = i->second;
    strOutConfig += field.preParsedArgs;
  }

  // Converting back to char[] ---> buffer.
  _snprintf(buffer, len, "%s.\n", strOutConfig.c_str());
  return 0;
}

int PerfCounterCustomMuninNodePlugin::GetValues(char *buffer, int len)
{
  PDH_STATUS status;

  status = PdhCollectQueryData(m_PerfQuery);
  if (status != ERROR_SUCCESS) return -1;

  std::string outValues;
  for (auto i = m_Fields.begin(); i != m_Fields.end(); i++) {
    const  Field& field = i->second;
    outValues += field.name + ".value ";

    PDH_FMT_COUNTERVALUE counterValue;
    status = PdhGetFormattedCounterValue(field.counter.handle, field.counter.format, NULL, &counterValue);
    if (status != ERROR_SUCCESS) return -1;

    // Emit the counterValue with the correct formatting, try to be as accurate as possible
    const int valuesBufLen = 256;
    char valuesAsStr[valuesBufLen] = { 0 };
    switch (field.counter.format) {
    case PDH_FMT_DOUBLE:
      _snprintf(valuesAsStr, valuesBufLen, "%0.16f", counterValue.doubleValue * field.counter.multiply);
      break;
    case PDH_FMT_LONG:
      _snprintf(valuesAsStr, valuesBufLen, "%.0f", counterValue.longValue * field.counter.multiply);
      break;
    case PDH_FMT_LARGE:
      _snprintf(valuesAsStr, valuesBufLen, "%.0f", counterValue.largeValue * field.counter.multiply);
      break;
    default:
      _Module.LogError("PerfCounterCustom plugin: %s: PDH get counter unknown type=%x", m_Name.c_str(), field.counter.format);
      _snprintf(valuesAsStr, valuesBufLen, "U");
      break;
    }
    outValues += valuesAsStr;
    outValues += EOL;
  }

  // Converting back to char[] ---> buffer.
  _snprintf(buffer, len, "%s.\n", outValues.c_str());
  return 0;
}
