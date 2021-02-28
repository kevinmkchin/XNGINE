@echo off
rem kevinmkchin's build file for OpenGLDemo
rem You must use the Visual C++ Command Line compiler MVSC - cl.exe.
rem You don't need to download VS; you only need to download "Build Tools for Visual Studio".
rem If you already have Visual Studio, you probably have the MSVC compiler.
rem You must run this batch script from a command line that can access the build tools. You have a few options:
rem 	1. Find and add your cl.exe to your Environment Paths.
rem 	2. Use the "Developer Command Prompt for VS20**". This comes with your VS installation and is configured to use all the VS tools.
rem For either option, make sure you are using the correct cl.exe version! x86 or x64
rem 	3. The call line below configures cmd.exe to access all of the Microsoft C++ Toolset. You can uncomment
rem        it and rely on it, but make sure that the path points to vcvarsall.bat on your computer.
rem call "C://Program Files (x86)//Microsoft Visual Studio//2019//Community//VC//Auxiliary//Build//vcvarsall.bat" x64

rem ****************************************************************************************************
rem Source Code to Build - Specify all the source files here - use ^ to split into next line if too long
set SOURCE=..\src\main.cpp ..\src\mesh.cpp ..\src\shader.cpp ..\src\camera.cpp ..\src\texture.cpp

rem ****************************************************************************************************

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

if NOT exist ..\bin mkdir ..\bin
pushd ..\bin
if NOT exist %OUTPUTDIR% mkdir %OUTPUTDIR%
cl -Zi /EHsc /W3 /std:c++14 /Fe%OUTPUTDIR%%OUTPUTEXE% %ADDINCLUDES% %SOURCE% /link %LINKEROPTIONS%
popd

rem pause


rem OTHER REMARKS
rem Shell32.lib required because https://discourse.libsdl.org/t/windows-build-fails-with-missing-symbol-imp-commandlinetoargvw/27256/3