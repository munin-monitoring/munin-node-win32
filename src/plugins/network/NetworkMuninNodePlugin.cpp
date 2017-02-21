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
#include <iphlpapi.h>

NetworkMuninNodePlugin::NetworkMuninNodePlugin()
{
}

NetworkMuninNodePlugin::~NetworkMuninNodePlugin()
{
}

static int doiftable(int mode, string &myout) {
  DWORD iftsize = sizeof(MIB_IFTABLE) * 1;
  MIB_IFTABLE * mibiftable = (MIB_IFTABLE *)malloc(iftsize);
  if (mibiftable == NULL) {
    return 1;
  }
  if (GetIfTable(mibiftable, &iftsize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
	// this failed, but set iftsize to the required size. So lets allocate that.
    free(mibiftable);
    mibiftable = (MIB_IFTABLE *)malloc(iftsize);
    if (mibiftable == NULL) {
      return 1;
    }
  }
  if (GetIfTable(mibiftable, &iftsize, FALSE) != NO_ERROR) {
	return 1;
  }
  for (DWORD i = 0; i < mibiftable->dwNumEntries; i++) {
    MIB_IFROW * mibifrow = (MIB_IFROW *)&mibiftable->table[i];
    if (mibifrow->dwType != IF_TYPE_ETHERNET_CSMACD) { // not Ethernet
      continue;
    }
	if (mibifrow->dwOperStatus != IF_OPER_STATUS_OPERATIONAL) { //not operational
      continue;
	}
	if (mibifrow->dwAdminStatus != 1) { // administratively disabled
      continue;
    }
	if (mibifrow->dwSpeed == 0x40000000) { // nonsense ifSpeed
	  continue;
    }
    char ifid[64];
	sprintf(ifid, "%d", mibifrow->dwIndex);
    myout += "multigraph if_eth"; myout += ifid; myout += "\n";
    if (mode == 0) {
      // print config
      myout += "graph_order down up\n";
      myout += "graph_title interface eth"; myout += ifid; myout += " traffic\n";
      myout += "graph_args --base 1000\n";
      myout += "graph_vlabel bits in (-) / out (+) per ${graph_period}\n";
      myout += "graph_category network\n";
      myout += "graph_info This graph shows the traffic of the windows interface with index ";
      myout += ifid;
      myout += ". Its description is ";
      char cleandescr[101];
      for (DWORD j = 0; j < sizeof(cleandescr); j++) {
        if ((mibifrow->bDescr[j] < 32) && (mibifrow->bDescr[j] > 126)) {
          mibifrow->bDescr[j] = '?';
        }
        if (j >= mibifrow->dwDescrLen) {
          cleandescr[j] = 0;
          break;
        }
        cleandescr[j] = mibifrow->bDescr[j];
      }
      cleandescr[100] = 0;
      myout += cleandescr;
      myout += ".\n";
      myout += "down.label received\n";
      myout += "down.type COUNTER\n";
      myout += "down.graph no\n";
      myout += "down.cdef down,8,*\n";
      myout += "down.min 0\n";
      myout += "up.label bps\n";
      myout += "up.type COUNTER\n";
      myout += "up.negative down\n";
      myout += "up.cdef up,8,*\n";
      myout += "up.min 0\n";
      myout += "up.info Traffic of the interface with index "; myout += ifid; myout += ".\n";
    } else {
      // Print values
      char valout[64];
      sprintf(valout, "%lu", mibifrow->dwInOctets);
      myout += "down.value "; myout += valout; myout += "\n";
      sprintf(valout, "%lu", mibifrow->dwOutOctets);
      myout += "up.value "; myout += valout; myout += "\n";
    }
    myout += "\n";
  }
  return 0;
}

int NetworkMuninNodePlugin::GetConfig(char *buffer, int len) {
  string myout("");
  myout += "multigraph network\n";
  myout += "graph_order down up\n";
  myout += "graph_title network traffic\n";
  myout += "graph_args --base 1000\n";
  myout += "graph_vlabel packets in (-) / out (+) per ${graph_period}\n";
  myout += "graph_category network\n";
  myout += "graph_info This graph shows the traffic of the network interfaces. Please note that the traffic is shown in packets per second, not bytes.\n";
  myout += "down.label pps\n";
  myout += "down.type COUNTER\n";
  myout += "down.graph no\n";
  myout += "up.label pps\n";
  myout += "up.type COUNTER\n";
  myout += "up.negative down\n";
  myout += "up.info Traffic of the interfaces. Maximum speed is 54000000 packets per second.\n";
  myout += "up.max 54000000\n";
  myout += "down.max 54000000\n";
  myout += "\n";
  doiftable(0, myout);
  myout += ".\n";
  strncpy(buffer, myout.c_str(), len);
  return 0;
};

int NetworkMuninNodePlugin::GetValues(char *buffer, int len) { 
  MIB_TCPSTATS tcpStats;
  MIB_UDPSTATS udpStats;

  string myout("");
  myout += "multigraph network\n";
  GetTcpStatistics(&tcpStats);
  GetUdpStatistics(&udpStats);
  char packetcntrstr[200];
  sprintf(packetcntrstr,
    "down.value %lu\nup.value %lu\n\n", 
    tcpStats.dwInSegs + udpStats.dwInDatagrams,
    tcpStats.dwOutSegs + udpStats.dwOutDatagrams);
  myout += packetcntrstr;
  doiftable(1, myout);
  myout += ".\n";
  strncpy(buffer, myout.c_str(), len);
  return 0;
};