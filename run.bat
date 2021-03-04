@echo off

rem run with argument "vs" if you want to start visual studio to debug

IF "%1"=="vs" (
	devenv build\debug\opengldemo.exe
) ELSE (
	pushd build\debug\
	opengldemo.exe
	popd
)