/* This file is part of munin-node-win32
 * Copyright (C) 2006-2008 Jory Stone (jcsston@jory.info)
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
#include "SpeedFanNodePlugin.h"

SpeedFanNodePlugin::SpeedFanNodePlugin()
{	
  m_Listener = new ListenerThread();
  m_Listener->JCThread_AddRef();
  m_Listener->Run();
}

SpeedFanNodePlugin::~SpeedFanNodePlugin()
{  
  m_Listener->Stop();
  m_Listener->JCThread_RemoveRef();
}

int SpeedFanNodePlugin::GetConfig(char *buffer, int len)
{
  int printCount;
  printCount = _snprintf(buffer, len, 
    "graph_title System temperature\n"
    "graph_args --base 1000 -l 0\n"
    "graph_vlabel temp in C\n"
    "graph_category sensors\n"
    "graph_info This graph shows the temperature in degrees Celsius.\n");
  len -= printCount;
  buffer += printCount;
  
  JCAutoLockCritSec lock(&m_Listener->m_BlocksCritSec);
  if (m_Listener->m_Blocks.size() > 1) {
    std::vector<xAPBlock *>::iterator it = m_Listener->m_Blocks.begin();
    // Skip first element (header)
    it++;
    for (; it != m_Listener->m_Blocks.end(); it++) {
      xAPBlockData *block = dynamic_cast<xAPBlockData *>(*it);
      if (block != NULL) {
        // Process the name to munin format
        std::string name = block->name;
        // Only include temp data
        if (name.find("temp") == 0) {
          // Replace all .'s with _'s
          char *nameStr = (char *)name.c_str();
          while (*(nameStr++) != NULL)
            if (*nameStr == '.')
              *nameStr = '_';

          printCount = _snprintf(buffer, len, "%s.label %s\n", name.c_str(), block->id.c_str());
          len -= printCount;
          buffer += printCount;
        }
      }
    }
    strncat(buffer, ".\n", len);
    return 0;
  }

  return -1;
}

int SpeedFanNodePlugin::GetValues(char *buffer, int len)
{
  int printCount;
  JCAutoLockCritSec lock(&m_Listener->m_BlocksCritSec);
  
  if (m_Listener->m_Blocks.size() > 1) {
    std::vector<xAPBlock *>::iterator it = m_Listener->m_Blocks.begin();
    // Skip first element (header)
    it++;
    for (; it != m_Listener->m_Blocks.end(); it++) {
      xAPBlockData *block = dynamic_cast<xAPBlockData *>(*it);
      if (block != NULL) {
        // Process the name to munin format
        std::string name = block->name;
        // Only include temp data
        if (name.find("temp") == 0) {
          // Replace all .'s with _'s
          char *nameStr = (char *)name.c_str();
          while (*(nameStr++) != NULL)
            if (*nameStr == '.')
              *nameStr = '_';

          printCount = _snprintf(buffer, len, 
            "%s.value %.2f\n"
            "%s.warning %.2f\n", 
            name.c_str(), block->current,
            name.c_str(), block->warning);
          len -= printCount;
          buffer += printCount;
        }
      }
    }
    strncat(buffer, ".\n", len);
    return 0;
  }
  return -1;
}


SpeedFanNodePlugin::xAPBlock::xAPBlock(const std::string &blockName) 
  : name(blockName)
{ 

};

SpeedFanNodePlugin::xAPBlock::~xAPBlock() 
{

};

bool SpeedFanNodePlugin::xAPBlock::ReadData(char *data) 
{
  // Split at the =
  char *split = strstr(data, "=");
  if (split != NULL) {
    // Extract the second part    
    *split = 0;
    split++;
    ProcessPair(data, split);
    return true;
  }    
  // Failed to extract =
  return false;
};


SpeedFanNodePlugin::xAPBlockHeader::xAPBlockHeader(const std::string &blockName) 
  : xAPBlock(blockName) 
{
  version = 0;
  hop = 0;
};

void SpeedFanNodePlugin::xAPBlockHeader::ProcessPair(char *name, char *value) 
{
  if (!strcmp(name, "v")) {
    version = atoi(value);
  
  } else if (!strcmp(name, "hop")) {
    hop = atoi(value);
  
  } else if (!strcmp(name, "uid")) {
    uid = value;

  } else if (!strcmp(name, "class")) {
    xAPClass = value;

  } else if (!strcmp(name, "source")) {
    source = value;    
  }
};

SpeedFanNodePlugin::xAPBlockData::xAPBlockData(const std::string &blockName) 
  : xAPBlock(blockName) 
{
  current = 0;
  want = 0;
  warning = 0;
};

void SpeedFanNodePlugin::xAPBlockData::ProcessPair(char *name, char *value) 
{
  if (!strcmp(name, "curr")) {
    current = atof(value);
  
  } else if (!strcmp(name, "want")) {
    want = atof(value);
  
  } else if (!strcmp(name, "warn")) {
    warning = atof(value);

  } else if (!strcmp(name, "id")) {
    id = value;
  }
};

SpeedFanNodePlugin::ListenerThread::ListenerThread()
{

};

SpeedFanNodePlugin::ListenerThread::~ListenerThread()
{

}

void SpeedFanNodePlugin::ListenerThread::Stop()
{
  JCThread::Stop();
  // Close the socket to abort the recvfrom call
  m_Socket.Close();
}

size_t SpeedFanNodePlugin::ListenerThread::ProcessBuffer(char *buffer)
{
  char *currentPos = buffer;
  char *endPos = NULL;
  
  bool inBlock = false;
  xAPBlock *currentBlock = NULL;
  std::vector<xAPBlock *> blocks;
  while (*currentPos)
  {
    endPos = strstr(currentPos, "\n");
    if (endPos == NULL) {
      // No more data after this
      endPos = currentPos + strlen(currentPos) - 1;
    }
    if (*currentPos == '{') {
      // Start of new block
      assert(!inBlock);
      inBlock = true;
    } else if (*currentPos == '}') {
      // End of section
      assert(inBlock);
      inBlock = false;
    } else {
      // Process the line of data
      *endPos = 0;
      if (!inBlock) {
        // Block name
        if (!strncmp(currentPos, "xAP-", 4)) {
          // Header block
          // We should only have one header block per data packet and it should be the first block
          assert(currentBlock == NULL);
          currentBlock = new xAPBlockHeader(currentPos);
        } else {
          // Data block
          if (currentBlock)
            blocks.push_back(currentBlock);
          currentBlock = new xAPBlockData(currentPos);
        }
      } else {
        // Block data
        assert(currentBlock != NULL);
        if (currentBlock != NULL)
          currentBlock->ReadData(currentPos);        
      }      
    }
    currentPos = endPos+1;
  }
  if (currentBlock)
    blocks.push_back(currentBlock);

  if (!blocks.empty()) {    
    xAPBlockHeader *headerBlock = dynamic_cast<xAPBlockHeader *>(blocks[0]);
    if (headerBlock != NULL) {
      // Check if this is a data packet we are interested in
      std::string uid = g_Config.GetValue("SpeedFanPlugin", "UID", "FF671100");
      if (headerBlock->uid == uid && headerBlock->xAPClass == "PC.status") {
        JCAutoLockCritSec lock(&m_BlocksCritSec);
        // Clear previous blocks
        for (std::vector<xAPBlock *>::iterator it = m_Blocks.begin(); it != m_Blocks.end(); it++)
          delete *it;
        // Copy over blocks
        m_Blocks = blocks;
        // Return block count
        return blocks.size();
      }
    }
  }

  // No blocks extracted that we are interested in
  return 0;
}


char *ReadInFile(const char *filename)
{
  FILE *exp_file = fopen(filename, "r");
  if (exp_file == NULL) {
    return NULL;
  }
  fseek(exp_file, 0, SEEK_END);
  long len = ftell(exp_file);  
  char *script = new char[len+1];
  fseek(exp_file, 0, SEEK_SET);
  len = fread(script, 1, len, exp_file);
  fclose(exp_file);
  if (len > 0) {
    script[len] = 0;
  } else {
    script[0] = 0;
  }  
  return script;
}

void ParserTest()
{
  //char *buffer = ReadInFile("C:\\Users\\Jory\\Documents\\Visual Studio Projects\\munin-node\\src\\plugins\\speedfan\\example.txt");
  //ProcessBuffer(buffer);
  //delete [] buffer;
}

bool SpeedFanNodePlugin::ListenerThread::OpenSocket()
{
  // Make sure the socket is closed
  m_Socket.Close();

  // Get a UDP socket
  if (!m_Socket.Create(AF_INET, JCSocket::UDP)) {
    printf("Error in SpeedFanNodePlugin at JCSocket::Create(): %ld\n", WSAGetLastError());
    return false;
  }

  // Setup the broadcast address
  std::string broadcastIP = g_Config.GetValue("SpeedFanPlugin", "BroadcastIP", "192.168.0.255");
  const int xAP_PORT = 3639;
  
  if (!m_Socket.Bind(xAP_PORT, broadcastIP.c_str())) {
    printf("Error in SpeedFanNodePlugin at JCSocket::Bind(): %ld\n", WSAGetLastError());
    return false;
  }

  return true;
}

void *SpeedFanNodePlugin::ListenerThread::Entry()
{
  if (!OpenSocket()) {
    // Failed to open socket
    return NULL;
  }

  // Use a 2048 byte buffer, Ethernet frame size is 1500
  const int bufferSize = 2048;
  char *buffer = new char[bufferSize];
  while (!TestDestroy()) {
    // Reset the socket after recieving a packet
    int ret = m_Socket.RecvFrom(buffer, bufferSize);
    if (ret > 0) {
      buffer[ret] = 0;
      ProcessBuffer(buffer);
    } else {
      // Reopen socket
      if (TestDestroy() || !OpenSocket()) {
        // We need to exit or we failed to re-open the socket
        break;
      }
    }
    Sleep(0);
  }

  delete [] buffer;

  return NULL;
}