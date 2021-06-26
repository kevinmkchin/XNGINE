@echo off
rem kevinmkchin's build file for OpenGLDemo
echo Build started: %time%

rem *****************************
rem Source Code to Build
rem You can do "somedirectory\*.cpp" to compile all source files in that directory
rem see https://stackoverflow.com/questions/33662375/how-can-you-compile-all-cpp-files-in-a-directory
set SOURCE=..\src\main_win64.cpp
rem *****************************

set PREPROCDEFINES=/DINTERNAL_BUILD=1 /DSLOW_BUILD=1
rem The /SUBSYSTEM option tells the OS how to run the .exe file.
rem The choice of subsystem affects the entry point function that the linker will choose.
set LINKERSUBSYSTEM=/SUBSYSTEM:CONSOLE

rem Additional Include Directories - preceded by /I
set ADDINCLUDES=/I"..\lib\GLEW\include" /I"..\lib\SDL\include" /I"..\lib\ASSIMP\include"
rem Additional Library/Dependency Directories - preceded by /LIBPATH:
set ADDLIBPATHS=/LIBPATH:"..\lib\GLEW\lib\Release\x64"	/LIBPATH:"..\lib\SDL\lib\x64" /LIBPATH:"..\lib\ASSIMP\lib"
rem Additional Dependencies; may be from above ADDLIBPATHS
set ADDDEPENDENCIES=Shell32.lib opengl32.lib glew32.lib SDL2.lib SDL2main.lib assimp-vc142-mt.lib

rem Binary Output Directory
set OUTPUTDIR="debug"
rem Output Executable Name
set OUTPUTEXE="opengldemo.exe"

rem Linker Options
set LINKEROPTIONS=%LINKERSUBSYSTEM% %ADDLIBPATHS% %ADDDEPENDENCIES%

if NOT exist build mkdir build
pushd build
if NOT exist %OUTPUTDIR% mkdir %OUTPUTDIR%
cl %PREPROCDEFINES% -Zi /EHsc /WX /W3 /std:c++14 /Fe%OUTPUTDIR%%OUTPUTEXE% %ADDINCLUDES% %SOURCE% /link %LINKEROPTIONS%
popd

echo Build finished: %time%


rem OTHER REMARKS
rem Shell32.lib required because https://discourse.libsdl.org/t/windows-build-fails-with-missing-symbol-imp-commandlinetoargvw/27256/3