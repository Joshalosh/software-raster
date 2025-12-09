@echo off

set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4996 -DGAME_SLOW=1 -DGAME_INTERNAL=1 -FC -Z7
set CommonLinkerFlags= user32.lib gdi32.lib winmm.lib
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl %CommonCompilerFlags% ..\code\render.cpp /DLL
cl %CommonCompilerFlags% ..\code\win32_platform.cpp /link %CommonLinkerFlags%
popd



