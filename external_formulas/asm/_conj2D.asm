; Disassembly: _conj2D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 13 bytes, decoded: 13 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    DD 02                   fld      qword ptr [edx]
0005:    D9 E0                   fchs    ; -ST(0)
0007:    DD 1A                   fstp     qword ptr [edx]
0009:    5D                      pop      ebp
000A:    C2 08 00                ret      8
