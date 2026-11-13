; Disassembly: _tocylindrical.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 29 bytes, decoded: 29 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    DD 00                   fld      qword ptr [eax]
0005:    D8 C8                   fmul     st(0)
0007:    DD 02                   fld      qword ptr [edx]
0009:    D8 C8                   fmul     st(0)
000B:    DE C1                   faddp    st(1)
000D:    D9 FA                   fsqrt    ; sqrt(ST(0))
000F:    DD 02                   fld      qword ptr [edx]
0011:    DD 00                   fld      qword ptr [eax]
0013:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0015:    DD 1A                   fstp     qword ptr [edx]
0017:    DD 18                   fstp     qword ptr [eax]
0019:    5D                      pop      ebp
001A:    C2 08 00                ret      8
