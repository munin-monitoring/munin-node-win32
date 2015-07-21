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
#include "deelx.h"

const char *PerfCounterMuninNodePlugin::SectionPrefix = "PerfCounterPlugin_";

PerfCounterMuninNodePlugin::PerfCounterMuninNodePlugin(const std::string &sectionName)
  : m_SectionName(sectionName)
{
  m_PerfQuery = NULL;

  DWORD regBufferSize = 65536;
  TCHAR *regBuffer;
  DWORD status;

  do {
	  // allocate memory and try to query registry
	  regBuffer = new TCHAR[regBufferSize];
	  status = RegQueryValueEx(HKEY_PERFORMANCE_DATA, L"Counter 009", NULL, NULL, (LPBYTE)regBuffer, &regBufferSize);

	  if (status == ERROR_MORE_DATA) {
		  // resize
		  delete regBuffer;
		  regBufferSize += 16384;
	  }
	  else {
		  if (status != ERROR_SUCCESS) {
			  delete regBuffer;
			  regBufferSize = 0;
			  _Module.LogEvent("PerfCounter plugin: %s: RegQueryValueEx error=%x", m_SectionName.c_str(), status);
			  break;
		  }
	  }
  } while (status != ERROR_SUCCESS);

  if (regBufferSize > 0) {
	  // create map of pdh index -> pdh name
	  for (TCHAR *idx = regBuffer; *idx; idx += _tcslen(idx) + 1) {
		  englishCounterNames[_ttol(idx)] = (idx += _tcslen(idx) + 1);
	  }
	  delete regBuffer;
  }

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
	if (englishCounterNames.empty()) {
		return englishName;
	}
	DWORD fIndex = 0;
	// try to find the name in the map of pdh objects&counters
	for (std::map<DWORD, TString>::iterator it = englishCounterNames.begin(); it != englishCounterNames.end(); ++it) {
		if (!it->second.compare(englishName)) {
			if (fIndex) {
				_Module.LogEvent("PerfCounter plugin: %s: duplicate %ls, idx %i/%i", m_SectionName.c_str(), englishName, fIndex, it->first);
				break;
			}
			fIndex = it->first;
		}
	}
	// not found
	if (fIndex <= 0)
		return englishName;

	DWORD status;
	DWORD bufSize = 0;
	PdhLookupPerfNameByIndex(NULL, fIndex, NULL, &bufSize);
	TCHAR *localName = new TCHAR[bufSize];
	status = PdhLookupPerfNameByIndex(NULL, fIndex, localName, &bufSize);
	if (status == PDH_INSUFFICIENT_BUFFER) {
		delete localName;
		bufSize = 1024;
		TCHAR *localName = new TCHAR[bufSize];
		status = PdhLookupPerfNameByIndex(NULL, fIndex, localName, &bufSize);
	}
	if (status != ERROR_SUCCESS) {
		// can happen, i.e. if the name is "*"
		delete localName;
		_Module.LogEvent("PerfCounter plugin: %s: Could not find a local name for %ls, error=%x", m_Name.c_str(), englishName, status);
		return englishName;
	}
	return localName;
}

