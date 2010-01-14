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
#include "NetworkMuninNodePlugin.h"

NetworkMuninNodePlugin::NetworkMuninNodePlugin()
{
  m_LastUdpPacketInCount = 0;
  m_LastUdpPacketOutCount = 0;
  m_LastTcpPacketInCount = 0;
  m_LastTcpPacketOutCount = 0;

  // Get the initial packet counts
  char buffer[65] = {0};
  GetValues(buffer, 64);
}

NetworkMuninNodePlugin::~NetworkMuninNodePlugin()
{
}

int NetworkMuninNodePlugin::GetConfig(char *buffer, int len) {
  int ret = 0;
  strncpy(buffer, "graph_order down up\n"
    "graph_title network traffic\n"
    "graph_args --base 1000\n"
    "graph_vlabel packets in (-) / out (+) per ${graph_period}\n"
    "graph_category network\n"
    "graph_info This graph shows the traffic of the network interfaces. Please note that the traffic is shown in packets per second, not bytes.\n"
    "down.label pps\n"
    "down.type COUNTER\n"
    "down.graph no\n"
    "up.label pps\n"
    "up.type COUNTER\n"
    "up.negative down\n"
    "up.info Traffic of the interfaces. Maximum speed is 54000000 packets per second.\n"
    "up.max 54000000\n"
    "down.max 54000000\n"
    ".\n", len);
  return 0;
};

int NetworkMuninNodePlugin::GetValues(char *buffer, int len) { 
  int ret = 0;
  MIB_TCPSTATS tcpStats;
  MIB_UDPSTATS udpStats;

  ret = GetTcpStatistics(&tcpStats);
  ret = GetUdpStatistics(&udpStats);
  _snprintf(buffer, len, "down.value %i\nup.value %i\n.\n", 
    tcpStats.dwInSegs + udpStats.dwInDatagrams,
    tcpStats.dwOutSegs + udpStats.dwOutDatagrams);

  return 0;
};
