@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
cl /nologo /O2 /arch:SSE2 /fp:strict /Fe:test_pm_x64.exe portable_math.c test_pm.c
