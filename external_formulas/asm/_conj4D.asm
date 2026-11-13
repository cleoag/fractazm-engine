; Disassembly: _conj4D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 27 bytes, decoded: 27 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    DD 02                   fld      qword ptr [edx]
0005:    D9 E0                   fchs    ; -ST(0)
0007:    DD 1A                   fstp     qword ptr [edx]
0009:    DD 01                   fld      qword ptr [ecx]
000B:    D9 E0                   fchs    ; -ST(0)
000D:    DD 19                   fstp     qword ptr [ecx]
000F:    DD 41 08                fld      qword ptr [ecx + 8]
0012:    D9 E0                   fchs    ; -ST(0)
0014:    DD 59 08                fstp     qword ptr [ecx + 8]
0017:    5D                      pop      ebp
0018:    C2 08 00                ret      8
