; Disassembly: Barnsley4Dc.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 484 bytes, decoded: 484 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 18 00 00 00       sub      esp, 0x18
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
0018:    0F 84 83 00 00 00       je       0xa1    ; Conditional jump
001E:    DD 03                   fld      qword ptr [ebx]
0020:    DD 47 F0                fld      qword ptr [edi - 0x10]
0023:    D8 C8                   fmul     st(0)
0025:    DD 47 E8                fld      qword ptr [edi - 0x18]
0028:    D8 C8                   fmul     st(0)
002A:    DE C1                   faddp    st(1)
002C:    DD 47 E0                fld      qword ptr [edi - 0x20]
002F:    D8 C8                   fmul     st(0)
0031:    DE C1                   faddp    st(1)
0033:    DD 47 D8                fld      qword ptr [edi - 0x28]
0036:    D8 C8                   fmul     st(0)
0038:    DE C1                   faddp    st(1)
003A:    D9 FA                   fsqrt    ; sqrt(ST(0))
003C:    DC 0F                   fmul     qword ptr [edi]
003E:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0040:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0042:    80 E4 41                and      ah, 0x41
0045:    77 26                   ja       0x6d    ; Conditional jump
0047:    DD 03                   fld      qword ptr [ebx]
0049:    DC 4F 08                fmul     qword ptr [edi + 8]
004C:    DD 1B                   fstp     qword ptr [ebx]
004E:    DD 02                   fld      qword ptr [edx]
0050:    DC 4F 08                fmul     qword ptr [edi + 8]
0053:    DD 1A                   fstp     qword ptr [edx]
0055:    DD 01                   fld      qword ptr [ecx]
0057:    DC 4F 08                fmul     qword ptr [edi + 8]
005A:    DD 19                   fstp     qword ptr [ecx]
005C:    DD 41 08                fld      qword ptr [ecx + 8]
005F:    DC 4F 08                fmul     qword ptr [edi + 8]
0062:    DC 47 D0                fadd     qword ptr [edi - 0x30]
0065:    DD 59 08                fstp     qword ptr [ecx + 8]
0068:    E9 B3 00 00 00          jmp      0x120
006D:    DD 47 F0                fld      qword ptr [edi - 0x10]
0070:    DC 23                   fsub     qword ptr [ebx]
0072:    DC 4F 08                fmul     qword ptr [edi + 8]
0075:    DD 1B                   fstp     qword ptr [ebx]
0077:    DD 47 E8                fld      qword ptr [edi - 0x18]
007A:    DC 22                   fsub     qword ptr [edx]
007C:    DC 4F 08                fmul     qword ptr [edi + 8]
007F:    DD 1A                   fstp     qword ptr [edx]
0081:    DD 47 E0                fld      qword ptr [edi - 0x20]
0084:    DC 21                   fsub     qword ptr [ecx]
0086:    DC 4F 08                fmul     qword ptr [edi + 8]
0089:    DD 19                   fstp     qword ptr [ecx]
008B:    DD 47 D8                fld      qword ptr [edi - 0x28]
008E:    DC 61 08                fsub     qword ptr [ecx + 8]
0091:    DC 47 D0                fadd     qword ptr [edi - 0x30]
0094:    DC 4F 08                fmul     qword ptr [edi + 8]
0097:    DD 59 08                fstp     qword ptr [ecx + 8]
009A:    90                      nop
009B:    EB 56                   jmp      0xf3
009D:    90                      nop
009E:    90                      nop
009F:    90                      nop
00A0:    90                      nop
00A1:    DD 03                   fld      qword ptr [ebx]
00A3:    DD 47 C8                fld      qword ptr [edi - 0x38]
00A6:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00A8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AA:    80 E4 41                and      ah, 0x41
00AD:    77 23                   ja       0xd2    ; Conditional jump
00AF:    DD 03                   fld      qword ptr [ebx]
00B1:    DC 47 F0                fadd     qword ptr [edi - 0x10]
00B4:    DD 1B                   fstp     qword ptr [ebx]
00B6:    DD 02                   fld      qword ptr [edx]
00B8:    DC 47 E8                fadd     qword ptr [edi - 0x18]
00BB:    DD 1A                   fstp     qword ptr [edx]
00BD:    DD 01                   fld      qword ptr [ecx]
00BF:    DC 47 E0                fadd     qword ptr [edi - 0x20]
00C2:    DD 19                   fstp     qword ptr [ecx]
00C4:    DD 41 08                fld      qword ptr [ecx + 8]
00C7:    DC 47 D8                fadd     qword ptr [edi - 0x28]
00CA:    DC 47 D0                fadd     qword ptr [edi - 0x30]
00CD:    DD 59 08                fstp     qword ptr [ecx + 8]
00D0:    EB 4E                   jmp      0x120
00D2:    DD 03                   fld      qword ptr [ebx]
00D4:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00D7:    DD 1B                   fstp     qword ptr [ebx]
00D9:    DD 02                   fld      qword ptr [edx]
00DB:    DC 67 E8                fsub     qword ptr [edi - 0x18]
00DE:    DD 1A                   fstp     qword ptr [edx]
00E0:    DD 01                   fld      qword ptr [ecx]
00E2:    DC 67 E0                fsub     qword ptr [edi - 0x20]
00E5:    DD 19                   fstp     qword ptr [ecx]
00E7:    DD 41 08                fld      qword ptr [ecx + 8]
00EA:    DC 67 D8                fsub     qword ptr [edi - 0x28]
00ED:    DC 47 D0                fadd     qword ptr [edi - 0x30]
00F0:    DD 59 08                fstp     qword ptr [ecx + 8]
00F3:    DD 46 20                fld      qword ptr [esi + 0x20]
00F6:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00F9:    83 7F C4 00             cmp      dword ptr [edi - 0x3c], 0
00FD:    74 09                   je       0x108    ; Conditional jump
00FF:    DD 46 20                fld      qword ptr [esi + 0x20]
0102:    D9 E0                   fchs    ; -ST(0)
0104:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0107:    90                      nop
0108:    83 7F C0 00             cmp      dword ptr [edi - 0x40], 0
010C:    74 19                   je       0x127    ; Conditional jump
010E:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0111:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0114:    DD 46 18                fld      qword ptr [esi + 0x18]
0117:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
011A:    EB 11                   jmp      0x12d
011C:    90                      nop
011D:    90                      nop
011E:    90                      nop
011F:    90                      nop
0120:    DD 46 20                fld      qword ptr [esi + 0x20]
0123:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0126:    90                      nop
0127:    DD 46 18                fld      qword ptr [esi + 0x18]
012A:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
012D:    DD 03                   fld      qword ptr [ebx]
012F:    90                      nop
0130:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0133:    90                      nop
0134:    90                      nop
0135:    DD 02                   fld      qword ptr [edx]
0137:    90                      nop
0138:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
013B:    D9 E0                   fchs    ; -ST(0)
013D:    DE C1                   faddp    st(1)
013F:    DD 01                   fld      qword ptr [ecx]
0141:    90                      nop
0142:    DC 4E 28                fmul     qword ptr [esi + 0x28]
0145:    D9 E0                   fchs    ; -ST(0)
0147:    DE C1                   faddp    st(1)
0149:    DD 41 08                fld      qword ptr [ecx + 8]
014C:    DC 4E C8                fmul     qword ptr [esi - 0x38]
014F:    90                      nop
0150:    90                      nop
0151:    DE C1                   faddp    st(1)
0153:    D9 D0                   fnop
0155:    DD 02                   fld      qword ptr [edx]
0157:    90                      nop
0158:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
015B:    90                      nop
015C:    90                      nop
015D:    DD 03                   fld      qword ptr [ebx]
015F:    90                      nop
0160:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0163:    90                      nop
0164:    90                      nop
0165:    DE C1                   faddp    st(1)
0167:    DD 41 08                fld      qword ptr [ecx + 8]
016A:    DC 4E 28                fmul     qword ptr [esi + 0x28]
016D:    D9 E0                   fchs    ; -ST(0)
016F:    DE C1                   faddp    st(1)
0171:    DD 01                   fld      qword ptr [ecx]
0173:    90                      nop
0174:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0177:    D9 E0                   fchs    ; -ST(0)
0179:    DE C1                   faddp    st(1)
017B:    D9 D0                   fnop
017D:    DD 01                   fld      qword ptr [ecx]
017F:    90                      nop
0180:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0183:    90                      nop
0184:    90                      nop
0185:    DD 41 08                fld      qword ptr [ecx + 8]
0188:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
018B:    D9 E0                   fchs    ; -ST(0)
018D:    DE C1                   faddp    st(1)
018F:    DD 03                   fld      qword ptr [ebx]
0191:    90                      nop
0192:    DC 4E 28                fmul     qword ptr [esi + 0x28]
0195:    90                      nop
0196:    90                      nop
0197:    DE C1                   faddp    st(1)
0199:    DD 02                   fld      qword ptr [edx]
019B:    90                      nop
019C:    DC 4E C8                fmul     qword ptr [esi - 0x38]
019F:    D9 E0                   fchs    ; -ST(0)
01A1:    DE C1                   faddp    st(1)
01A3:    D9 D0                   fnop
01A5:    DD 41 08                fld      qword ptr [ecx + 8]
01A8:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
01AB:    90                      nop
01AC:    90                      nop
01AD:    DD 01                   fld      qword ptr [ecx]
01AF:    90                      nop
01B0:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
01B3:    90                      nop
01B4:    90                      nop
01B5:    DE C1                   faddp    st(1)
01B7:    DD 02                   fld      qword ptr [edx]
01B9:    90                      nop
01BA:    DC 4E 28                fmul     qword ptr [esi + 0x28]
01BD:    90                      nop
01BE:    90                      nop
01BF:    DE C1                   faddp    st(1)
01C1:    DD 03                   fld      qword ptr [ebx]
01C3:    90                      nop
01C4:    DC 4E C8                fmul     qword ptr [esi - 0x38]
01C7:    90                      nop
01C8:    90                      nop
01C9:    DE C1                   faddp    st(1)
01CB:    D9 D0                   fnop
01CD:    90                      nop
01CE:    DD 59 08                fstp     qword ptr [ecx + 8]
01D1:    DD 19                   fstp     qword ptr [ecx]
01D3:    DD 1A                   fstp     qword ptr [edx]
01D5:    DD 1B                   fstp     qword ptr [ebx]
01D7:    D9 D0                   fnop
01D9:    8B C3                   mov      eax, ebx
01DB:    5F                      pop      edi
01DC:    5E                      pop      esi
01DD:    5B                      pop      ebx
01DE:    89 EC                   mov      esp, ebp
01E0:    5D                      pop      ebp
01E1:    C2 08 00                ret      8
