== Runing the viewer
use:
RayBuild2 path/to/map.map
can also use it with relative paths like 
RayBuild2 ../mymap.map 
you must have palette.dat and tiles*.art unpacked in folder with map

== Running the duke
Well, duke is absolutely desynced, but you can still run it:
you'll need duke3d.grp in same folder as DukeGame.exe
then it may fail couple of times, until it extracts cons and etc.
in the end you can run it same way as RayBuild2, providing path to any map anywhere.
Essentially it runs duke code on the loaded map, and uses GRP file to get cons for gameplay.

Keys:
Alt + Esc = exit.
enter colorpicker once to disable cursor on start.


== Ken's Build 2
also bundled in this package, for consistency. it is DX software render implementation by Ken Silverman.