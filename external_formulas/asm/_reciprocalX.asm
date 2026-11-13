; Disassembly: _reciprocalX.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 15 bytes, decoded: 15 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    DD 00                   fld      qword ptr [eax]
0005:    D9 E8                   fld1    ; Load 1.0
0007:    DE F1                   fdivrp   st(1)
0009:    DD 18                   fstp     qword ptr [eax]
000B:    5D                      pop      ebp
000C:    C2 08 00                ret      8
