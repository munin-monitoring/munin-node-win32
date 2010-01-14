#pragma once
#include "../../core/MuninNodePlugin.h"

class DiskMuninNodePlugin : public MuninNodePlugin {
  char drives[32][4];
public:
  DiskMuninNodePlugin();
  virtual ~DiskMuninNodePlugin();

  virtual const char *GetName() { return "df"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
  virtual bool IsThreadSafe() { return true; };
};
