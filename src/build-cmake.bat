@echo off
echo Build started: %time%
cmake -S .././ -B ../build
cmake --build ../build
echo Build finished: %time%