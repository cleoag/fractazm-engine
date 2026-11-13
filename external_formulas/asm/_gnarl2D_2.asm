; Disassembly: _gnarl2D_2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 92 bytes, decoded: 92 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 00                   fld      qword ptr [eax]
000C:    D9 C0                   fld      st(0)
000E:    D9 FF                   fcos    ; cos(ST(0))
0010:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0013:    DE C1                   faddp    st(1)
0015:    D9 FE                   fsin    ; sin(ST(0))
0017:    DC 4E D8                fmul     qword ptr [esi - 0x28]
001A:    DD 02                   fld      qword ptr [edx]
001C:    DC 00                   fadd     qword ptr [eax]
001E:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0021:    DE E1                   fsubrp   st(1)
0023:    D9 FE                   fsin    ; sin(ST(0))
0025:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0028:    DC 02                   fadd     qword ptr [edx]
002A:    90                      nop
002B:    DD 02                   fld      qword ptr [edx]
002D:    D9 C0                   fld      st(0)
002F:    D9 FF                   fcos    ; cos(ST(0))
0031:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0034:    DE C1                   faddp    st(1)
0036:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0039:    D9 FE                   fsin    ; sin(ST(0))
003B:    DD 02                   fld      qword ptr [edx]
003D:    DC 28                   fsubr    qword ptr [eax]
003F:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0042:    DE C1                   faddp    st(1)
0044:    D9 FE                   fsin    ; sin(ST(0))
0046:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0049:    DC 00                   fadd     qword ptr [eax]
004B:    90                      nop
004C:    DD 46 F0                fld      qword ptr [esi - 0x10]
004F:    DC C9                   fmul     st(1), st(0)
0051:    DE CA                   fmulp    st(2)
0053:    DD 1A                   fstp     qword ptr [edx]
0055:    DD 18                   fstp     qword ptr [eax]
0057:    5E                      pop      esi
0058:    5D                      pop      ebp
0059:    C2 08 00                ret      8
