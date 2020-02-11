#pragma once
#include "../../../core/MuninNodePlugin.h"
#include "../SMARTMuninNodePlugin.h"

class StartStopMuninNodePlugin : public MuninNodePlugin {
public:
    StartStopMuninNodePlugin();
  virtual ~StartStopMuninNodePlugin();

  virtual const char *GetName() { return "startstopcyclehdd"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
