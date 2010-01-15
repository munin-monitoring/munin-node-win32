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
#include "registry.h"

unsigned __int64 ntoh64(unsigned __int64 inval)
{
	unsigned __int64 outval = 0;
	int i=0;
	for(i=0;i<8; i++)
		outval=(outval<<8)+ ((inval >> (i *8))&255);
	return outval;
}

void CRegistry::Check()
{
	LONG ret, len;
	ret = RegQueryValue(m_key, L"", NULL, &len);
	if (ret == ERROR_KEY_DELETED ||
		ret == ERROR_INVALID_HANDLE)
	{
		if (m_key)
			RegCloseKey(m_key);
		RegOpenKeyEx( m_rootkey, m_strkey.c_str(), 0, KEY_READ, &m_key);
	}
}

string CRegistry::GetValue(const char* valName, string const defValue)
{
	DWORD type, len = DEFAULT_VALUE_SIZE;
	TCHAR stackValue[DEFAULT_VALUE_SIZE];
	LONG ret;

	TString tValueName = A2TConvert(valName);

	ret = RegQueryValueEx(m_key, 
		tValueName.c_str(), 
		NULL,
		&type, 
		reinterpret_cast<LPBYTE>(stackValue),
		&len);
	if (type != REG_SZ)
		return defValue;

	if (ret == ERROR_SUCCESS)		
		return T2AConvert(stackValue);

	if (ret != ERROR_MORE_DATA)
		return defValue;

	BYTE* buffer = new BYTE[len];
	if (buffer)
	{
		ret =  RegQueryValueEx(m_key, 
			tValueName.c_str(), 
			NULL,
			&type, 
			buffer,
			&len);
		if (ret == ERROR_SUCCESS)		
		{
			string r = T2AConvert( reinterpret_cast<TCHAR*>(buffer) );
			delete[] buffer;
			return r;
		}
		else
			delete[] buffer;
	}
	return defValue;
}

int CRegistry::GetValueI(string const valName, int const defValue)
{
	DWORD type, len = sizeof(DWORD);
	DWORD val;
	LONG ret;

	TString tValueName = A2TConvert(valName);

	ret = RegQueryValueEx(m_key, 
		tValueName.c_str(), 
		NULL,
		&type, 
		reinterpret_cast<LPBYTE>(&val),
		&len);
	if (type != REG_DWORD || ret != ERROR_SUCCESS)
		return defValue;
	return val;
}

double CRegistry::GetValueF(string const valName, double const defValue)
{
	DWORD type, len = DEFAULT_VALUE_SIZE;
	TCHAR stackValue[DEFAULT_VALUE_SIZE];
	LONG ret;

	TString tValueName = A2TConvert(valName);

	ret = RegQueryValueEx(m_key, 
		tValueName.c_str(), 
		NULL,
		&type, 
		reinterpret_cast<LPBYTE>(stackValue),
		&len);
	if ((type != REG_SZ && type != REG_DWORD && type != REG_BINARY) || 
		ret != ERROR_SUCCESS)
		return defValue;

	if (type == REG_SZ)
		return atof( T2AConvert(stackValue).c_str() ); 
	else if (type = REG_DWORD)
		return *((DWORD*)stackValue);
	else
	{
		//TODO: binary data treat as __int64
		return defValue;
	}
}

__int64 CRegistry::GetValueI64(string const valName, __int64 defValue)
{
	DWORD type, len = DEFAULT_VALUE_SIZE;
	TCHAR stackValue[DEFAULT_VALUE_SIZE];
	LONG ret;

	TString tValueName = A2TConvert(valName);

	ret = RegQueryValueEx(m_key, 
		tValueName.c_str(), 
		NULL,
		&type, 
		reinterpret_cast<LPBYTE>(stackValue),
		&len);
	if (type != REG_SZ || ret != ERROR_SUCCESS)
		return defValue;

	return _atoi64( T2AConvert(stackValue).c_str() ); 
}

void CRegistry::EnumKeys(vector<string>& keys)
{
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys=0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode; 

	DWORD cchValue = MAX_VALUE_NAME; 

	if (!m_key)
		return;

	retCode = RegQueryInfoKey(
		m_key,                    
		achClass,                
		&cchClassName,           
		NULL,                    
		&cSubKeys,               
		&cbMaxSubKey,            
		&cchMaxClass,            
		&cValues,                
		&cchMaxValue,            
		&cbMaxValueData,         
		&cbSecurityDescriptor,
		&ftLastWriteTime);    

	if (cSubKeys)
	{
		for (i=0; i<cSubKeys; i++) 
		{ 
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(m_key, i,
				achKey, 
				&cbName, 
				NULL, 
				NULL, 
				NULL, 
				&ftLastWriteTime); 
			if (retCode == ERROR_SUCCESS) 
			{
				keys.push_back(T2AConvert(achKey));
			}
		}
	} 
}

vector<string> CRegistry::GetValues(const char* valName)
{
	vector<string> l;

	DWORD type, len = DEFAULT_VALUE_SIZE;
	TCHAR stackValue[DEFAULT_VALUE_SIZE];
	LONG ret;

	TString tValueName = A2TConvert(valName);

	ret = RegQueryValueEx(m_key, 
		tValueName.c_str(), 
		NULL,
		&type, 
		reinterpret_cast<LPBYTE>(stackValue),
		&len);
	if (type != REG_MULTI_SZ)
		return l;

	if (ret == ERROR_SUCCESS)
	{
		convert_multisz(stackValue, len, l);
		return l;
	}

	if (ret != ERROR_MORE_DATA)
		return l;

	BYTE* buffer = new BYTE[len];
	if (buffer)
	{
		ret =  RegQueryValueEx(m_key, 
			tValueName.c_str(), 
			NULL,
			&type, 
			buffer,
			&len);
		if (ret == ERROR_SUCCESS)		
		{
			convert_multisz(reinterpret_cast<TCHAR*>(buffer), len, l);			
			delete[] buffer;
			return l;
		}
		else
			delete[] buffer;
	}
	return l;
}

void CRegistry::convert_multisz(TCHAR* s, DWORD len, vector<string>& l)
{
	while(len > 0 && *s)
	{
		size_t sub_len = wcslen(s);
		l.push_back(T2AConvert(s));
		s += sub_len + 1;
	}	
}
