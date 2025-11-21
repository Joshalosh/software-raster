@echo off

mkdir ..\build
pushd ..\build
cl -Zi ..\code\raster.cpp user32.lib
popd