const TCHAR *PerfCounterMuninNodePlugin::GetPdhCounterEnglishName(const TCHAR *localName)
{
	DWORD status;
	DWORD fIndex;
	status = PdhLookupPerfIndexByNameW(NULL, localName, &fIndex);
	if (status == ERROR_SUCCESS && englishCounterNames.count(fIndex) > 0) {
		return englishCounterNames.at(fIndex).c_str();
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
  TString instanceName = A2TConvert(g_Config.GetValue(m_SectionName, "Instance", "*"));

  bool useEnglishNames = g_Config.GetValueB(m_SectionName, "UseEnglishObjectNames", true);
  if (useEnglishNames) {
	  counterName = GetPdhCounterLocalizedName(counterName.c_str());
	  objectName = GetPdhCounterLocalizedName(objectName.c_str());
	  // NB: instance names are not localized
  }

  TCHAR *counterPath = new TCHAR[MAX_PATH];

  status = _sntprintf(counterPath, MAX_PATH, _T("\\%s(%s)\\%s"), objectName.c_str(), instanceName.c_str(), counterName.c_str());
  DWORD pathListBufsz = 0;
  status = PdhExpandWildCardPath(NULL, counterPath, NULL, &pathListBufsz, 0);
  if (status != PDH_MORE_DATA && status != PDH_INSUFFICIENT_BUFFER) {
	  _Module.LogError("PerfCounter plugin: %s: PdhExpandWildCardPath error=%x", m_Name.c_str(), status);
	  return false;
  }
  TCHAR *pathList = new TCHAR[pathListBufsz+2];
  status = PdhExpandWildCardPath(NULL, counterPath, (PZZTSTR)pathList, &pathListBufsz, 0);
  if (status != ERROR_SUCCESS) {
	  _Module.LogError("PerfCounter plugin: %s: PdhExpandWildCardPath error=%x", m_Name.c_str(), status);
	  return false;
  }

  // read config
  TString includeRE = A2TConvert(g_Config.GetValue(m_SectionName, "IncludePaths", ".+"));
  TString excludeRE = A2TConvert(g_Config.GetValue(m_SectionName, "ExcludePaths", "^$"));

  // create regex matchers
  CRegexpT <TCHAR> includeRegEx(includeRE.c_str(), IGNORECASE);
  CRegexpT <TCHAR> excludeRegEx(excludeRE.c_str(), IGNORECASE);

  HCOUNTER counterHandle;
  for (; *pathList; pathList += _tcslen(pathList) + 1) {
	  /* anatomy of a PDH path: \\machine\object(instance)\counter
	   * anomalies of a PDH path: the instance name is optional, and
	   * may contain parantheses and/or backslashes
	   * thus, the safest way is to check for the first '(' and the
	   * last '\' to separate the object, counter, instance names
	   */
	  int backslashes[2] = {0, 0};
	  int para = 0;
	  int idx;
	  for (idx = 2; pathList[idx]; idx++) {
		  if (pathList[idx] == '\\') {
			  if (backslashes[0]) {
				  backslashes[1] = idx;
			  } else {
				  backslashes[0] = idx;
			  }
		  } else if (pathList[idx] == '(' && (!para) && backslashes[0] && (!backslashes[1])) {
			  para = idx;
		  }
	  }
	  TCHAR *matchPath;
	  // use regex matching either on the original path or the
	  // back-translated constructed path
	  if (useEnglishNames) {
		  DWORD instPos = para ? para : backslashes[1];
		  TCHAR *oName = new TCHAR[instPos - backslashes[0]];
		  TCHAR *cName = new TCHAR[idx - backslashes[1] + 1];
		  TCHAR *iName = new TCHAR[backslashes[1] - para + 1];
		  // copy the object name
		  wcsncpy(oName, pathList + backslashes[0] + 1, instPos - backslashes[0] - 1);
		  oName[instPos - backslashes[0] - 1] = 0;
		  // copy the counter name (end of complete path, so \0 already exists)
		  wcsncpy(cName, pathList + backslashes[1] + 1, idx - backslashes[1]);
		  cName[idx - backslashes[1]] = 0;
		  // if there's an instance name, copy it, too.
		  if (para) {
			  wcsncpy(iName, pathList + para, backslashes[1] - para);
		  }
		  iName[backslashes[1] - para] = 0;
		  matchPath = new TCHAR[MAX_PATH];
		  _snwprintf(matchPath, MAX_PATH, _T("\\%s%s\\%s"), GetPdhCounterEnglishName(oName), iName, GetPdhCounterEnglishName(cName));
		  delete oName;
		  delete cName;
		  delete iName;
	  } else {
		  matchPath = pathList;
	  }
	  // handle includes & excludes
	  if (!includeRegEx.MatchExact(matchPath).IsMatched()
		  || excludeRegEx.MatchExact(matchPath).IsMatched()) {
		  if (useEnglishNames) {
			  delete matchPath;
		  }
		  continue;
	  }
	  if (useEnglishNames) {
		  delete matchPath;
	  }
	  // add the counter
	  status = PdhAddCounter(m_PerfQuery, pathList, 0, &counterHandle);
	  if (status != ERROR_SUCCESS) {
		  _Module.LogError("PerfCounter plugin: %s: PDH add counter error=%x", m_Name.c_str(), status);
		  return false;
	  }

	  // replace the backslash before the counter name with a space
	  pathList[backslashes[1]] = ' ';

	  std::string cn = W2IConvert(pathList + (para ? para : (backslashes[1] + 1)));
	  _Module.LogEvent("PerfCounter plugin: %s: added counter %s", m_Name.c_str(), cn.c_str());
	  m_Counters.push_back(counterHandle);
	  m_CounterNames.push_back(cn);
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
		if (status != PDH_MORE_DATA && status != PDH_INSUFFICIENT_BUFFER)
			return -1;

		PDH_COUNTER_INFO *info = (PDH_COUNTER_INFO *)malloc(infoSize);
		status = PdhGetCounterInfo(m_Counters[0], TRUE, &infoSize, info);
		if (status != ERROR_SUCCESS)
			return -1;

		int printCount;
		std::string graphTitle = g_Config.GetValue(m_SectionName, "GraphTitle", "Disk Time");
		std::string graphCategory = g_Config.GetValue(m_SectionName, "GraphCategory", "system");
		std::string graphArgs = g_Config.GetValue(m_SectionName, "GraphArgs", "--base 1000 -l 0");
		std::string explainText = g_Config.GetValue(m_SectionName, "GraphInfo",
			info->szExplainText ? W2IConvert(info->szExplainText) : m_CounterNames[0].c_str());
		std::string counterName = g_Config.GetValue(m_SectionName, "GraphVLabel", W2IConvert(info->szCounterName));
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

		if (counterType == "DERIVE") {
			minValue = "%s.min 0\n";
			minValueNumbered = "%s_%i_.min 0\n";
		}
		else {
			minValue = minValueNumbered = "";
		}

		assert(m_CounterNames.size() == m_Counters.size());

		std::string labels;

		// We handle multiple counters
		for (size_t i = 0; i < m_CounterNames.size(); i++) {
			PDH_STATUS status;
			DWORD infoSize = 0;
			status = PdhGetCounterInfo(m_Counters[i], TRUE, &infoSize, NULL);
			if (status != PDH_MORE_DATA && status != PDH_INSUFFICIENT_BUFFER)
				return -1;

			PDH_COUNTER_INFO *info = (PDH_COUNTER_INFO *)malloc(infoSize);
			status = PdhGetCounterInfo(m_Counters[i], TRUE, &infoSize, info);
			if (status != ERROR_SUCCESS) {
				free(info);
				return -1;
			}

			_Module.LogEvent("PerfCounter plugin: %s: counter id %i", m_Name.c_str(), info->dwType);

			DWORD npl = 260;
			TCHAR *np = new TCHAR[npl];
			PdhMakeCounterPath(&(info->CounterPath), np, &npl, 0);
			_Module.LogEvent("PerfCounter plugin: %s: counter path %ls, %ls", m_Name.c_str(), np, info->CounterPath.szObjectName);
			npl = 260;
			PdhMakeCounterPath(&(info->CounterPath), np, &npl, PDH_PATH_WBEM_INPUT);
			_Module.LogEvent("PerfCounter plugin: %s: counter path %ls, %ls", m_Name.c_str(), np, info->CounterPath.szObjectName);

			std::string explainText = info->szExplainText ? W2IConvert(info->szExplainText) : m_CounterNames[i].c_str();
			if (i == 0) {

				labels = "%s.label %s\n"
					"%s.draw %s\n"
					"%s.type %s\n"
					"%s.info %s\n";
				labels += minValue;

				// First counter gets a normal name
				printCount = _snprintf(buffer, len,
					labels.c_str(),
					m_Name.c_str(), m_CounterNames[i].c_str(),
					m_Name.c_str(), graphDraw.c_str(),
					m_Name.c_str(), counterType.c_str(),
					m_Name.c_str(), explainText.c_str(),
					m_Name.c_str());
			}
			else {
				// Rest of the counters are numbered

				labels = "%s_%i_.label %s\n"
					"%s_%i_.draw %s\n"
					"%s_%i_.type %s\n"
					"%s_%i_.info %s\n";
				labels += minValueNumbered;

				printCount = _snprintf(buffer, len,
					labels.c_str(),
					m_Name.c_str(), i, m_CounterNames[i].c_str(),
					m_Name.c_str(), i, graphDraw.c_str(),
					m_Name.c_str(), i, counterType.c_str(),
					m_Name.c_str(), i, explainText.c_str(),
					m_Name.c_str(), i);
			}
			free(info);
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
