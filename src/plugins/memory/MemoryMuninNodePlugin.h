#pragma once
#include "../../core/MuninNodePlugin.h"

class MemoryMuninNodePlugin : public MuninNodePlugin
{
public:
  MemoryMuninNodePlugin();
  virtual ~MemoryMuninNodePlugin();

  virtual const char *GetName() { return "memory"; }; 
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
