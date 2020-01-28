#pragma once
#include "../../../core/MuninNodePlugin.h"
#include "../SMARTMuninNodePlugin.h"

class ReadErrorMuninNodePlugin : public MuninNodePlugin {
public:
  ReadErrorMuninNodePlugin();
  virtual ~ReadErrorMuninNodePlugin();

  virtual const char *GetName() { return "readerrorrate"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
