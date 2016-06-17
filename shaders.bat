::@ECHO OFF

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

msbuild .build\projects\vs2015\shaderc.vcxproj /target:Build /property:Configuration=Release;Platform=x64


for %%f in ("./assets/shaders/*") (
	echo %%f
)


.build\win64_vs2015\bin\shadercRelease.exe 