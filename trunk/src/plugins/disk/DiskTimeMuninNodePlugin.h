#pragma once
#include "../../core/MuninNodePlugin.h"
#include <vector>
#include <string>
#include <PDHMsg.h>

class DiskTimeMuninNodePlugin : public MuninNodePlugin
{
public:
  DiskTimeMuninNodePlugin();
  ~DiskTimeMuninNodePlugin();

  virtual const char *GetName() { return "disktime"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return m_Loaded; };

private:
  bool OpenCounter();

  bool m_Loaded;
  HQUERY m_PerfQuery;
  std::vector<std::string> m_DiskTimeNames;
  std::vector<HCOUNTER> m_DiskTimeCounters;
};
