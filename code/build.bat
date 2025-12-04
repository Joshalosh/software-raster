@echo off

mkdir ..\build
pushd ..\build
cl -DGAME_SLOW=1 -DGAME_INTERNAL=1 -FC -Zi ..\code\win32_platform.cpp user32.lib gdi32.lib winmm.lib
popd



