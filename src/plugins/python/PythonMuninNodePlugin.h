#pragma once
#include "../../core/MuninNodePlugin.h"

class PythonMuninNodePlugin : public MuninNodePlugin
{
public:
  PythonMuninNodePlugin(const char *filename);
  ~PythonMuninNodePlugin();

  virtual const char *GetName() { return m_Name.c_str(); }; 
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return !!m_pObject; };

protected:
  std::string m_Name;
  PyObject *m_pObject;
};
