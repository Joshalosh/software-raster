@echo off

mkdir ..\build
pushd ..\build
cl -MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4996 -DGAME_SLOW=1 -DGAME_INTERNAL=1 -FC -Z7 ..\code\win32_platform.cpp user32.lib gdi32.lib winmm.lib
popd



