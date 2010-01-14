// Service.h: interface for the CService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVICE_H__CCA2ED69_EC91_11D5_966E_000347A347FE__INCLUDED_)
#define AFX_SERVICE_H__CCA2ED69_EC91_11D5_966E_000347A347FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../extra/XEventLog.h"

class CService  
{
public:
	CService();
	virtual ~CService();

	void Init(LPCTSTR pServiceName,LPCTSTR pServiceDisplayedName);
    void Start();
	void ServiceMain();
    void Handler(DWORD dwOpcode);
    void Run();
    BOOL IsInstalled();
    BOOL Install();
    BOOL Uninstall(DWORD dwTimeout = 10000);
	void LogEvent(LPCSTR pszFormat, ...);
    void SetServiceStatus(DWORD dwState);
    void SetQuiet(bool bQuiet) { m_bQuiet = bQuiet; };
    void ShowMessage(LPCTSTR szMessage);

    LPCTSTR GetServiceName() { return m_szServiceName; };
    LPCTSTR GetServiceDisplayName() { return m_szServiceDisplayedName; };
private:
	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI _Handler(DWORD dwOpcode);

	DWORD m_dwThreadID;
    CXEventLog m_EventLog;

public:
    TCHAR m_szServiceName[256];
	TCHAR m_szServiceDisplayedName[256];
    SERVICE_STATUS_HANDLE m_hServiceStatus;
    SERVICE_STATUS m_status;
	BOOL m_bService;
    bool m_bQuiet;
};

extern CService _Module;
#endif // !defined(AFX_SERVICE_H__CCA2ED69_EC91_11D5_966E_000347A347FE__INCLUDED_)
