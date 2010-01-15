/* This file is part of munin-node-win32
* Copyright (C) 2006-2008 Chris Song (chris__song@hotmail.com)
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
#include "RegistryMuninNodePlugin.h"
#include "../extra/registry.h"

const char *RegistryMuninNodePlugin::SectionPrefix = "RegistryPlugin_";

RegistryMuninNodePlugin::RegistryMuninNodePlugin(const std::string &sectionName)
: m_SectionName(sectionName), m_RootKey(HKEY_LOCAL_MACHINE)
{
	std::string rootKey = g_Config.GetValue(m_SectionName, "RootKey", "HKLM");
	m_SubKey = g_Config.GetValue(m_SectionName, "SubKey", "Software\\munin-node");
	if (rootKey.find("HKCU") != string::npos ||
		rootKey.find("HKEY_CURRENT_USER") != string::npos)
		m_RootKey = HKEY_CURRENT_USER;

	m_Name = m_SectionName.substr(strlen(RegistryMuninNodePlugin::SectionPrefix));
}

RegistryMuninNodePlugin::~RegistryMuninNodePlugin()
{

}

int RegistryMuninNodePlugin::GetConfig(char *buffer, int len)
{
	CRegistry reg(m_RootKey, m_SubKey);
	reg.Check(); // reopen if reg is removed then added

	string graphTitle = reg.GetValue("graph_title", "Disk Time");
	string graphCategory = reg.GetValue("graph_category", "system");
	string graphArgs = reg.GetValue("graph_args", "--base 1000 -l 0");
	string graphInfo = reg.GetValue("graph_info", "disk time");
	string graphVlabel = reg.GetValue("graph_vlabel", "Disk Time");

	int printCount;
	printCount = _snprintf(buffer, len, "graph_title %s\n"
		"graph_category %s\n"
		"graph_args %s\n"
		"graph_info %s\n"
		"graph_vlabel %s\n", 
		graphTitle.c_str(), graphCategory.c_str(), 
		graphArgs.c_str(), graphInfo.c_str(), 
		graphVlabel.c_str() );
	len -= printCount;
	buffer += printCount;

	vector<string> keys;
	reg.EnumKeys(keys);

	// for keys, add definition
	for(vector<string>::iterator it=keys.begin(); it!=keys.end(); ++it)
	{
		string subkey(m_SubKey);
		CRegistry sub_reg(m_RootKey, subkey.append("\\").append(*it));
		string label = sub_reg.GetValue("label", "disk time");
		string draw = sub_reg.GetValue("draw", "Disk Time");

		printCount = _snprintf(buffer, len, "%s.label %s\n"
			"%s.draw %s\n", 
			it->c_str(), label.c_str(),
			it->c_str(), draw.c_str() );

		len -= printCount;
		buffer += printCount;
	}
	strncat(buffer, ".\n", len);

	return 0;
}

int RegistryMuninNodePlugin::GetValues(char *buffer, int len)
{
	CRegistry reg(m_RootKey, m_SubKey);
	vector<string> keys;
	reg.EnumKeys(keys);

	// for keys, add definition
	for(vector<string>::iterator it=keys.begin(); it!=keys.end(); ++it)
	{
		int printCount;
		string subkey(m_SubKey);
		CRegistry sub_reg(m_RootKey, subkey.append("\\").append(*it));
		double v = sub_reg.GetValueF("value", 0.0);
		
		printCount = _snprintf(buffer, len, "%s.value %.2f\n", it->c_str(), v);

		len -= printCount;
		buffer += printCount;
	}

	strncat(buffer, ".\n", len);

	return 0;
}


