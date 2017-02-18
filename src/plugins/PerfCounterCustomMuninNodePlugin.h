#pragma once
#include "core/MuninNodePlugin.h"

struct Field {
  std::string name;
  std::string preParsedArgs;

  // PerfCounter API params
  struct Counter {
    std::string path;
    HCOUNTER handle;
    DWORD format;
    double multiply;
  } counter;
};

class PerfCounterCustomMuninNodePlugin : public MuninNodePluginHelper
{
public:

  /// \param sectionName The INI File section name for this plugin
  PerfCounterCustomMuninNodePlugin(const std::string &sectionName);
  virtual ~PerfCounterCustomMuninNodePlugin();

  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return m_Loaded; };

  static const char *SectionPrefix;
private:
  bool OpenCounter();

  bool m_Loaded;
  std::string m_SectionName;

  HQUERY m_PerfQuery;

  // All the fields are represented here
  std::map<std::string, Field> m_Fields;

  // Precomputed graph parameters
  std::string m_GraphParameters;
};
