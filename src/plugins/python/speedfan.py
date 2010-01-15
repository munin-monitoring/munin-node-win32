#!/usr/bin/env python
import SocketServer

class SpeedFanRequestHandler(SocketServer.DatagramRequestHandler):
    def parse(self, data):
        for line in data.splitlines():
            print line
            
    def handle(self):
        self.parse(self.request[0])
        
##  // Listen on port 3639 UDP
##  /** Sample xAP SpeedFan output
##  xAP-header
##  {
##  v=12
##  hop=1
##  uid=FF671100
##  class=PC.status
##  source=Almico.SpeedFan.JCSSTON
##  }
##  temp.1
##  {
##  id=Case
##  curr=38.0
##  want=40
##  warn=50
##  }
##  temp.2
##  {
##  id=CPU
##  curr=39.0
##  want=40
##  warn=50
##  }
##  temp.4
##  {
##  id=Core 0
##  curr=48.0
##  want=40
##  warn=50
##  }
##  temp.5
##  {
##  id=Core 1
##  curr=52.0
##  want=40
##  warn=50
##  }
##  fan.1
##  {
##  id=Fan1
##  curr=2689
##  }
##  fan.2
##  {
##  id=Fan2
##  curr=1412
##  }
##  fan.3
##  {
##  id=Fan3
##  curr=0
##  }
##  fan.4
##  {
##  id=Fan4
##  curr=0
##  }

class SpeedFanPlugin(SocketServer.UDPServer):
    def __init__(self):
        SocketServer.UDPServer.__init__(self, ("192.168.0.255", 3639), SpeedFanRequestHandler)

	def close(self):
		return
		
    def name(self):
        return "speedfan"
    
    def config(self):
        return ".\n"

    def values(self):
        return ".\n"

# This is the instance the munin-node-win32 will use,
# It will call munin_node_plugin.name(), munin_node_plugin.config(), and
# munin_node_plugin.values()
munin_node_plugin = SpeedFanPlugin()

#server = SocketServer.UDPServer()
munin_node_plugin.handle_request()

munin_node_plugin.server_close()
