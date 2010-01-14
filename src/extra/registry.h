#ifndef CRegistry_H
#define CRegistry_H

#include "../core/TString.h"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#define DEFAULT_VALUE_SIZE 64

using namespace std;

class CRegistry
{
public:
	CRegistry(HKEY rootKey, const string& subKey) : m_key(NULL), m_rootkey(rootKey)
	{
		m_strkey = A2TConvert(subKey);
		RegOpenKeyEx( m_rootkey, m_strkey.c_str(), 0, KEY_READ, &m_key);
	}

	~CRegistry()
	{
		if (m_key)
			RegCloseKey(m_key);
	}

	void Check();

	string GetValue(const char* valName, string const defValue="");
	int GetValueI(string const valName, int const defValue = 0);
	bool   GetValueB(string const valName, bool const defValue = false) {
		return !!GetValueI( valName, int( defValue));
	}
	double  GetValueF(string const valName, double const defValue = 0.0);
	__int64 GetValueI64(string const valName, __int64 defValue = 0);
	vector<string> GetValues(const char* valName);

	void EnumKeys(vector<string>& keys);

private:
	HKEY m_rootkey;
	HKEY m_key;
	TString m_strkey;
	void convert_multisz(TCHAR* s, DWORD len, vector<string>& l);
};

#endif