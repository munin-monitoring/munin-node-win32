#pragma once

#include "MuninNodePlugin.h"
#include "MuninPluginManager.h"
#include "JCSocket.h"

class MuninNodeClient : public JCThread {
  JCThread *m_Server;
  MuninPluginManager *m_PluginManager;
  JCSocket *m_Client;
public:
  MuninNodeClient(JCSocket *client, JCThread *server, MuninPluginManager *pluginManager);
  virtual ~MuninNodeClient();

  int SendLine(const char *line);
  int RecvLine(char *line, int len);

  virtual void *Entry();
};
