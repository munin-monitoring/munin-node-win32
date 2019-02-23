#pragma once
#include "../core/MuninNodePlugin.h"

class PerfCounterMuninNodePlugin : public MuninNodePluginHelper
{
public:
  /// \param sectionName The INI File section name for this plugin
  PerfCounterMuninNodePlugin(const std::string &sectionName);
  virtual ~PerfCounterMuninNodePlugin();

  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return m_Loaded; };

  static const char *SectionPrefix;
private:
  bool VerifyOS();
  bool CreateQuery();
  void GetNames(TString& objectName, TString& counterName);
  bool GetCounters();
  bool EnumCounters(TString& objectName);
  bool InitCounters(TString& objectName, TString& counterName);
  bool CollectInitData();
  bool SetupFormat();
  bool OpenCounter();
  const TCHAR *PerfCounterMuninNodePlugin::GetPdhCounterLocalizedName(const TCHAR *englishName);

  bool m_Loaded;
  bool m_HasInstances;
  std::string m_SectionName;
  DWORD m_dwCounterFormat;
  double m_CounterMultiply;
  HQUERY m_PerfQuery;
  std::vector<std::string> m_CounterNames;
  std::vector<HCOUNTER> m_Counters;
};
