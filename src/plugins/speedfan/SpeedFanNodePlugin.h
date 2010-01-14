#pragma once
#include "../../core/MuninNodePlugin.h"
#include "../../core/JCSocket.h"

class SpeedFanNodePlugin : public MuninNodePlugin
{
public:
  SpeedFanNodePlugin();
  virtual ~SpeedFanNodePlugin();

  virtual bool IsLoaded() { return m_Listener != NULL; };  
  virtual const char *GetName() { return "speedfan"; };
  virtual int GetConfig(char *buffer, int len);
  virtual int GetValues(char *buffer, int len);
  virtual bool IsThreadSafe() { return true; };

private:
  class xAPBlock
  {
  public:
    xAPBlock(const std::string &blockName);
    virtual ~xAPBlock();
    virtual bool ReadData(char *data);
    virtual void ProcessPair(char *name, char *value) = 0;

    std::string name;
  };

  class xAPBlockHeader : public xAPBlock
  {
  public:
    xAPBlockHeader(const std::string &blockName);
    virtual void ProcessPair(char *name, char *value);

    int version;
    int hop;
    std::string uid;
    std::string xAPClass;
    std::string source;
  };

  class xAPBlockData : public xAPBlock
  {
  public:
    xAPBlockData(const std::string &blockName);
    virtual void ProcessPair(char *name, char *value);

    std::string id;
    double current;
    double want;
    double warning;
  };
  
  class ListenerThread : public JCThread {
  public:		
    ListenerThread();
    virtual ~ListenerThread();

    virtual void Stop();

    size_t ProcessBuffer(char *data);
    bool OpenSocket();
    virtual void *Entry();

    JCSocket m_Socket;
    JCCritSec m_BlocksCritSec;
    std::vector<xAPBlock *> m_Blocks;
  };
  ListenerThread *m_Listener;
};

