; Disassembly: _invtorical.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 44 bytes, decoded: 44 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 00                   fld      qword ptr [eax]
000C:    D9 E0                   fchs    ; -ST(0)
000E:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0011:    D9 E1                   fabs    ; |ST(0)|
0013:    DD 02                   fld      qword ptr [edx]
0015:    D9 FF                   fcos    ; cos(ST(0))
0017:    D9 E1                   fabs    ; |ST(0)|
0019:    DE C9                   fmulp    st(1)
001B:    DD 10                   fst      qword ptr [eax]
001D:    DD 02                   fld      qword ptr [edx]
001F:    D9 F2                   fptan
0021:    DE C9                   fmulp    st(1)
0023:    DE C9                   fmulp    st(1)
0025:    DD 1A                   fstp     qword ptr [edx]
0027:    5E                      pop      esi
0028:    5D                      pop      ebp
0029:    C2 08 00                ret      8
