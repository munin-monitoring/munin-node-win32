#pragma once
#include "../../../core/MuninNodePlugin.h"
#include "../SMARTMuninNodePlugin.h"

class PowerOffRetractMuninNodePlugin : public MuninNodePlugin {
public:
    PowerOffRetractMuninNodePlugin();
  virtual ~PowerOffRetractMuninNodePlugin();

  virtual const char *GetName() { return "poweroffretract"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
