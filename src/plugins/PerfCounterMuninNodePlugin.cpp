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
#include "PerfCounterMuninNodePlugin.h"

#include "../core/Service.h"

const char *PerfCounterMuninNodePlugin::SectionPrefix = "PerfCounterPlugin_";

PerfCounterMuninNodePlugin::PerfCounterMuninNodePlugin(const std::string &sectionName)
  : m_SectionName(sectionName)
{
  m_PerfQuery = NULL;
  m_Loaded = OpenCounter();
}

PerfCounterMuninNodePlugin::~PerfCounterMuninNodePlugin()
{
  for (size_t i = 0; i < m_Counters.size(); i++) {
    // Close the counters
    PdhRemoveCounter(m_Counters[i]);
  }

  if (m_PerfQuery != NULL) {
    // Close the query
    PdhCloseQuery(&m_PerfQuery);
  }
}

const TCHAR *PerfCounterMuninNodePlugin::GetPdhCounterLocalizedName(const TCHAR *englishName)
{
	TCHAR *regBuffer;
	DWORD regBufferSize = 4096;
	DWORD status;

	do {
		regBuffer = new TCHAR[regBufferSize];
		status = RegQueryValueEx(HKEY_PERFORMANCE_DATA, L"Counter 009", NULL, NULL, (LPBYTE)regBuffer, &regBufferSize);

		if (status == ERROR_MORE_DATA) {
			delete [] regBuffer;
			regBufferSize += 4096;
		} else {
			if (status != ERROR_SUCCESS) {
				delete [] regBuffer;
				_Module.LogEvent("PerfCounter plugin: %s: RegQueryValueEx error=%x", m_Name.c_str(), status);
				return englishName;
			}
		}
	} while (status != ERROR_SUCCESS);

	DWORD fIndex = -1;

	for (TCHAR *idx = regBuffer; *idx; idx += wcslen(idx) + 1) {
		TCHAR *cName = idx + wcslen(idx) + 1;
		if (_wcsicmp(cName, englishName) == 0) {
			fIndex = _wtol(idx);
			break;
		}
		idx = cName;
	}

	delete regBuffer;
	
	DWORD bufSize = 0;
	PdhLookupPerfNameByIndex(NULL, fIndex, NULL, &bufSize);
	TCHAR *localName = new TCHAR[bufSize];
	status = PdhLookupPerfNameByIndex(NULL, fIndex, localName, &bufSize);
	if (status != ERROR_SUCCESS) {
		_Module.LogError("PerfCounter plugin: %s: Could not find a local name for %ls, error=%x", m_Name.c_str(), englishName, status);
		return englishName;
	}
	return localName;
}


