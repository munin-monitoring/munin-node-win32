#pragma once
#include "../../core/MuninNodePlugin.h"
#include "SmartReader.h"

class SMARTMuninNodePlugin : public MuninNodePlugin {  
public:
  SMARTMuninNodePlugin();
  virtual ~SMARTMuninNodePlugin();

  virtual const char *GetName() { return "smart"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return false; };
};
