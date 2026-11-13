; Disassembly: _tospherical.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 45 bytes, decoded: 45 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    DD 00                   fld      qword ptr [eax]
0005:    DC 08                   fmul     qword ptr [eax]
0007:    DD 02                   fld      qword ptr [edx]
0009:    DC 0A                   fmul     qword ptr [edx]
000B:    DE C1                   faddp    st(1)
000D:    DD 01                   fld      qword ptr [ecx]
000F:    DC 09                   fmul     qword ptr [ecx]
0011:    D8 C1                   fadd     st(1)
0013:    D9 FA                   fsqrt    ; sqrt(ST(0))
0015:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0017:    D9 FA                   fsqrt    ; sqrt(ST(0))
0019:    DD 01                   fld      qword ptr [ecx]
001B:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
001D:    DD 02                   fld      qword ptr [edx]
001F:    DD 00                   fld      qword ptr [eax]
0021:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0023:    DD 19                   fstp     qword ptr [ecx]
0025:    DD 1A                   fstp     qword ptr [edx]
0027:    DD 18                   fstp     qword ptr [eax]
0029:    5D                      pop      ebp
002A:    C2 08 00                ret      8
