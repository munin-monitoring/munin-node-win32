#pragma once

#include "MuninPluginManager.h"
#include "MuninNodeClient.h"
#include "JCSocket.h"

class MuninNodeServer : public JCThread {
public:
  virtual void Stop();

  virtual void *Entry();
private:
  MuninPluginManager m_PluginManager;
  JCSocket m_ServerSocket;
};
