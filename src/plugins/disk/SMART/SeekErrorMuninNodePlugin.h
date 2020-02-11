#pragma once
#include "../../../core/MuninNodePlugin.h"
#include "../SMARTMuninNodePlugin.h"

class SeekErrorMuninNodePlugin : public MuninNodePlugin {
public:
    SeekErrorMuninNodePlugin();
  virtual ~SeekErrorMuninNodePlugin();

  virtual const char *GetName() { return "seekerrorrate"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
