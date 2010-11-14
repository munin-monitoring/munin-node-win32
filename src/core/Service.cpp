// Service.cpp: implementation of the CService class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Service.h"
#include "MuninNodeServer.h"

CService _Module;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CService::CService() : m_bQuiet(false)
{
  
}

CService::~CService()
{
  
}

void CService::Init(LPCTSTR pServiceName,LPCTSTR pServiceDisplayedName)
{
  lstrcpy(m_szServiceName,pServiceName);
  lstrcpy(m_szServiceDisplayedName,pServiceDisplayedName);

  // set up the initial service status 
  m_hServiceStatus = NULL;
  m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  m_status.dwCurrentState = SERVICE_STOPPED;
  m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  m_status.dwWin32ExitCode = 0;
  m_status.dwServiceSpecificExitCode = 0;
  m_status.dwCheckPoint = 0;
  m_status.dwWaitHint = 0;

  // Setup Event Log
  m_EventLog.Init(m_szServiceName);
}

void CService::Start()
{
  SERVICE_TABLE_ENTRY st[] =
  {
    { m_szServiceName, _ServiceMain },
    { NULL, NULL }
  };
  if (m_bService) 
  {
    if (!::StartServiceCtrlDispatcher(st))
    {
      DWORD dw = GetLastError();
      LogEvent("StartServiceCtrlDispatcher Error=%d",dw);
      m_bService = FALSE;
    }
  }

  if (m_bService == FALSE)
    Run();
}

void CService::ServiceMain()
{
  // Register the control request handler
  m_status.dwCurrentState = SERVICE_START_PENDING;
  m_hServiceStatus = RegisterServiceCtrlHandler(m_szServiceName, _Handler);
  if (m_hServiceStatus == NULL)
  {
    LogEvent("Handler not installed");
    return;
  }
  SetServiceStatus(SERVICE_START_PENDING);

  m_status.dwWin32ExitCode = S_OK;
  m_status.dwCheckPoint = 0;
  m_status.dwWaitHint = 0;

  // When the Run function returns, the service has stopped.
  Run();

  SetServiceStatus(SERVICE_STOPPED);
  LogEvent("Service stopped");
}

inline void CService::Handler(DWORD dwOpcode)
{
  switch (dwOpcode)
  {
  case SERVICE_CONTROL_STOP:
    LogEvent("Request to stop...");
    SetServiceStatus(SERVICE_STOP_PENDING);
    PostThreadMessage(m_dwThreadID, WM_QUIT, 0, 0);
    break;
  case SERVICE_CONTROL_PAUSE:
    break;
  case SERVICE_CONTROL_CONTINUE:
    break;
  case SERVICE_CONTROL_INTERROGATE:
    break;
  case SERVICE_CONTROL_SHUTDOWN:
    break;
  default:
    LogEvent("Bad service request");
    break;
  }
}

void WINAPI CService::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
  _Module.ServiceMain();
}
void WINAPI CService::_Handler(DWORD dwOpcode)
{
  _Module.Handler(dwOpcode); 
}

void CService::SetServiceStatus(DWORD dwState)
{
  m_status.dwCurrentState = dwState;
  ::SetServiceStatus(m_hServiceStatus, &m_status);
}

