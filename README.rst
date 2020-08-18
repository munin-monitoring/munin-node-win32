============================
Munin Node for Windows
============================

**I will try my best to keep maintaining this repo,**
**but I am also busy writing a new Munin Node For Windows in C# that would work better in newer versions of windows**

Introduction
------------
Munin Node for Windows AKA munin-node-win32, is a Windows client for the Munin monitoring system.

It is written in C++ with most plugins built into the executable. 
This is different from the standard munin-node client, which only uses external plugins written as shell and Perl scripts.

Command Line Usage
------------------

  * -install Install as a system service.
  * -uninstall Removes the installed service.
  * -quiet Close the console window, running in the background.
  * -run Run as a normal program, rather than a service.

Configuration File
------------------

The configuration file munin-node.ini uses the standard INI file format.

The main section is the Plugins section which defines which plugins are enabled and which are disabled. Use 1 to enable and 0 to disable. Most plugins handle failing to load gracefully, however some may have a bug (if so please report :).

You can disable plugins you don't want, to save on memory and cpu usage.

Plugins
-------

  * CPU (cpu):

    * Reports the current user cpu usage

  * Disk (df):

    * Reports disk drive space usage

    * Configuration in DiskPlugin section.

  * HD (hdd):

    * Reports HardDrive temperature

  * Spin (spinup)
    
    * Reports HardDrive spin-up time

  * Online (onlinehours)

    * Reports the total HardDrive Online Hours

  * Readerror (readerrorrate)

    * Reports the HardDrive Read Error Rate

  * Startstop (startstopcyclehdd)

    * Reports the total HardDrive Start Stop Cycles

  * Reallocsector (reallocsectorcount)

    * Reports the HardDrive reallocated sector count

  * Seekerror (seekerrorrate)

    * Reports the HardDrive Seek-Error rate

  * Spinretry (spinretrycount)

    * Reports the HardDrive Spin Retry count

  * Reporteduncorr (reporteduncorrectableerrors)

    * Reports the HardDrive Reported Uncorrectable errors

  * Poweroffretract (poweroffretract)

    * Reports the amount of Unsafe Shutdowns in the HardDrive's lifetime

  * Memory (memory):

    * Reports memory usage

  * Process (processes):

    * Reports process and thread count

  * Network (network):

    * Reports network bytes send and received

  * MBM (mbm, mbm_volt, mbm_fan, mbm_cpu):

    * Reports sensor data from Motherboard Monitor

  * SpeedFan (speedfan):

    * Reports sensor data from SpeedFan xAP broadcasts

    * Configuration in SpeedFanPlugin section. You have to change the BroadcastIP and UID settings to match SpeedFan (Configuration->xAP)

  * Performance Counter:

    * Reports performance counter values, very extenable to monitor any counter

    * An instance of this plugin is created for every section starting with PerfCounterPlugin in the Configuration. For example there is a PerfCounterPlugin_uptime section in the stock configuration file. This defines a plugin with a name of uptime.

    * An example section is listed below ::

        ; The Object and Counter settings are used to access the Performance Counter
        ; For uptime this would result in \System\System Up Time
        Object=System
        Counter=System Up Time
        ; The Graph settings are reported to munin
        GraphTitle=Uptime
        GraphCategory=system
        GraphDraw=AREA
        GraphArgs=--base 1000 -l 0
        ; The DropTotal setting will drop the last instance from the counter list, 
        ; which is often _Total
        ; Has no effect on single instance counters (Uptime)
        DropTotal=0
        ; The CounterFormat setting controls what format the counter value is read in as 
        ; a double, int, or large (int64).
        ; The plugin always outputs doubles, so this shouldn't have that much effect
        CounterFormat=large
        ; The CounterMultiply setting sets a value the counter value is multiplied by, 
        ; use it to adjust the scale
        ; 1.1574074074074073e-005 is the result of(1 / 86400.0), 
        ; the uptime counter reports seconds and we want to report days.
        ; So we want to divide the counter value by the number of seconds in a day, 86400.
        CounterMultiply=1.1574074074074073e-005

  * External Plugin:

    * A plugin that supports external plugins in the style of munin-node.

    * Configuration in [ExternalPlugin] section. Just add an entry with the path to the program to run, It doesn't matter what the name of the name=value pair is.

    * The output of the external program should be similar to the following,

    * Note: add quotes (") around the value if it has spaces! ::

        >disk_free.py name
        drive_free
        
        >disk_free.py
        drive_free_c.value 40.3635149113
        .
        
        >disk_free.py config
        graph_title Filesystem free (in %)
        graph_category disk
        graph_info This graph shows the amount of free space on each disk.
        graph_args --upper-limit 100 -l 0
        graph_vlabel %
        drive_free_c.label C:
        .

Version History
---------------
See ChangeLog


Building a release
------------------

Using Visual Studio 2019 with Build Tool v142.

Open solution file (.sln) with Visual Studio.

On the top menu, find Build, and the select 'Build Solution'.


Creating an Installer
-----------------------

Using Inno Setup

Open '/Installer Files/Munin-node.iss'

Compile the installer



