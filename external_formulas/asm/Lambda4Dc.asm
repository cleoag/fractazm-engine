; Disassembly: Lambda4Dc.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 330 bytes, decoded: 330 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    50                      push     eax
0005:    56                      push     esi
0006:    57                      push     edi
0007:    90                      nop
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B D8                   mov      ebx, eax
000D:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0010:    DD 03                   fld      qword ptr [ebx]
0012:    90                      nop
0013:    DC 0B                   fmul     qword ptr [ebx]
0015:    D9 E0                   fchs    ; -ST(0)
0017:    90                      nop
0018:    DD 02                   fld      qword ptr [edx]
001A:    90                      nop
001B:    DC 0A                   fmul     qword ptr [edx]
001D:    90                      nop
001E:    90                      nop
001F:    90                      nop
0020:    DE C1                   faddp    st(1)
0022:    DD 01                   fld      qword ptr [ecx]
0024:    90                      nop
0025:    DC 09                   fmul     qword ptr [ecx]
0027:    90                      nop
0028:    90                      nop
0029:    90                      nop
002A:    DE C1                   faddp    st(1)
002C:    DD 41 08                fld      qword ptr [ecx + 8]
002F:    DC 49 08                fmul     qword ptr [ecx + 8]
0032:    D9 E0                   fchs    ; -ST(0)
0034:    DE C1                   faddp    st(1)
0036:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0039:    D9 D0                   fnop
003B:    DD 03                   fld      qword ptr [ebx]
003D:    90                      nop
003E:    DC 0A                   fmul     qword ptr [edx]
0040:    D9 E0                   fchs    ; -ST(0)
0042:    90                      nop
0043:    D8 C0                   fadd     st(0)
0045:    DD 01                   fld      qword ptr [ecx]
0047:    90                      nop
0048:    DC 49 08                fmul     qword ptr [ecx + 8]
004B:    90                      nop
004C:    90                      nop
004D:    D8 C0                   fadd     st(0)
004F:    DE C1                   faddp    st(1)
0051:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0054:    D9 D0                   fnop
0056:    DD 03                   fld      qword ptr [ebx]
0058:    90                      nop
0059:    DC 09                   fmul     qword ptr [ecx]
005B:    D9 E0                   fchs    ; -ST(0)
005D:    90                      nop
005E:    D8 C0                   fadd     st(0)
0060:    DD 02                   fld      qword ptr [edx]
0062:    90                      nop
0063:    DC 49 08                fmul     qword ptr [ecx + 8]
0066:    90                      nop
0067:    90                      nop
0068:    D8 C0                   fadd     st(0)
006A:    DE C1                   faddp    st(1)
006C:    DC 47 E0                fadd     qword ptr [edi - 0x20]
006F:    D9 D0                   fnop
0071:    DD 03                   fld      qword ptr [ebx]
0073:    90                      nop
0074:    DC 49 08                fmul     qword ptr [ecx + 8]
0077:    D9 E0                   fchs    ; -ST(0)
0079:    D8 C0                   fadd     st(0)
007B:    DD 02                   fld      qword ptr [edx]
007D:    90                      nop
007E:    DC 09                   fmul     qword ptr [ecx]
0080:    D9 E0                   fchs    ; -ST(0)
0082:    90                      nop
0083:    D8 C0                   fadd     st(0)
0085:    DE C1                   faddp    st(1)
0087:    DC 47 D8                fadd     qword ptr [edi - 0x28]
008A:    D9 D0                   fnop
008C:    DD 59 08                fstp     qword ptr [ecx + 8]
008F:    DD 19                   fstp     qword ptr [ecx]
0091:    DD 1A                   fstp     qword ptr [edx]
0093:    DD 1B                   fstp     qword ptr [ebx]
0095:    DD 03                   fld      qword ptr [ebx]
0097:    90                      nop
0098:    DC 4E 18                fmul     qword ptr [esi + 0x18]
009B:    90                      nop
009C:    90                      nop
009D:    DD 02                   fld      qword ptr [edx]
009F:    90                      nop
00A0:    DC 4E 20                fmul     qword ptr [esi + 0x20]
00A3:    D9 E0                   fchs    ; -ST(0)
00A5:    DE C1                   faddp    st(1)
00A7:    DD 01                   fld      qword ptr [ecx]
00A9:    90                      nop
00AA:    DC 4E 28                fmul     qword ptr [esi + 0x28]
00AD:    D9 E0                   fchs    ; -ST(0)
00AF:    DE C1                   faddp    st(1)
00B1:    DD 41 08                fld      qword ptr [ecx + 8]
00B4:    DC 4E C8                fmul     qword ptr [esi - 0x38]
00B7:    90                      nop
00B8:    90                      nop
00B9:    DE C1                   faddp    st(1)
00BB:    D9 D0                   fnop
00BD:    DD 02                   fld      qword ptr [edx]
00BF:    90                      nop
00C0:    DC 4E 18                fmul     qword ptr [esi + 0x18]
00C3:    90                      nop
00C4:    90                      nop
00C5:    DD 03                   fld      qword ptr [ebx]
00C7:    90                      nop
00C8:    DC 4E 20                fmul     qword ptr [esi + 0x20]
00CB:    90                      nop
00CC:    90                      nop
00CD:    DE C1                   faddp    st(1)
00CF:    DD 41 08                fld      qword ptr [ecx + 8]
00D2:    DC 4E 28                fmul     qword ptr [esi + 0x28]
00D5:    D9 E0                   fchs    ; -ST(0)
00D7:    DE C1                   faddp    st(1)
00D9:    DD 01                   fld      qword ptr [ecx]
00DB:    90                      nop
00DC:    DC 4E C8                fmul     qword ptr [esi - 0x38]
00DF:    D9 E0                   fchs    ; -ST(0)
00E1:    DE C1                   faddp    st(1)
00E3:    D9 D0                   fnop
00E5:    DD 01                   fld      qword ptr [ecx]
00E7:    90                      nop
00E8:    DC 4E 18                fmul     qword ptr [esi + 0x18]
00EB:    90                      nop
00EC:    90                      nop
00ED:    DD 41 08                fld      qword ptr [ecx + 8]
00F0:    DC 4E 20                fmul     qword ptr [esi + 0x20]
00F3:    D9 E0                   fchs    ; -ST(0)
00F5:    DE C1                   faddp    st(1)
00F7:    DD 03                   fld      qword ptr [ebx]
00F9:    90                      nop
00FA:    DC 4E 28                fmul     qword ptr [esi + 0x28]
00FD:    90                      nop
00FE:    90                      nop
00FF:    DE C1                   faddp    st(1)
0101:    DD 02                   fld      qword ptr [edx]
0103:    90                      nop
0104:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0107:    D9 E0                   fchs    ; -ST(0)
0109:    DE C1                   faddp    st(1)
010B:    D9 D0                   fnop
010D:    DD 41 08                fld      qword ptr [ecx + 8]
0110:    DC 4E 18                fmul     qword ptr [esi + 0x18]
0113:    90                      nop
0114:    90                      nop
0115:    DD 01                   fld      qword ptr [ecx]
0117:    90                      nop
0118:    DC 4E 20                fmul     qword ptr [esi + 0x20]
011B:    90                      nop
011C:    90                      nop
011D:    DE C1                   faddp    st(1)
011F:    DD 02                   fld      qword ptr [edx]
0121:    90                      nop
0122:    DC 4E 28                fmul     qword ptr [esi + 0x28]
0125:    90                      nop
0126:    90                      nop
0127:    DE C1                   faddp    st(1)
0129:    DD 03                   fld      qword ptr [ebx]
012B:    90                      nop
012C:    DC 4E C8                fmul     qword ptr [esi - 0x38]
012F:    90                      nop
0130:    90                      nop
0131:    DE C1                   faddp    st(1)
0133:    D9 D0                   fnop
0135:    90                      nop
0136:    DD 59 08                fstp     qword ptr [ecx + 8]
0139:    DD 19                   fstp     qword ptr [ecx]
013B:    DD 1A                   fstp     qword ptr [edx]
013D:    DD 1B                   fstp     qword ptr [ebx]
013F:    90                      nop
0140:    8B C3                   mov      eax, ebx
0142:    5F                      pop      edi
0143:    5E                      pop      esi
0144:    58                      pop      eax
0145:    5B                      pop      ebx
0146:    5D                      pop      ebp
0147:    C2 08 00                ret      8
