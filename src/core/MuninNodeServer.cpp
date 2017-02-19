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
#include "MuninNodeServer.h"
#include "MuninNodeSettings.h"
#include "Service.h"

void MuninNodeServer::Stop()
{
  JCThread::Stop();
  // Close the server socket to force the accept call to abort
  m_ServerSocket.Close();
}

void *MuninNodeServer::Entry()
{	
  int portNumber = g_Config.GetValueI("MuninNode", "PortNumber", 4949);
  bool logConnections = g_Config.GetValueB("MuninNode", "LogConnections", true);
  std::string masterAddress = g_Config.GetValue("MuninNode", "MasterAddress", "*");
  //std::string bindAddress = g_Config.GetValue("MuninNode", "BindAddress", "");

  //the socket function creates our SOCKET  
  if (!m_ServerSocket.Create()) {
    return 0;
  }

  //bind links the socket we just created with the sockaddr_in 
  //structure. Basically it connects the socket with 
  //the local address and a specified port.
  //If it returns non-zero quit, as this indicates error
  if (!m_ServerSocket.Bind(portNumber)) {
    return 0;
  }

  //listen instructs the socket to listen for incoming 
  //connections from clients. The second arg is the backlog
  if (!m_ServerSocket.Listen(10)) {
    return 0;
  }

  _Module.LogEvent("Server Thread Started");

  while (!TestDestroy()) {
    // Wait for new client connection
    JCSocket *client = new JCSocket();
    if (m_ServerSocket.Accept(client)) {
      // TODO: Add ip address matching, http://stackoverflow.com/questions/594112/matching-an-ip-to-a-cidr-mask-in-php5
      const char *ipAddress = inet_ntoa(client->m_Address.sin_addr);
      if ( !ipAddress )
        ipAddress = "unknown address";
        if (masterAddress == "*" || ipAddress == masterAddress) {
          if (logConnections) {
            _Module.LogEvent("Connection from %s", ipAddress);
          }
          // Start child thread to process client socket
          MuninNodeClient *clientThread = new MuninNodeClient(client, this, &m_PluginManager);
          clientThread->Run();
        } else {
          _Module.LogError("Rejecting connection from %s", ipAddress);
          delete client;
        }
    } else {
      delete client;
      break;
    }
  }

  m_ServerSocket.Shutdown(SD_SEND);

  return 0;
}
