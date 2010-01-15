#pragma once
#include "../../core/MuninNodePlugin.h"

class ProcessesMuninNodePlugin : public MuninNodePlugin
{
public:
  ProcessesMuninNodePlugin();
  virtual ~ProcessesMuninNodePlugin();

  virtual const char *GetName() { return "processes"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
