..\..\.build\win64_vs2015\bin\shadercRelease.exe -i . -f basic\vs_basic.sc -o dx11\vs_basic.bin --type vertex --platform windows -p vs_5_0 -O 3 
..\..\.build\win64_vs2015\bin\shadercRelease.exe -i . -f basic\fs_basic.sc -o dx11\fs_basic.bin --type fragment --platform windows -p ps_5_0 -O 3


REM embeds

..\..\.build\win64_vs2015\bin\shadercRelease.exe -i . -f basic\vs_basic.sc -o ..\..\src\graphics\vs_texture.bin.h --type vertex --platform windows -p vs_5_0 -O 3 --bin2c vs_texture_dx11
..\..\.build\win64_vs2015\bin\shadercRelease.exe -i . -f basic\fs_basic.sc -o ..\..\src\graphics\fs_texture.bin.h --type fragment --platform windows -p ps_5_0 -O 3 --bin2c fs_texture_dx11

REM ..\..\.build\win64_vs2015\bin\shadercRelease.exe -i . -f basic\fs_basic.sc -o gl\vs_basic.bin --type vertex --platform linux -p 440
REM ..\..\.build\win64_vs2015\bin\shadercRelease.exe -i . -f basic\fs_basic.sc -o gl\fs_basic.bin --type fragment --platform linux -p 440