bool PerfCounterMuninNodePlugin::OpenCounter()
{
  PDH_STATUS status;  

  m_Name = m_SectionName.substr(strlen(PerfCounterMuninNodePlugin::SectionPrefix));

  OSVERSIONINFO osvi;    
  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (!GetVersionEx(&osvi) || (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT)) {
	  _Module.LogError("PerfCounter plugin: %s: unknown OS or not NT based", m_Name.c_str());
	  return false; //unknown OS or not NT based
  }

  // Create a PDH query
  status = PdhOpenQuery(NULL, 0, &m_PerfQuery);
  if (status != ERROR_SUCCESS) {
	  _Module.LogError("PerfCounter plugin: %s: PdhOpenQuery error=%x", m_Name.c_str(), status);
	  return false;
  }

  TString objectName = A2TConvert(g_Config.GetValue(m_SectionName, "Object", "LogicalDisk"));
  TString counterName = A2TConvert(g_Config.GetValue(m_SectionName, "Counter", "% Disk Time"));
  
  DWORD counterListLength = 0;  
  DWORD instanceListLength = 0;
  if (g_Config.GetValueB(m_SectionName, "UseEnglishObjectNames", true)) {
	  counterName = GetPdhCounterLocalizedName(counterName.c_str());
	  objectName = GetPdhCounterLocalizedName(objectName.c_str());
  }
  status = PdhEnumObjectItems(NULL, NULL, objectName.c_str(), NULL, &counterListLength, NULL, &instanceListLength, PERF_DETAIL_EXPERT, 0);
  if (status != PDH_MORE_DATA) {
	  _Module.LogError("PerfCounter plugin: %s: PdhEnumObjectItems error=%x", m_Name.c_str(), status);
	  return false;
  }

  TCHAR *counterList = new TCHAR[counterListLength+2];
  TCHAR *instanceList = new TCHAR[instanceListLength+2];
  counterList[0] = NULL;
  instanceList[0] = NULL;
  counterList[1] = NULL;
  instanceList[1] = NULL;

  status = PdhEnumObjectItems(NULL, NULL, objectName.c_str(), counterList, &counterListLength, instanceList, &instanceListLength, PERF_DETAIL_EXPERT, 0);
  if (status != ERROR_SUCCESS) {
    delete [] counterList;
    delete [] instanceList;
	_Module.LogError("PerfCounter plugin: %s: PdhEnumObjectItems error=%x", m_Name.c_str(), status);
    return false;  
  }

  int pos = 0;
  TCHAR *instanceName = instanceList;
  while (instanceName[0] != NULL) {
    std::string counterInstanceName = T2AConvert(instanceName);
    m_CounterNames.push_back(counterInstanceName);
    while (instanceName[0] != NULL)
      instanceName++;
    instanceName++;
  }
  delete [] counterList;
  delete [] instanceList;

  TCHAR counterPath[MAX_PATH] = {0};
  HCOUNTER counterHandle;
  if (!m_CounterNames.empty()) {
    if (g_Config.GetValueB(m_SectionName, "DropTotal", true)) {
      assert(m_CounterNames.back().compare("_Total") == 0);
      // We drop the last instance name as it is _Total
      m_CounterNames.pop_back();
    }

    for (size_t i = 0; i < m_CounterNames.size(); i++) {
      TString instanceNameStr = A2TConvert(m_CounterNames[i]);
      _sntprintf(counterPath, MAX_PATH, _T("\\%s(%s)\\%s"), objectName.c_str(), instanceNameStr.c_str(), counterName.c_str());
      // Associate the uptime counter with the query
      status = PdhAddCounter(m_PerfQuery, counterPath, 0, &counterHandle);
	  if (status != ERROR_SUCCESS) {
		  _Module.LogError("PerfCounter plugin: %s: PDH add counter error=%x", m_Name.c_str(), status);
		  return false;
	  }
      
      m_Counters.push_back(counterHandle);
    }
  } else {
    // A counter with a single instance (Uptime for example)
    m_CounterNames.push_back("0");
    _sntprintf(counterPath, MAX_PATH, _T("\\%s\\%s"), objectName.c_str(), counterName.c_str());
    // Associate the uptime counter with the query
    status = PdhAddCounter(m_PerfQuery, counterPath, 0, &counterHandle);
	if (status != ERROR_SUCCESS) {
		_Module.LogError("PerfCounter plugin: %s: PDH add counter error=%x", m_Name.c_str(), status);
		return false;
	}
    
    m_Counters.push_back(counterHandle);
  }
  
  // Collect init data
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
	  _Module.LogError("PerfCounter plugin: %s: PDH collect data error", m_Name.c_str());
	  return false;
  }

  // Setup Counter Format
  m_dwCounterFormat = PDH_FMT_DOUBLE;
  std::string counterFormatStr = g_Config.GetValue(m_SectionName, "CounterFormat", "double");
  if (!counterFormatStr.compare("double")
    || !counterFormatStr.compare("float")) {
    m_dwCounterFormat = PDH_FMT_DOUBLE;

  } else if (!counterFormatStr.compare("int") 
    || !counterFormatStr.compare("long")) {
    m_dwCounterFormat = PDH_FMT_LONG;

  } else if (!counterFormatStr.compare("int64") 
    || !counterFormatStr.compare("longlong") 
    || !counterFormatStr.compare("large")) {
    m_dwCounterFormat = PDH_FMT_LARGE;

  } else {
	  _Module.LogError("PerfCounter plugin: %s: Unknown CounterFormat", m_Name.c_str());
	  assert(!"Unknown CounterFormat!");
  }

  m_CounterMultiply = g_Config.GetValueF(m_SectionName, "CounterMultiply", 1.0);

  return true;
}

