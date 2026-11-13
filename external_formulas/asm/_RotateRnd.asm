; Disassembly: _RotateRnd.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 482 bytes, decoded: 482 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    83 C4 D0                add      esp, -0x30
0011:    FF 86 D0 00 00 00       inc      dword ptr [esi + 0xd0]
0017:    89 54 24 10             mov      dword ptr [esp + 0x10], edx
001B:    90                      nop
001C:    8B 86 D0 00 00 00       mov      eax, dword ptr [esi + 0xd0]
0022:    66 2D 07 3D             sub      ax, 0x3d07
0026:    F7 E0                   mul      eax
0028:    89 44 24 0C             mov      dword ptr [esp + 0xc], eax
002C:    8B 47 DC                mov      eax, dword ptr [edi - 0x24]
002F:    66 F7 64 24 0C          mul      word ptr [esp + 0xc]
0034:    25 FF 3F 00 00          and      eax, 0x3fff
0039:    89 04 24                mov      dword ptr [esp], eax
003C:    8B 47 D8                mov      eax, dword ptr [edi - 0x28]
003F:    66 F7 64 24 0C          mul      word ptr [esp + 0xc]
0044:    25 FF 3F 00 00          and      eax, 0x3fff
0049:    89 44 24 04             mov      dword ptr [esp + 4], eax
004D:    8B 47 D4                mov      eax, dword ptr [edi - 0x2c]
0050:    66 F7 64 24 0C          mul      word ptr [esp + 0xc]
0055:    25 FF 3F 00 00          and      eax, 0x3fff
005A:    89 44 24 08             mov      dword ptr [esp + 8], eax
005E:    90                      nop
005F:    DD 47 F0                fld      qword ptr [edi - 0x10]
0062:    D9 E1                   fabs    ; |ST(0)|
0064:    DD 07                   fld      qword ptr [edi]
0066:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0068:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006C:    72 6E                   jb       0xdc    ; Conditional jump
006E:    66 BA 00 00             mov      dx, 0
0072:    DB 04 24                fild     dword ptr [esp]
0075:    DC 4F 08                fmul     qword ptr [edi + 8]
0078:    DC 67 10                fsub     qword ptr [edi + 0x10]
007B:    DC 4F F0                fmul     qword ptr [edi - 0x10]
007E:    D9 E8                   fld1    ; Load 1.0
0080:    D8 D9                   fcomp    st(1)
0082:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0084:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0086:    73 0A                   jae      0x92    ; Conditional jump
0088:    DD 07                   fld      qword ptr [edi]
008A:    DE E1                   fsubrp   st(1)
008C:    66 BA 01 00             mov      dx, 1
0090:    EB 16                   jmp      0xa8
0092:    D9 E8                   fld1    ; Load 1.0
0094:    D9 E0                   fchs    ; -ST(0)
0096:    D8 D9                   fcomp    st(1)
0098:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
009C:    72 0A                   jb       0xa8    ; Conditional jump
009E:    DD 07                   fld      qword ptr [edi]
00A0:    DE C1                   faddp    st(1)
00A2:    D9 E0                   fchs    ; -ST(0)
00A4:    66 BA 01 00             mov      dx, 1
00A8:    D9 C0                   fld      st(0)
00AA:    D8 C8                   fmul     st(0)
00AC:    D9 E8                   fld1    ; Load 1.0
00AE:    DE E1                   fsubrp   st(1)
00B0:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B2:    66 83 FA 00             cmp      dx, 0
00B6:    74 02                   je       0xba    ; Conditional jump
00B8:    D9 E0                   fchs    ; -ST(0)
00BA:    8B 54 24 10             mov      edx, dword ptr [esp + 0x10]
00BE:    DD 03                   fld      qword ptr [ebx]
00C0:    D8 CA                   fmul     st(2)
00C2:    DD 02                   fld      qword ptr [edx]
00C4:    D8 CA                   fmul     st(2)
00C6:    DE E1                   fsubrp   st(1)
00C8:    DD 03                   fld      qword ptr [ebx]
00CA:    D8 CA                   fmul     st(2)
00CC:    DD 02                   fld      qword ptr [edx]
00CE:    D8 CC                   fmul     st(4)
00D0:    DE C1                   faddp    st(1)
00D2:    DD 1B                   fstp     qword ptr [ebx]
00D4:    DD 1A                   fstp     qword ptr [edx]
00D6:    DD D8                   fstp     st(0)
00D8:    DD D8                   fstp     st(0)
00DA:    DD D8                   fstp     st(0)
00DC:    90                      nop
00DD:    DD 47 E8                fld      qword ptr [edi - 0x18]
00E0:    D9 E1                   fabs    ; |ST(0)|
00E2:    DD 07                   fld      qword ptr [edi]
00E4:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00E6:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E8:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00EA:    72 6B                   jb       0x157    ; Conditional jump
00EC:    66 BA 00 00             mov      dx, 0
00F0:    DB 44 24 04             fild     dword ptr [esp + 4]
00F4:    DC 4F 08                fmul     qword ptr [edi + 8]
00F7:    DC 67 10                fsub     qword ptr [edi + 0x10]
00FA:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00FD:    D9 E8                   fld1    ; Load 1.0
00FF:    D8 D9                   fcomp    st(1)
0101:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0103:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0105:    73 0A                   jae      0x111    ; Conditional jump
0107:    DD 07                   fld      qword ptr [edi]
0109:    DE E1                   fsubrp   st(1)
010B:    66 BA 01 00             mov      dx, 1
010F:    EB 16                   jmp      0x127
0111:    D9 E8                   fld1    ; Load 1.0
0113:    D9 E0                   fchs    ; -ST(0)
0115:    D8 D9                   fcomp    st(1)
0117:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0119:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
011B:    72 0A                   jb       0x127    ; Conditional jump
011D:    DD 07                   fld      qword ptr [edi]
011F:    DE C1                   faddp    st(1)
0121:    D9 E0                   fchs    ; -ST(0)
0123:    66 BA 01 00             mov      dx, 1
0127:    D9 C0                   fld      st(0)
0129:    D8 C8                   fmul     st(0)
012B:    D9 E8                   fld1    ; Load 1.0
012D:    DE E1                   fsubrp   st(1)
012F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0131:    66 83 FA 00             cmp      dx, 0
0135:    74 02                   je       0x139    ; Conditional jump
0137:    D9 E0                   fchs    ; -ST(0)
0139:    DD 03                   fld      qword ptr [ebx]
013B:    D8 CA                   fmul     st(2)
013D:    DD 01                   fld      qword ptr [ecx]
013F:    D8 CA                   fmul     st(2)
0141:    DE E1                   fsubrp   st(1)
0143:    DD 03                   fld      qword ptr [ebx]
0145:    D8 CA                   fmul     st(2)
0147:    DD 01                   fld      qword ptr [ecx]
0149:    D8 CC                   fmul     st(4)
014B:    DE C1                   faddp    st(1)
014D:    DD 1B                   fstp     qword ptr [ebx]
014F:    DD 19                   fstp     qword ptr [ecx]
0151:    DD D8                   fstp     st(0)
0153:    DD D8                   fstp     st(0)
0155:    DD D8                   fstp     st(0)
0157:    90                      nop
0158:    DD 47 E0                fld      qword ptr [edi - 0x20]
015B:    D9 E1                   fabs    ; |ST(0)|
015D:    DD 07                   fld      qword ptr [edi]
015F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0161:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0163:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0165:    72 6F                   jb       0x1d6    ; Conditional jump
0167:    66 BA 00 00             mov      dx, 0
016B:    DB 44 24 08             fild     dword ptr [esp + 8]
016F:    DC 4F 08                fmul     qword ptr [edi + 8]
0172:    DC 67 10                fsub     qword ptr [edi + 0x10]
0175:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0178:    D9 E8                   fld1    ; Load 1.0
017A:    D8 D9                   fcomp    st(1)
017C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
017E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0180:    73 0A                   jae      0x18c    ; Conditional jump
0182:    DD 07                   fld      qword ptr [edi]
0184:    DE E1                   fsubrp   st(1)
0186:    66 BA 01 00             mov      dx, 1
018A:    EB 16                   jmp      0x1a2
018C:    D9 E8                   fld1    ; Load 1.0
018E:    D9 E0                   fchs    ; -ST(0)
0190:    D8 D9                   fcomp    st(1)
0192:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0194:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0196:    72 0A                   jb       0x1a2    ; Conditional jump
0198:    DD 07                   fld      qword ptr [edi]
019A:    DE C1                   faddp    st(1)
019C:    D9 E0                   fchs    ; -ST(0)
019E:    66 BA 01 00             mov      dx, 1
01A2:    D9 C0                   fld      st(0)
01A4:    D8 C8                   fmul     st(0)
01A6:    D9 E8                   fld1    ; Load 1.0
01A8:    DE E1                   fsubrp   st(1)
01AA:    D9 FA                   fsqrt    ; sqrt(ST(0))
01AC:    66 83 FA 00             cmp      dx, 0
01B0:    74 02                   je       0x1b4    ; Conditional jump
01B2:    D9 E0                   fchs    ; -ST(0)
01B4:    8B 54 24 10             mov      edx, dword ptr [esp + 0x10]
01B8:    DD 02                   fld      qword ptr [edx]
01BA:    D8 CA                   fmul     st(2)
01BC:    DD 01                   fld      qword ptr [ecx]
01BE:    D8 CA                   fmul     st(2)
01C0:    DE E1                   fsubrp   st(1)
01C2:    DD 02                   fld      qword ptr [edx]
01C4:    D8 CA                   fmul     st(2)
01C6:    DD 01                   fld      qword ptr [ecx]
01C8:    D8 CC                   fmul     st(4)
01CA:    DE C1                   faddp    st(1)
01CC:    DD 1A                   fstp     qword ptr [edx]
01CE:    DD 19                   fstp     qword ptr [ecx]
01D0:    DD D8                   fstp     st(0)
01D2:    DD D8                   fstp     st(0)
01D4:    DD D8                   fstp     st(0)
01D6:    83 C4 30                add      esp, 0x30
01D9:    89 D8                   mov      eax, ebx
01DB:    5B                      pop      ebx
01DC:    5F                      pop      edi
01DD:    5E                      pop      esi
01DE:    5D                      pop      ebp
01DF:    C2 08 00                ret      8
