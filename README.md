# opengl-3d-renderer
3D graphics render engine using OpenGL in C++

[GLEW](http://glew.sourceforge.net/) for interfacing with OpenGL.
[SDL](https://www.libsdl.org/) for the Win32API and OpenGL Context. 

## How to Build
I'm not using Visual Studio or a Make system. Instead, I'm using a batch file in /src called build.bat. This batch script builds the project using Visual Studio's MSVC command line compiler: cl.exe. Therefore, the only thing you need to build this project is Visual Studio's [Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019). You don't need the Visual Studio IDE. Simply run build.bat from a command line that has access to these build tools. You can use "Developer Command Prompt for VS20**" or find and run vcvarsall.bat (probably located in "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvarsall.bat") from the regular Command Prompt. Make sure your command line is for the same platform as our build (x64 for x64).

## License

<img src="https://i.imgur.com/V9VYXiR.png" alt="license" width="480" height="270">