int PerfCounterMuninNodePlugin::GetConfig(char *buffer, int len) 
{  
  if (!m_Counters.empty()) {
    PDH_STATUS status;  
    DWORD infoSize = 0;
    status = PdhGetCounterInfo(m_Counters[0], TRUE, &infoSize, NULL);
    if (status != PDH_MORE_DATA)
      return -1;

    PDH_COUNTER_INFO *info = (PDH_COUNTER_INFO *)malloc(infoSize);
    status = PdhGetCounterInfo(m_Counters[0], TRUE, &infoSize, info);
    if (status != ERROR_SUCCESS) {
		free(info);
		return -1;
	}

    int printCount;
    std::string graphTitle = g_Config.GetValue(m_SectionName, "GraphTitle", "Disk Time");
    std::string graphCategory = g_Config.GetValue(m_SectionName, "GraphCategory", "system");
    std::string graphArgs = g_Config.GetValue(m_SectionName, "GraphArgs", "--base 1000 -l 0");
    std::string explainText = info->szExplainText ? W2AConvert(info->szExplainText) : m_CounterNames[0].c_str();
	std::string counterName = W2AConvert(info->szCounterName);
    printCount = _snprintf(buffer, len, "graph_title %s\n"
      "graph_category %s\n"
      "graph_args %s\n"
      "graph_info %s\n"
      "graph_vlabel %s\n", 
      graphTitle.c_str(), graphCategory.c_str(), 
      graphArgs.c_str(),
	  explainText.c_str(), counterName.c_str());
    len -= printCount;
    buffer += printCount;

    free(info);

    std::string graphDraw = g_Config.GetValue(m_SectionName, "GraphDraw", "LINE");
	std::string counterType = g_Config.GetValue(m_SectionName, "CounterType", "GAUGE");

	std::string minValue;
	std::string minValueNumbered;

	if(counterType == "DERIVE") {
		minValue = "%s.min 0\n";
		minValueNumbered = "%s_%i_.min 0\n";
	} else {
		minValue = minValueNumbered = "";
	}

    assert(m_CounterNames.size() == m_Counters.size());

	std::string labels;

      // We handle multiple counters
      for (size_t i = 0; i < m_CounterNames.size(); i++) {
        if (i == 0) {

		  labels = "%s.label %s\n"
				   "%s.draw %s\n"
				   "%s.type %s\n";
		  labels += minValue;

          // First counter gets a normal name
          printCount = _snprintf(buffer, len, 
			labels.c_str(),
            m_Name.c_str(), m_CounterNames[i].c_str(),
            m_Name.c_str(), graphDraw.c_str(),
			m_Name.c_str(), counterType.c_str(),
			m_Name.c_str());
        } else {
          // Rest of the counters are numbered

		  labels = "%s_%i_.label %s\n"
 				   "%s_%i_.draw %s\n"
				   "%s_%i_.type %s\n";
		  labels += minValueNumbered;

          printCount = _snprintf(buffer, len, 
            labels.c_str(),
            m_Name.c_str(), i, m_CounterNames[i].c_str(),
            m_Name.c_str(), i, graphDraw.c_str(),
			m_Name.c_str(), i, counterType.c_str(),
			m_Name.c_str(), i);
        }
        len -= printCount;
        buffer += printCount;
      }
    }

  strncat(buffer, ".\n", len);
  return 0;
}

int printvalue(char* buffer, size_t len, const char* name, size_t i, double value, DWORD counterformat) {
	if(counterformat == PDH_FMT_LONG)
		if(0==i)
			return _snprintf(buffer, len, "%s.value %i\n", name, (int)value);
		else
			return _snprintf(buffer, len, "%s_%i_.value %i\n", name, i, (int)value);
	else 
		if(0==i)
			return _snprintf(buffer, len, "%s.value %.2f\n", name, value);
		else
			return _snprintf(buffer, len, "%s_%i_.value %.2f\n", name, i, value);
}

int PerfCounterMuninNodePlugin::GetValues(char *buffer, int len)
{
  PDH_STATUS status;
  PDH_FMT_COUNTERVALUE counterValue;
  int printCount;

  status = PdhCollectQueryData(m_PerfQuery);
  if (status != ERROR_SUCCESS)
    return -1;  

  for (size_t i = 0; i < m_Counters.size(); i++) {
    // Get the formatted counter value    
    status = PdhGetFormattedCounterValue(m_Counters[i], m_dwCounterFormat, NULL, &counterValue);
    if (status != ERROR_SUCCESS)
      return -1;
    double value = 0;
    switch (m_dwCounterFormat) {
      case PDH_FMT_DOUBLE:        
        value = counterValue.doubleValue * m_CounterMultiply;
        break;
      case PDH_FMT_LONG:
        value = counterValue.longValue * m_CounterMultiply;
        break;
      case PDH_FMT_LARGE:
        value = counterValue.largeValue * m_CounterMultiply;
        break;
    }
    printCount = printvalue(buffer, len, m_Name.c_str(), i, value, m_dwCounterFormat);
    len -= printCount;
    buffer += printCount;
  }
  strncat(buffer, ".\n", len);
  return 0;
}
