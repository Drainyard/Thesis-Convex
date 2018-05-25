@echo off

set WERROR=
set DEBUG=-DDEBUG
set GLM=
set PRP=
set WIGNORED=-wd4668 -wd4820 -wd5045 -wd4571 -wd4201 -wd4996 -wd5039 -wd4505
 
WHERE cl
IF %ERRORLEVEL% NEQ 0 call %VCVARSALL% x64

echo %cd%
set CommonCompilerFlags=/MD -nologo -Od -Oi -W4 -Gm- -EHsc -FC -Z7 %PRP% %WIGNORED% %DEBUG%  /I..\libs /I..\libs\glad\include 
set CommonLinkerFlags= Comdlg32.lib Ole32.lib kernel32.lib user32.lib gdi32.lib winmm.lib Shlwapi.lib opengl32.lib shell32.lib ..\libs\glfw\lib-vc2015\glfw3.lib ..\libs\glad\glad.obj 
set ExtraLinkerFlags=/NODEFAULTLIB:"LIBCMT" -incremental:no -opt:ref /ignore:4099


IF NOT EXIST build mkdir build
pushd build

REM 64-bit build
del *.pdb > NUL 2> NUL

echo Compilation started on: %time%
cl %CommonCompilerFlags% ..\src\main.cpp -Femain  /link %ExtraLinkerFlags% %CommonLinkerFlags%
echo Compilation finished on: %time%
popd