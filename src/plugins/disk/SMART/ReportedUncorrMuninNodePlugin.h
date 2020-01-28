#pragma once
#include "../../../core/MuninNodePlugin.h"
#include "../SMARTMuninNodePlugin.h"

class ReportedUncorrMuninNodePlugin : public MuninNodePlugin {
public:
    ReportedUncorrMuninNodePlugin();
  virtual ~ReportedUncorrMuninNodePlugin();

  virtual const char *GetName() { return "reporteduncorrectableerrors"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsLoaded() { return true; };
};
