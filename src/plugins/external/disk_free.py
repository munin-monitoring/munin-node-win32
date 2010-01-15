import ctypes
import sys
class DriveSpaceFree:
	def name(self):
		return "drive_free"
    
	def config(self):
		return 	"""graph_title Filesystem free (in %)
graph_category disk
graph_info This graph shows the amount of free space on each disk.
graph_args --upper-limit 100 -l 0
graph_vlabel %
drive_free_c.label C:
.
"""

	def values(self):
		try:
			drive = unicode("C:")
			freeuser = ctypes.c_int64()
			total = ctypes.c_int64()
			free = ctypes.c_int64()
			ctypes.windll.kernel32.GetDiskFreeSpaceExW(drive, ctypes.byref(freeuser), ctypes.byref(total), ctypes.byref(free))
			return "drive_free_c.value " + str(100.0 * free.value / total.value) + """
.
"""
		except Error, e:
			return e, """
.
"""
		
munin_node_plugin = DriveSpaceFree()

if __name__ == "__main__":
    if (len(sys.argv) > 1):
        if (sys.argv[1] == "name"):
            print munin_node_plugin.name(),
        elif (sys.argv[1] == "config"):
            print munin_node_plugin.config(),
    else:
        print munin_node_plugin.values(),