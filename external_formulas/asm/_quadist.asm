; Disassembly: _quadist.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 168 bytes, decoded: 168 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 00                   fld      qword ptr [eax]
000C:    DC 4E F0                fmul     qword ptr [esi - 0x10]
000F:    DD 00                   fld      qword ptr [eax]
0011:    D8 C8                   fmul     st(0)
0013:    DD 46 D0                fld      qword ptr [esi - 0x30]
0016:    DC 66 E8                fsub     qword ptr [esi - 0x18]
0019:    DE C9                   fmulp    st(1)
001B:    DE C1                   faddp    st(1)
001D:    DD 02                   fld      qword ptr [edx]
001F:    D8 C8                   fmul     st(0)
0021:    DD 46 C8                fld      qword ptr [esi - 0x38]
0024:    DC 66 E0                fsub     qword ptr [esi - 0x20]
0027:    DE C9                   fmulp    st(1)
0029:    DE C1                   faddp    st(1)
002B:    DD 01                   fld      qword ptr [ecx]
002D:    D8 C8                   fmul     st(0)
002F:    DD 46 C0                fld      qword ptr [esi - 0x40]
0032:    DC 66 D8                fsub     qword ptr [esi - 0x28]
0035:    DE C9                   fmulp    st(1)
0037:    DE C1                   faddp    st(1)
0039:    D9 D0                   fnop
003B:    DD 02                   fld      qword ptr [edx]
003D:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0040:    DD 00                   fld      qword ptr [eax]
0042:    D8 C8                   fmul     st(0)
0044:    DD 46 C0                fld      qword ptr [esi - 0x40]
0047:    DC 66 E8                fsub     qword ptr [esi - 0x18]
004A:    DE C9                   fmulp    st(1)
004C:    DE C1                   faddp    st(1)
004E:    DD 02                   fld      qword ptr [edx]
0050:    D8 C8                   fmul     st(0)
0052:    DD 46 D0                fld      qword ptr [esi - 0x30]
0055:    DC 66 E0                fsub     qword ptr [esi - 0x20]
0058:    DE C9                   fmulp    st(1)
005A:    DE C1                   faddp    st(1)
005C:    DD 01                   fld      qword ptr [ecx]
005E:    D8 C8                   fmul     st(0)
0060:    DD 46 C8                fld      qword ptr [esi - 0x38]
0063:    DC 66 D8                fsub     qword ptr [esi - 0x28]
0066:    DE C9                   fmulp    st(1)
0068:    DE C1                   faddp    st(1)
006A:    D9 D0                   fnop
006C:    DD 01                   fld      qword ptr [ecx]
006E:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0071:    DD 00                   fld      qword ptr [eax]
0073:    D8 C8                   fmul     st(0)
0075:    DD 46 C8                fld      qword ptr [esi - 0x38]
0078:    DC 66 E8                fsub     qword ptr [esi - 0x18]
007B:    DE C9                   fmulp    st(1)
007D:    DE C1                   faddp    st(1)
007F:    DD 02                   fld      qword ptr [edx]
0081:    D8 C8                   fmul     st(0)
0083:    DD 46 C0                fld      qword ptr [esi - 0x40]
0086:    DC 66 E0                fsub     qword ptr [esi - 0x20]
0089:    DE C9                   fmulp    st(1)
008B:    DE C1                   faddp    st(1)
008D:    DD 01                   fld      qword ptr [ecx]
008F:    D8 C8                   fmul     st(0)
0091:    DD 46 D0                fld      qword ptr [esi - 0x30]
0094:    DC 66 D8                fsub     qword ptr [esi - 0x28]
0097:    DE C9                   fmulp    st(1)
0099:    DE C1                   faddp    st(1)
009B:    D9 D0                   fnop
009D:    DD 19                   fstp     qword ptr [ecx]
009F:    DD 1A                   fstp     qword ptr [edx]
00A1:    DD 18                   fstp     qword ptr [eax]
00A3:    5E                      pop      esi
00A4:    5D                      pop      ebp
00A5:    C2 08 00                ret      8
