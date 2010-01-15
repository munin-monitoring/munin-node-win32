#pragma once
#include "../../core/MuninNodePlugin.h"

class UptimeMuninNodePlugin : public MuninNodePlugin
{
  HQUERY m_PerfQuery;
  HCOUNTER m_UptimeCounter;
public:
  UptimeMuninNodePlugin();
  ~UptimeMuninNodePlugin();

  virtual const char *GetName() { return "uptime"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
