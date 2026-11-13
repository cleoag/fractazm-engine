@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86
cl /nologo /O2 /arch:SSE2 /fp:strict /Fa /D MB3D_PORTABLE_MATH_DLL /LD /Fe:portable_math.dll portable_math.c
