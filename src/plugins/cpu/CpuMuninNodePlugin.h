#pragma once
#include "../../core/MuninNodePlugin.h"


class CpuMuninNodePlugin : public MuninNodePlugin
{
public:
  CpuMuninNodePlugin();
  virtual ~CpuMuninNodePlugin();

  virtual const char *GetName() { return "cpu"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; }

private:
  void CalculateCpuLoad();

  double dbIdleTime;
  double dbSystemTime;  
  LARGE_INTEGER liOldIdleTime;
  LARGE_INTEGER liOldSystemTime;
};
