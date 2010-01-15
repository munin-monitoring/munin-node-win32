#pragma once
#include "../../core/MuninNodePlugin.h"
#include "mbm.h"

class MBMMuninNodePlugin : public MuninNodePlugin
{
  char m_Name[32];
  mbm::sensor_t m_Mode;
  mbm::mbm m_MBM;
public:
  MBMMuninNodePlugin(mbm::sensor_t mode);
  virtual ~MBMMuninNodePlugin();

  virtual const char *GetName() { return m_Name; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded();
};
