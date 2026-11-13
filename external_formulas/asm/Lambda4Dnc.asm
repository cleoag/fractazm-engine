; Disassembly: Lambda4Dnc.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 302 bytes, decoded: 302 bytes (100.0%)
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
0032:    90                      nop
0033:    90                      nop
0034:    DE C1                   faddp    st(1)
0036:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0039:    D9 D0                   fnop
003B:    DD 03                   fld      qword ptr [ebx]
003D:    90                      nop
003E:    DC 0A                   fmul     qword ptr [edx]
0040:    D9 E0                   fchs    ; -ST(0)
0042:    90                      nop
0043:    D8 C0                   fadd     st(0)
0045:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0048:    D9 D0                   fnop
004A:    DD 03                   fld      qword ptr [ebx]
004C:    90                      nop
004D:    DC 09                   fmul     qword ptr [ecx]
004F:    D9 E0                   fchs    ; -ST(0)
0051:    90                      nop
0052:    D8 C0                   fadd     st(0)
0054:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0057:    D9 D0                   fnop
0059:    DD 03                   fld      qword ptr [ebx]
005B:    90                      nop
005C:    DC 49 08                fmul     qword ptr [ecx + 8]
005F:    D9 E0                   fchs    ; -ST(0)
0061:    D8 C0                   fadd     st(0)
0063:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0066:    D9 D0                   fnop
0068:    D9 D0                   fnop
006A:    D9 D0                   fnop
006C:    D9 D0                   fnop
006E:    90                      nop
006F:    90                      nop
0070:    DD 59 08                fstp     qword ptr [ecx + 8]
0073:    DD 19                   fstp     qword ptr [ecx]
0075:    DD 1A                   fstp     qword ptr [edx]
0077:    DD 1B                   fstp     qword ptr [ebx]
0079:    DD 03                   fld      qword ptr [ebx]
007B:    90                      nop
007C:    DC 4E 18                fmul     qword ptr [esi + 0x18]
007F:    90                      nop
0080:    90                      nop
0081:    DD 02                   fld      qword ptr [edx]
0083:    90                      nop
0084:    DC 4E 20                fmul     qword ptr [esi + 0x20]
0087:    D9 E0                   fchs    ; -ST(0)
0089:    DE C1                   faddp    st(1)
008B:    DD 01                   fld      qword ptr [ecx]
008D:    90                      nop
008E:    DC 4E 28                fmul     qword ptr [esi + 0x28]
0091:    D9 E0                   fchs    ; -ST(0)
0093:    DE C1                   faddp    st(1)
0095:    DD 41 08                fld      qword ptr [ecx + 8]
0098:    DC 4E C8                fmul     qword ptr [esi - 0x38]
009B:    D9 E0                   fchs    ; -ST(0)
009D:    DE C1                   faddp    st(1)
009F:    D9 D0                   fnop
00A1:    DD 03                   fld      qword ptr [ebx]
00A3:    90                      nop
00A4:    DC 4E 20                fmul     qword ptr [esi + 0x20]
00A7:    90                      nop
00A8:    90                      nop
00A9:    DD 02                   fld      qword ptr [edx]
00AB:    90                      nop
00AC:    DC 4E 18                fmul     qword ptr [esi + 0x18]
00AF:    90                      nop
00B0:    90                      nop
00B1:    DE C1                   faddp    st(1)
00B3:    DD 01                   fld      qword ptr [ecx]
00B5:    90                      nop
00B6:    DC 4E C8                fmul     qword ptr [esi - 0x38]
00B9:    90                      nop
00BA:    90                      nop
00BB:    DE C1                   faddp    st(1)
00BD:    DD 41 08                fld      qword ptr [ecx + 8]
00C0:    DC 4E 28                fmul     qword ptr [esi + 0x28]
00C3:    D9 E0                   fchs    ; -ST(0)
00C5:    DE C1                   faddp    st(1)
00C7:    D9 D0                   fnop
00C9:    DD 03                   fld      qword ptr [ebx]
00CB:    90                      nop
00CC:    DC 4E 28                fmul     qword ptr [esi + 0x28]
00CF:    90                      nop
00D0:    90                      nop
00D1:    DD 02                   fld      qword ptr [edx]
00D3:    90                      nop
00D4:    DC 4E C8                fmul     qword ptr [esi - 0x38]
00D7:    D9 E0                   fchs    ; -ST(0)
00D9:    DE C1                   faddp    st(1)
00DB:    DD 01                   fld      qword ptr [ecx]
00DD:    90                      nop
00DE:    DC 4E 18                fmul     qword ptr [esi + 0x18]
00E1:    90                      nop
00E2:    90                      nop
00E3:    DE C1                   faddp    st(1)
00E5:    DD 41 08                fld      qword ptr [ecx + 8]
00E8:    DC 4E 20                fmul     qword ptr [esi + 0x20]
00EB:    90                      nop
00EC:    90                      nop
00ED:    DE C1                   faddp    st(1)
00EF:    D9 D0                   fnop
00F1:    DD 03                   fld      qword ptr [ebx]
00F3:    90                      nop
00F4:    DC 4E C8                fmul     qword ptr [esi - 0x38]
00F7:    90                      nop
00F8:    90                      nop
00F9:    DD 02                   fld      qword ptr [edx]
00FB:    90                      nop
00FC:    DC 4E 28                fmul     qword ptr [esi + 0x28]
00FF:    90                      nop
0100:    90                      nop
0101:    DE C1                   faddp    st(1)
0103:    DD 01                   fld      qword ptr [ecx]
0105:    90                      nop
0106:    DC 4E 20                fmul     qword ptr [esi + 0x20]
0109:    D9 E0                   fchs    ; -ST(0)
010B:    DE C1                   faddp    st(1)
010D:    DD 41 08                fld      qword ptr [ecx + 8]
0110:    DC 4E 18                fmul     qword ptr [esi + 0x18]
0113:    90                      nop
0114:    90                      nop
0115:    DE C1                   faddp    st(1)
0117:    D9 D0                   fnop
0119:    90                      nop
011A:    DD 59 08                fstp     qword ptr [ecx + 8]
011D:    DD 19                   fstp     qword ptr [ecx]
011F:    DD 1A                   fstp     qword ptr [edx]
0121:    DD 1B                   fstp     qword ptr [ebx]
0123:    90                      nop
0124:    8B C3                   mov      eax, ebx
0126:    5F                      pop      edi
0127:    5E                      pop      esi
0128:    58                      pop      eax
0129:    5B                      pop      ebx
012A:    5D                      pop      ebp
012B:    C2 08 00                ret      8
