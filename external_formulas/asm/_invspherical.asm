; Disassembly: _invspherical.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 41 bytes, decoded: 41 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    DD 02                   fld      qword ptr [edx]
0005:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0007:    DD 01                   fld      qword ptr [ecx]
0009:    D9 FB                   fsincos    ; ST(0)=cos, push sin
000B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
000D:    DC 08                   fmul     qword ptr [eax]
000F:    DD 19                   fstp     qword ptr [ecx]
0011:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0013:    DC C9                   fmul     st(1), st(0)
0015:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0017:    DC 08                   fmul     qword ptr [eax]
0019:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
001B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
001D:    DC 08                   fmul     qword ptr [eax]
001F:    DE C9                   fmulp    st(1)
0021:    DD 1A                   fstp     qword ptr [edx]
0023:    DD 18                   fstp     qword ptr [eax]
0025:    5D                      pop      ebp
0026:    C2 08 00                ret      8
