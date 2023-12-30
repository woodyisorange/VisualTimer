@echo off

set OutputDir=..\Binaries
set SourceDir=..\Source
set CommonOptions=/std:clatest /W4 /wd4100
set DebugOptions=/Zi
set ReleaseOptions=/O2
set Options=%CommonOptions% %ReleaseOptions%

if not exist %OutputDir% mkdir %OutputDir%
pushd %OutputDir%
cl %Options% %SourceDir%\VisualTimer.c user32.lib gdi32.lib
popd
