; Disassembly: _FlipYZ.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 15 bytes, decoded: 15 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    DD 01                   fld      qword ptr [ecx]
0005:    DD 02                   fld      qword ptr [edx]
0007:    DD 19                   fstp     qword ptr [ecx]
0009:    DD 1A                   fstp     qword ptr [edx]
000B:    5D                      pop      ebp
000C:    C2 08 00                ret      8
