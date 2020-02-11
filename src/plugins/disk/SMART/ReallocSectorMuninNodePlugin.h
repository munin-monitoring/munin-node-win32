#pragma once
#include "../../../core/MuninNodePlugin.h"
#include "../SMARTMuninNodePlugin.h"

class ReallocSectorMuninNodePlugin : public MuninNodePlugin {
public:
  ReallocSectorMuninNodePlugin();
  virtual ~ReallocSectorMuninNodePlugin();

  virtual const char *GetName() { return "reallocsectorcount"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
