@echo off
rem kevinmkchin's build file for OpenGLDemo
echo Build started: %time%


rem *****************************
rem Source Code to Build
set SOURCE=..\src\main.cpp
rem *****************************

rem Additional Include Directories - preceded by /I
set ADDINCLUDES=/I"..\libs\GLEW\include" /I"..\libs\SDL\include" /I"..\libs\GLM"
rem Additional Library/Dependency Directories - preceded by /LIBPATH:
set ADDLIBPATHS=/LIBPATH:"..\libs\GLEW\lib\Release\x64"	/LIBPATH:"..\libs\SDL\lib\x64"
rem Additional Dependencies; may be from above ADDLIBPATHS
set ADDDEPENDENCIES=Shell32.lib opengl32.lib glew32.lib SDL2.lib SDL2main.lib

rem Binary Output Directory
set OUTPUTDIR="debug"
rem Output Executable Name
set OUTPUTEXE="opengldemo.exe"

rem Linker Options
set LINKEROPTIONS=/SUBSYSTEM:CONSOLE %ADDLIBPATHS% %ADDDEPENDENCIES%

if NOT exist build mkdir build
pushd build
if NOT exist %OUTPUTDIR% mkdir %OUTPUTDIR%
cl -Zi /EHsc /W3 /std:c++14 /Fe%OUTPUTDIR%%OUTPUTEXE% %ADDINCLUDES% %SOURCE% /link %LINKEROPTIONS%
popd

echo Build finished: %time%


rem OTHER REMARKS
rem Shell32.lib required because https://discourse.libsdl.org/t/windows-build-fails-with-missing-symbol-imp-commandlinetoargvw/27256/3