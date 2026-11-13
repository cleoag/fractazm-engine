; Disassembly: _gnarl2D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 103 bytes, decoded: 103 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 02                   fld      qword ptr [edx]
000C:    DD 46 D0                fld      qword ptr [esi - 0x30]
000F:    DE C9                   fmulp    st(1)
0011:    D9 FE                   fsin    ; sin(ST(0))
0013:    DD 02                   fld      qword ptr [edx]
0015:    DE C1                   faddp    st(1)
0017:    DD 46 D8                fld      qword ptr [esi - 0x28]
001A:    DE C9                   fmulp    st(1)
001C:    D9 FE                   fsin    ; sin(ST(0))
001E:    DD 02                   fld      qword ptr [edx]
0020:    DE C1                   faddp    st(1)
0022:    D9 FE                   fsin    ; sin(ST(0))
0024:    DD 46 E8                fld      qword ptr [esi - 0x18]
0027:    DE C9                   fmulp    st(1)
0029:    D9 E0                   fchs    ; -ST(0)
002B:    DD 00                   fld      qword ptr [eax]
002D:    DE C1                   faddp    st(1)
002F:    DD 46 F0                fld      qword ptr [esi - 0x10]
0032:    DE C9                   fmulp    st(1)
0034:    DD 00                   fld      qword ptr [eax]
0036:    DD 46 D0                fld      qword ptr [esi - 0x30]
0039:    DE C9                   fmulp    st(1)
003B:    D9 FE                   fsin    ; sin(ST(0))
003D:    DD 00                   fld      qword ptr [eax]
003F:    DE C1                   faddp    st(1)
0041:    DD 46 D8                fld      qword ptr [esi - 0x28]
0044:    DE C9                   fmulp    st(1)
0046:    D9 FE                   fsin    ; sin(ST(0))
0048:    DD 00                   fld      qword ptr [eax]
004A:    DE C1                   faddp    st(1)
004C:    D9 FE                   fsin    ; sin(ST(0))
004E:    DD 46 E0                fld      qword ptr [esi - 0x20]
0051:    DE C9                   fmulp    st(1)
0053:    D9 E0                   fchs    ; -ST(0)
0055:    DD 02                   fld      qword ptr [edx]
0057:    DE C1                   faddp    st(1)
0059:    DD 46 F0                fld      qword ptr [esi - 0x10]
005C:    DE C9                   fmulp    st(1)
005E:    DD 1A                   fstp     qword ptr [edx]
0060:    DD 18                   fstp     qword ptr [eax]
0062:    5E                      pop      esi
0063:    5D                      pop      ebp
0064:    C2 08 00                ret      8
