@echo off

set CommonCompilerFlags= -MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4996 -DGAME_SLOW=1 -DGAME_INTERNAL=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

del *.pdb > NUL 2> NUL

cl %CommonCompilerFlags% ..\code\render.cpp -Fmgame.map -LD /link -incremental:no -PDB:game_%random%.pdb -EXPORT:GameUpdateAndRender
cl %CommonCompilerFlags% ..\code\win32_platform.cpp -Fmwin_32.map /link %CommonLinkerFlags%
popd



