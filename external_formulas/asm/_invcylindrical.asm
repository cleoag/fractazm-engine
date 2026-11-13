; Disassembly: _invcylindrical.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 21 bytes, decoded: 21 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    DD 02                   fld      qword ptr [edx]
0005:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0007:    DD 00                   fld      qword ptr [eax]
0009:    DC CA                   fmul     st(2), st(0)
000B:    DE C9                   fmulp    st(1)
000D:    DD 18                   fstp     qword ptr [eax]
000F:    DD 1A                   fstp     qword ptr [edx]
0011:    5D                      pop      ebp
0012:    C2 08 00                ret      8
