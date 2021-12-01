@echo off

rem run with argument "vs" if you want to start visual studio to debug

IF "%1"=="vs" (
    devenv cmake-build-debug\xngine.exe
) ELSE (
    pushd cmake-build-debug
    START /D ..\data xngine.exe
    popd
)