void CService::Run()
{
  LogEvent("Service started");
  m_dwThreadID = GetCurrentThreadId();

  if (m_bService)
    SetServiceStatus(SERVICE_RUNNING);
  
  // The service is running.
  MuninNodeServer *server = new MuninNodeServer();
  server->JCThread_AddRef();
  LogEvent("Starting Server Thread");
  server->Run();

  //LogEvent("Updating INI File");
  // Save any changes to the INI file
  // NAAAAH, never ever modify my settings
  // g_Config.WriteFile();

  if (m_bService) 
  {
    BOOL bRet;
    MSG msg;
    while ((bRet = GetMessage(&msg,NULL,NULL,NULL)) != 0)
    {
      if (bRet == -1)
      {
        // handle the error and possibly exit
        break;
      }
      else
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }
  else
  {
    // Let the server go for a few seconds
    Sleep(100 * 1000);
  }

  LogEvent("Stopping Server Thread");
  // The service is going to be stopped.
  server->Stop();
  server->JCThread_RemoveRef();
}

BOOL CService::Install()
{
  if (IsInstalled())
    return TRUE;

  SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (hSCM == NULL)
  {
    ShowMessage(_T("Couldn't open service manager"));
    return FALSE;
  }

  // Get the executable file path
  TCHAR szFilePath[_MAX_PATH];
  ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

  DWORD dwStartupType = SERVICE_AUTO_START;
  SC_HANDLE hService = ::CreateService(
    hSCM, m_szServiceName, m_szServiceDisplayedName,
    SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
    dwStartupType, SERVICE_ERROR_NORMAL,
    szFilePath, NULL, NULL, NULL, NULL, NULL);

  if (hService == NULL)
  {
    ::CloseServiceHandle(hSCM);
    ShowMessage(_T("Couldn't create service"));
    return FALSE;
  }

  if (dwStartupType == SERVICE_AUTO_START)
    StartService(hService, 0, NULL);

  ::CloseServiceHandle(hService);
  ::CloseServiceHandle(hSCM);
  return TRUE;
}

BOOL CService::Uninstall(DWORD dwTimeout)
{
  if (!IsInstalled())
    return TRUE;

  SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if (hSCM == NULL)
  {
    ShowMessage(_T("Couldn't open service manager"));
    return FALSE;
  }

  SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_STOP | DELETE | SERVICE_QUERY_STATUS);

  if (hService == NULL)
  {
    ::CloseServiceHandle(hSCM);
    ShowMessage(_T("Couldn't open service"));
    return FALSE;
  }
  SERVICE_STATUS status = {0};
  DWORD dwStartTime = GetTickCount();

  if (ControlService(hService, SERVICE_CONTROL_STOP, &status))
  {
    // Wait for the service to stop
    while ( status.dwCurrentState != SERVICE_STOPPED )
    {
      Sleep( status.dwWaitHint );
      if ( !QueryServiceStatus( hService, &status ) )
        return FALSE;

      if ( status.dwCurrentState == SERVICE_STOPPED )
        break;

      if ( GetTickCount() - dwStartTime > dwTimeout )
      {
        ShowMessage(_T("Service could not be stopped"));
        return FALSE;
      }
    }
  }

  if (!m_EventLog.UnRegisterSource()) 
  {
    ShowMessage(_T("Failed to unregister event log"));
  }

  BOOL bDelete = ::DeleteService(hService);
  ::CloseServiceHandle(hService);
  ::CloseServiceHandle(hSCM);

  if (bDelete)
    return TRUE;

  ShowMessage(_T("Service could not be deleted"));
  return FALSE;
}

BOOL CService::IsInstalled()
{
  BOOL bResult = FALSE;

  SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if (hSCM != NULL)
  {
    SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_QUERY_CONFIG);
    if (hService != NULL)
    {
      bResult = TRUE;
      ::CloseServiceHandle(hService);
    }
    ::CloseServiceHandle(hSCM);
  }
  return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////
// Logging functions
void CService::ShowMessage(LPCTSTR szMessage)
{
  if (m_bQuiet)
    _tprintf(_T("%s\n"), szMessage);
  else
    MessageBox(NULL, szMessage, m_szServiceName, MB_OK);
}

void CService::LogEvent(LPCSTR pFormat, ...)
{
  char chMsg[512];    
  va_list pArg;
  va_start(pArg, pFormat);
  _vsnprintf(chMsg, 512, pFormat, pArg);
  va_end(pArg);
  chMsg[511] = 0; 

  if (m_bService)
  {
    bool debuglog = g_Config.GetValueB("MuninNode","DebugLog", false);
    if(debuglog)
      m_EventLog.Write(EVENTLOG_INFORMATION_TYPE, A2TConvert(chMsg).c_str());
  }
  else
  {
    // As we don't have an event log handle, just write the error to the console.
    printf(chMsg);
    printf("\n");
    fflush(stdout);
  }
}

void CService::LogError(LPCSTR pFormat, ...)
{
  char chMsg[512];    
  va_list pArg;
  va_start(pArg, pFormat);
  _vsnprintf(chMsg, 512, pFormat, pArg);
  va_end(pArg);
  chMsg[511] = 0; 

  if (m_bService)
  {
    bool debuglog = g_Config.GetValueB("MuninNode","DebugLog", false);
    if(debuglog) m_EventLog.Write(EVENTLOG_INFORMATION_TYPE, A2TConvert(chMsg).c_str());
  }
  else
  {
    // As we don't have an event log handle, just write the error to the console.
	printf("ERROR:");
    printf(chMsg);
    printf("\n");
    fflush(stdout);
  }
}
