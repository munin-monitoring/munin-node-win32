#pragma once
#include "../../core/MuninNodePlugin.h"

class NetworkMuninNodePlugin : public MuninNodePlugin
{
public:
  NetworkMuninNodePlugin();
  virtual ~NetworkMuninNodePlugin();

  virtual const char *GetName() { return "network"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
