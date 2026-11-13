; Disassembly: _vanderpol2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 162 bytes, decoded: 162 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 00                   fld      qword ptr [eax]
000C:    DC 4E B8                fmul     qword ptr [esi - 0x48]
000F:    DD 02                   fld      qword ptr [edx]
0011:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0014:    DE C1                   faddp    st(1)
0016:    DD 01                   fld      qword ptr [ecx]
0018:    DC 4E E0                fmul     qword ptr [esi - 0x20]
001B:    DE C1                   faddp    st(1)
001D:    DD 41 08                fld      qword ptr [ecx + 8]
0020:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0023:    DE C1                   faddp    st(1)
0025:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0028:    D9 D0                   fnop
002A:    DD 46 D0                fld      qword ptr [esi - 0x30]
002D:    DC 09                   fmul     qword ptr [ecx]
002F:    DC 49 08                fmul     qword ptr [ecx + 8]
0032:    DD 46 E8                fld      qword ptr [esi - 0x18]
0035:    DC 08                   fmul     qword ptr [eax]
0037:    D9 E0                   fchs    ; -ST(0)
0039:    DE C1                   faddp    st(1)
003B:    DD 00                   fld      qword ptr [eax]
003D:    D8 C8                   fmul     st(0)
003F:    D9 E0                   fchs    ; -ST(0)
0041:    DC 4E B0                fmul     qword ptr [esi - 0x50]
0044:    DC 46 E8                fadd     qword ptr [esi - 0x18]
0047:    DC 0A                   fmul     qword ptr [edx]
0049:    DE C1                   faddp    st(1)
004B:    D9 D0                   fnop
004D:    DD 46 C8                fld      qword ptr [esi - 0x38]
0050:    DC 0A                   fmul     qword ptr [edx]
0052:    DC 49 08                fmul     qword ptr [ecx + 8]
0055:    DD 46 E0                fld      qword ptr [esi - 0x20]
0058:    DC 08                   fmul     qword ptr [eax]
005A:    D9 E0                   fchs    ; -ST(0)
005C:    DE C1                   faddp    st(1)
005E:    DD 00                   fld      qword ptr [eax]
0060:    D8 C8                   fmul     st(0)
0062:    D9 E0                   fchs    ; -ST(0)
0064:    DC 4E B0                fmul     qword ptr [esi - 0x50]
0067:    DC 46 E0                fadd     qword ptr [esi - 0x20]
006A:    DC 09                   fmul     qword ptr [ecx]
006C:    DE C1                   faddp    st(1)
006E:    D9 D0                   fnop
0070:    DD 46 C0                fld      qword ptr [esi - 0x40]
0073:    DC 09                   fmul     qword ptr [ecx]
0075:    DC 0A                   fmul     qword ptr [edx]
0077:    90                      nop
0078:    DD 46 D8                fld      qword ptr [esi - 0x28]
007B:    DC 08                   fmul     qword ptr [eax]
007D:    D9 E0                   fchs    ; -ST(0)
007F:    DE C1                   faddp    st(1)
0081:    DD 00                   fld      qword ptr [eax]
0083:    D8 C8                   fmul     st(0)
0085:    D9 E0                   fchs    ; -ST(0)
0087:    DC 4E B0                fmul     qword ptr [esi - 0x50]
008A:    DC 46 D8                fadd     qword ptr [esi - 0x28]
008D:    DC 49 08                fmul     qword ptr [ecx + 8]
0090:    DE C1                   faddp    st(1)
0092:    D9 D0                   fnop
0094:    DD 59 08                fstp     qword ptr [ecx + 8]
0097:    DD 19                   fstp     qword ptr [ecx]
0099:    DD 1A                   fstp     qword ptr [edx]
009B:    DD 18                   fstp     qword ptr [eax]
009D:    5E                      pop      esi
009E:    5D                      pop      ebp
009F:    C2 08 00                ret      8
