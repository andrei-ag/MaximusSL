@echo off
bc compiler.prj /m
if exist *.swp del *.swp
if exist find.obj del find.obj
upx -9 compiler.exe
markexe compiler.exe compiler.nfo
upx -t compiler.exe
copy /y compiler.exe c:\utils >nul
echo press any key to continue
pause >nul