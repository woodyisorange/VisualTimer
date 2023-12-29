@echo off

set OutputDir=..\Binaries
set SourceDir=..\Source

if not exist %OutputDir% mkdir %OutputDir%
pushd %OutputDir%
cl /std:clatest /Zi %SourceDir%\VisualTimer.c user32.lib
popd
