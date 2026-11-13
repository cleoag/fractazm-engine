; Disassembly: _SinePow2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 76 bytes, decoded: 76 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    DD 01                   fld      qword ptr [ecx]
0009:    DD 02                   fld      qword ptr [edx]
000B:    DD 00                   fld      qword ptr [eax]
000D:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0010:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
0013:    D9 C1                   fld      st(1)
0015:    D8 CA                   fmul     st(2)
0017:    D9 C1                   fld      st(1)
0019:    D8 CA                   fmul     st(2)
001B:    D9 C0                   fld      st(0)
001D:    D8 C2                   fadd     st(2)
001F:    D9 C0                   fld      st(0)
0021:    D9 FA                   fsqrt    ; sqrt(ST(0))
0023:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0026:    D8 CE                   fmul     st(6)
0028:    D8 C0                   fadd     st(0)
002A:    DD 19                   fstp     qword ptr [ecx]
002C:    D9 C5                   fld      st(5)
002E:    DE CE                   fmulp    st(6)
0030:    D9 C0                   fld      st(0)
0032:    DE E6                   fsubrp   st(6)
0034:    DE FD                   fdivp    st(5)
0036:    DE E1                   fsubrp   st(1)
0038:    D8 CB                   fmul     st(3)
003A:    DD 18                   fstp     qword ptr [eax]
003C:    DE C9                   fmulp    st(1)
003E:    DE C9                   fmulp    st(1)
0040:    D8 C0                   fadd     st(0)
0042:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0045:    DD 1A                   fstp     qword ptr [edx]
0047:    5E                      pop      esi
0048:    5D                      pop      ebp
0049:    C2 08 00                ret      8
