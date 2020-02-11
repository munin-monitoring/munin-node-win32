#pragma once
#include "../../../core/MuninNodePlugin.h"
#include "../SMARTMuninNodePlugin.h"

class SpinUpMuninNodePlugin : public MuninNodePlugin {
public:
    SpinUpMuninNodePlugin();
    virtual ~SpinUpMuninNodePlugin();

    virtual const char* GetName() { return "spinup"; };
    virtual int GetConfig(char* buffer, int len);
    virtual int GetValues(char* buffer, int len);
    virtual bool IsLoaded() { return true; };
};