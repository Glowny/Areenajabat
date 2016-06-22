::@ECHO OFF

REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

REM msbuild .build\projects\vs2015\shaderc.vcxproj /target:Build /property:Configuration=Release;Platform=x64


cd assets\shaders\
compile.bat
cd ..\..\