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
#include "PythonMuninNodePlugin.h"

#include <DelayImp.h>

LONG WINAPI DelayLoadDllExceptionFilter(PEXCEPTION_POINTERS pExcPointers) {
   LONG lDisposition = EXCEPTION_EXECUTE_HANDLER;
   PDelayLoadInfo pDelayLoadInfo =
    PDelayLoadInfo(pExcPointers->ExceptionRecord->ExceptionInformation[0]);

   if (pExcPointers->ExceptionRecord->ExceptionCode == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND)
     && strstr(pDelayLoadInfo->szDll, "python")) {
       // Handle Python not being present
        return EXCEPTION_EXECUTE_HANDLER;
   }

   return EXCEPTION_CONTINUE_SEARCH;
}

class PythonLoader : public JCRefCount {
public:
  PythonLoader() {
    m_bIsLoaded = PythonLoader::Init();
  };
  ~PythonLoader() {
    if (m_bIsLoaded)
      Py_Finalize();
  }

  static bool Init() {
    __try {
      // Setup Python
      Py_Initialize();
      return true;
   } __except (DelayLoadDllExceptionFilter(GetExceptionInformation())) {
      // Prepare to exit elegantly
   }
    return false;
  }

  bool IsLoaded() { return m_bIsLoaded; };

private:
  bool m_bIsLoaded;
};

static PythonLoader *g_PythonLoader = NULL;

PythonMuninNodePlugin::PythonMuninNodePlugin(const char *filename) 
{
  m_pObject = NULL;
  if (g_PythonLoader == NULL) {
    g_PythonLoader = new PythonLoader();
  }
  g_PythonLoader->AddRef();
  if (!g_PythonLoader->IsLoaded())
    return;

  // Open and execute the Python file  
  FILE *exp_file = fopen(filename, "r");
  if (exp_file == NULL) {
    return;
  }
  fseek(exp_file, 0, SEEK_END);
  long len = ftell(exp_file);  
  char *script = new char[len+1];
  fseek(exp_file, 0, SEEK_SET);
  len = fread(script, 1, len, exp_file);
  fclose(exp_file);
  if (len > 0) {
    script[len] = 0;
  } else {
    script[0] = 0;
  }  
  PyRun_SimpleString(script); 
  delete [] script;

  PyObject *pMainModule = PyImport_AddModule("__main__");
  if (pMainModule) {
    m_pObject = PyObject_GetAttrString(pMainModule, "munin_node_plugin");
    Py_DECREF(pMainModule);
  }

  m_Name = "python_";
  PyObject *pValue = PyObject_CallMethod(m_pObject, "name", NULL);
  if (pValue != NULL) {
    char *str = PyString_AsString(pValue);
    if (str != NULL) {      
      m_Name += str;
    }
    Py_DECREF(pValue);
  }
  
}

PythonMuninNodePlugin::~PythonMuninNodePlugin()
{
  if (g_PythonLoader->IsLoaded() && m_pObject != NULL) {
    PyObject *pValue = PyObject_CallMethod(m_pObject, "close", NULL);
    Py_XDECREF(pValue);
    Py_DECREF(m_pObject);
  }
  if (g_PythonLoader->RemoveRef() == 0) {
    g_PythonLoader = NULL;
  }
}


int PythonMuninNodePlugin::GetConfig(char *buffer, int len) 
{
  int ret = -1;
  if (!g_PythonLoader->IsLoaded())
    return -1;

  PyObject *pValue = PyObject_CallMethod(m_pObject, "config", NULL);
  if (pValue != NULL) {
    char *str = PyString_AsString(pValue);
    if (str != NULL) {
      strncpy(buffer, str, len);
      ret = 0;
    }
    Py_DECREF(pValue);
  }

  return ret;
}

int PythonMuninNodePlugin::GetValues(char *buffer, int len) 
{
  int ret = -1;
  if (!g_PythonLoader->IsLoaded())
    return -1;

  PyObject *pValue = PyObject_CallMethod(m_pObject, "values", NULL);
  if (pValue != NULL) {
    char *str = PyString_AsString(pValue);
    if (str != NULL) {
      strncpy(buffer, str, len);  
      ret = 0;
    }
    Py_DECREF(pValue);
  }

  return ret;
}
