@echo off

mkdir ..\build
pushd ..\build
cl -Zi ..\code\raster.cpp user32.lib gdi32.lib
popd



