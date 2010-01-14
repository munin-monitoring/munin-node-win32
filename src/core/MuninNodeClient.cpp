/* This file is part of munin-node-win32
 * Copyright (C) 2006-2007 Jory Stone (jcsston@jory.info)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "StdAfx.h"
#include "MuninNodeClient.h"
#include "Service.h"
#include "../extra/verinfo.h"

MuninNodeClient::MuninNodeClient(JCSocket *client, JCThread *server, MuninPluginManager *pluginManager) 
  : m_Client(client)
  , m_Server(server)
  , m_PluginManager(pluginManager)
{
  m_Server->JCThread_AddRef();
}

MuninNodeClient::~MuninNodeClient()
{
  // Close the client socket
  if (m_Client != NULL) {
    delete m_Client;
  }
  m_Server->JCThread_RemoveRef();
}

int MuninNodeClient::SendLine(const char *line) {
  int ret = 0;
  int len = (int)strlen(line);
  int sent = 0;

  while (sent != len) {
    ret = m_Client->Send(line+sent, len-sent);
    if (ret == SOCKET_ERROR) {
      _Module.LogEvent("Socket Error Sending: %i", WSAGetLastError());
      return -1;
    } else if (ret == 0) {
      _Module.LogEvent("Socket Error Send: No data sent");
      return -1;
    }
    sent += ret;
  }

  return sent;
}

int MuninNodeClient::RecvLine(char *line, int len) {
  int ret = 0;
  int received = 0;

  // Make sure we reset the buffer
  memset(line, NULL, len);

  // Read until we get an end-of-line marker
  while (strstr(line, "\n") == NULL) {
    ret = m_Client->Recv(line+received, len-received);
    if (ret == SOCKET_ERROR) {
      _Module.LogEvent("Socket Error Recv: %i", WSAGetLastError());
      return -1;
    } else if (ret == 0) {
      //_Module.LogEvent("Socket Error Recv: No data recieved");
      return -1;
    }
    received += ret;
    // Wait for more data
    Sleep(0);
  }

  return received;
}

void *MuninNodeClient::Entry()
{	
  int ret = 0;
  static const int BUFFER_SIZE = 8096;
  char buffer[BUFFER_SIZE] = {0};
  char hostname[64] = {0};
  int len = 0;
  
  ret = gethostname(hostname, 64);
  if (ret) {
    _Module.LogEvent("Failed to get hostname!");
  }
  ret = _snprintf(buffer, BUFFER_SIZE, "# munin node at %s\n", hostname);

  // we simply send this string to the client
  ret = SendLine(buffer);

  while (!TestDestroy()) {
    // Now wait for a reply
    ret = RecvLine(buffer, BUFFER_SIZE);
    if (ret == -1) {
      // Recieve error, connection reset?
      break;
    }
    // Remove newlines
    char *buffer2 = strstr(buffer, "\n");
    if (buffer2 != NULL) {
      *buffer2 = NULL;
    }
    buffer2 = strstr(buffer, "\r");
    if (buffer2 != NULL) {
      *buffer2 = NULL;
    }
    if (strstr(buffer, "quit") == buffer) {
      break;

    } else if (strstr(buffer, "version") == buffer) {   
      // Read in Version Infomation
      CFileVersionInfo ver;
      ver.Open(GetModuleHandle(NULL));
      ret = _snprintf(buffer, BUFFER_SIZE, "munin node on %s version: Munin Node for Windows %i.%i.%i\n", hostname, ver.GetFileVersionMajor(), ver.GetFileVersionMinor(), ver.GetFileVersionQFE());
      ret = SendLine(buffer);        

    } else if (strstr(buffer, "nodes") == buffer) {        
      // This version only supports one node
      ret = _snprintf(buffer, BUFFER_SIZE, "%s\n.\n", hostname);
      ret = SendLine(buffer);

    } else if (strstr(buffer, "list") == buffer) {     
      memset(buffer, 0, BUFFER_SIZE);
      m_PluginManager->FillPluginList(buffer, BUFFER_SIZE);
      ret = SendLine(buffer);

    } else if (strstr(buffer, "config") == buffer) {      
      MuninNodePlugin *plugin = m_PluginManager->LookupPlugin(buffer);
      if (plugin != NULL) {
        memset(buffer, 0, BUFFER_SIZE);
        ret = plugin->GetConfig(buffer, BUFFER_SIZE);
        if (ret < 0) {
          ret = SendLine("# Unknown Error\n.\n");
        } else {
          ret = SendLine(buffer);
        }
      } else {
        ret = SendLine("# Unknown service\n.\n");
      }

    } else if (strstr(buffer, "fetch") == buffer) {    
      MuninNodePlugin *plugin = m_PluginManager->LookupPlugin(buffer);
      if (plugin != NULL) {
        memset(buffer, 0, BUFFER_SIZE);
        ret = plugin->GetValues(buffer, BUFFER_SIZE);
        if (ret < 0) {
          ret = SendLine("# Unknown Error\n.\n");
        } else {
          ret = SendLine(buffer);
        }
      } else {
        ret = SendLine("# Unknown service\n.\n");
      }

    } else {
      ret = SendLine("# Unknown command. Try list, nodes, config, fetch, version or quit\n");
    }
    
  }
  
  return 0;
}