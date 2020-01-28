#pragma once
#include "../../../core/MuninNodePlugin.h"
#include "../SMARTMuninNodePlugin.h"

class OnlineMuninNodePlugin : public MuninNodePlugin {
public:
  OnlineMuninNodePlugin();
  virtual ~OnlineMuninNodePlugin();

  virtual const char *GetName() { return "onlinehours"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
