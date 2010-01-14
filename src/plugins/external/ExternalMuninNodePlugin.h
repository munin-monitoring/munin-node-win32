#pragma once
#include "../../core/MuninNodePlugin.h"
#include "consolePipe.h"

class ExternalMuninNodePlugin : public MuninNodePluginHelper
{
public:
  ExternalMuninNodePlugin(const std::string &externalPlugin);
  virtual ~ExternalMuninNodePlugin();

  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return !m_Name.empty(); };
  virtual bool IsThreadSafe() { return true; };

private:  
  class PluginPipe : public CConsolePipe {
  public:
    PluginPipe();
    virtual ~PluginPipe();

    virtual void OnReceivedOutput(LPCTSTR pszText);
    TString GetOutput();
  protected:
    JCCritSec m_BufferCritSec;
    TString m_Buffer;
  };
  std::string Run(const char *command);

  std::string m_ExternalPlugin;
};
