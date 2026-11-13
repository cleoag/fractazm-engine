; Disassembly: _YplusSinZ.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 17 bytes, decoded: 17 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    DD 02                   fld      qword ptr [edx]
0005:    DD 01                   fld      qword ptr [ecx]
0007:    D9 FE                   fsin    ; sin(ST(0))
0009:    DE C1                   faddp    st(1)
000B:    DD 1A                   fstp     qword ptr [edx]
000D:    5D                      pop      ebp
000E:    C2 08 00                ret      8
