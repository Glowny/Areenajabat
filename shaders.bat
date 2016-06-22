::@ECHO OFF

REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

REM msbuild .build\projects\vs2015\shaderc.vcxproj /target:Build /property:Configuration=Release;Platform=x64


cd assets\shaders\


..\..\.build\win64_vs2015\bin\shadercRelease.exe -i . -f basic\vs_basic.sc -o dx11\vs_basic.bin --type vertex --platform windows -p vs_5_0 -O 3 
..\..\.build\win64_vs2015\bin\shadercRelease.exe -i . -f basic\fs_basic.sc -o dx11\fs_basic.bin --type fragment --platform windows -p ps_5_0 -O 3


cd ..\..