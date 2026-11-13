; Disassembly: _totorical.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 42 bytes, decoded: 42 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 00                   fld      qword ptr [eax]
000C:    D8 C8                   fmul     st(0)
000E:    DD 02                   fld      qword ptr [edx]
0010:    D8 C8                   fmul     st(0)
0012:    DE C1                   faddp    st(1)
0014:    D9 FA                   fsqrt    ; sqrt(ST(0))
0016:    D9 E0                   fchs    ; -ST(0)
0018:    DC 46 F0                fadd     qword ptr [esi - 0x10]
001B:    DD 02                   fld      qword ptr [edx]
001D:    DD 00                   fld      qword ptr [eax]
001F:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0021:    DD 1A                   fstp     qword ptr [edx]
0023:    DD 18                   fstp     qword ptr [eax]
0025:    5E                      pop      esi
0026:    5D                      pop      ebp
0027:    C2 08 00                ret      8
