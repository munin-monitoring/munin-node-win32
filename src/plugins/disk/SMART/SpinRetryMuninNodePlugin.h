#pragma once
#include "../../../core/MuninNodePlugin.h"
#include "../SMARTMuninNodePlugin.h"

class SpinRetryMuninNodePlugin : public MuninNodePlugin {
public:
    SpinRetryMuninNodePlugin();
  virtual ~SpinRetryMuninNodePlugin();

  virtual const char *GetName() { return "spinretrycount"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
