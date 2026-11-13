; Disassembly: Makin4D p8b.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 426 bytes, decoded: 426 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    57                      push     edi
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    8B D8                   mov      ebx, eax
000E:    DD 03                   fld      qword ptr [ebx]
0010:    D8 C8                   fmul     st(0)
0012:    DD 02                   fld      qword ptr [edx]
0014:    D8 C8                   fmul     st(0)
0016:    DE C1                   faddp    st(1)
0018:    90                      nop
0019:    90                      nop
001A:    DD 01                   fld      qword ptr [ecx]
001C:    D8 C8                   fmul     st(0)
001E:    DD 41 08                fld      qword ptr [ecx + 8]
0021:    D8 C8                   fmul     st(0)
0023:    DE C1                   faddp    st(1)
0025:    90                      nop
0026:    90                      nop
0027:    DE C1                   faddp    st(1)
0029:    90                      nop
002A:    DD 03                   fld      qword ptr [ebx]
002C:    DC 09                   fmul     qword ptr [ecx]
002E:    DD 02                   fld      qword ptr [edx]
0030:    DC 49 08                fmul     qword ptr [ecx + 8]
0033:    DE E9                   fsubp    st(1)
0035:    D8 C0                   fadd     st(0)
0037:    90                      nop
0038:    DD 03                   fld      qword ptr [ebx]
003A:    DC 49 08                fmul     qword ptr [ecx + 8]
003D:    DD 02                   fld      qword ptr [edx]
003F:    DC 09                   fmul     qword ptr [ecx]
0041:    DE C1                   faddp    st(1)
0043:    D8 C0                   fadd     st(0)
0045:    90                      nop
0046:    DD 03                   fld      qword ptr [ebx]
0048:    D8 C8                   fmul     st(0)
004A:    DD 02                   fld      qword ptr [edx]
004C:    D8 C8                   fmul     st(0)
004E:    DE E9                   fsubp    st(1)
0050:    D8 C0                   fadd     st(0)
0052:    90                      nop
0053:    DD 03                   fld      qword ptr [ebx]
0055:    DC 0A                   fmul     qword ptr [edx]
0057:    D8 C0                   fadd     st(0)
0059:    D8 C0                   fadd     st(0)
005B:    DD 1A                   fstp     qword ptr [edx]
005D:    DD 1B                   fstp     qword ptr [ebx]
005F:    DD 03                   fld      qword ptr [ebx]
0061:    D8 C8                   fmul     st(0)
0063:    DD 02                   fld      qword ptr [edx]
0065:    D8 C8                   fmul     st(0)
0067:    DE C1                   faddp    st(1)
0069:    D9 FA                   fsqrt    ; sqrt(ST(0))
006B:    D8 FB                   fdivr    st(3)
006D:    D9 E0                   fchs    ; -ST(0)
006F:    D9 E8                   fld1    ; Load 1.0
0071:    DE C1                   faddp    st(1)
0073:    DD 03                   fld      qword ptr [ebx]
0075:    D8 C9                   fmul     st(1)
0077:    DD 1B                   fstp     qword ptr [ebx]
0079:    DD 02                   fld      qword ptr [edx]
007B:    DE C9                   fmulp    st(1)
007D:    DD 1A                   fstp     qword ptr [edx]
007F:    DD 59 08                fstp     qword ptr [ecx + 8]
0082:    DD 19                   fstp     qword ptr [ecx]
0084:    DD D8                   fstp     st(0)
0086:    D9 D0                   fnop
0088:    DD 03                   fld      qword ptr [ebx]
008A:    D8 C8                   fmul     st(0)
008C:    DD 02                   fld      qword ptr [edx]
008E:    D8 C8                   fmul     st(0)
0090:    DE C1                   faddp    st(1)
0092:    90                      nop
0093:    90                      nop
0094:    DD 01                   fld      qword ptr [ecx]
0096:    D8 C8                   fmul     st(0)
0098:    DD 41 08                fld      qword ptr [ecx + 8]
009B:    D8 C8                   fmul     st(0)
009D:    DE C1                   faddp    st(1)
009F:    90                      nop
00A0:    90                      nop
00A1:    DE C1                   faddp    st(1)
00A3:    90                      nop
00A4:    DD 03                   fld      qword ptr [ebx]
00A6:    DC 09                   fmul     qword ptr [ecx]
00A8:    DD 02                   fld      qword ptr [edx]
00AA:    DC 49 08                fmul     qword ptr [ecx + 8]
00AD:    DE E9                   fsubp    st(1)
00AF:    D8 C0                   fadd     st(0)
00B1:    90                      nop
00B2:    DD 03                   fld      qword ptr [ebx]
00B4:    DC 49 08                fmul     qword ptr [ecx + 8]
00B7:    DD 02                   fld      qword ptr [edx]
00B9:    DC 09                   fmul     qword ptr [ecx]
00BB:    DE C1                   faddp    st(1)
00BD:    D8 C0                   fadd     st(0)
00BF:    90                      nop
00C0:    DD 03                   fld      qword ptr [ebx]
00C2:    D8 C8                   fmul     st(0)
00C4:    DD 02                   fld      qword ptr [edx]
00C6:    D8 C8                   fmul     st(0)
00C8:    DE E9                   fsubp    st(1)
00CA:    D8 C0                   fadd     st(0)
00CC:    90                      nop
00CD:    DD 03                   fld      qword ptr [ebx]
00CF:    DC 0A                   fmul     qword ptr [edx]
00D1:    D8 C0                   fadd     st(0)
00D3:    D8 C0                   fadd     st(0)
00D5:    DD 1A                   fstp     qword ptr [edx]
00D7:    DD 1B                   fstp     qword ptr [ebx]
00D9:    DD 03                   fld      qword ptr [ebx]
00DB:    D8 C8                   fmul     st(0)
00DD:    DD 02                   fld      qword ptr [edx]
00DF:    D8 C8                   fmul     st(0)
00E1:    DE C1                   faddp    st(1)
00E3:    D9 FA                   fsqrt    ; sqrt(ST(0))
00E5:    D8 FB                   fdivr    st(3)
00E7:    D9 E0                   fchs    ; -ST(0)
00E9:    D9 E8                   fld1    ; Load 1.0
00EB:    DE C1                   faddp    st(1)
00ED:    DD 03                   fld      qword ptr [ebx]
00EF:    D8 C9                   fmul     st(1)
00F1:    DD 1B                   fstp     qword ptr [ebx]
00F3:    DD 02                   fld      qword ptr [edx]
00F5:    DE C9                   fmulp    st(1)
00F7:    DD 1A                   fstp     qword ptr [edx]
00F9:    DD 59 08                fstp     qword ptr [ecx + 8]
00FC:    DD 19                   fstp     qword ptr [ecx]
00FE:    DD D8                   fstp     st(0)
0100:    D9 D0                   fnop
0102:    DD 03                   fld      qword ptr [ebx]
0104:    D8 C8                   fmul     st(0)
0106:    DD 02                   fld      qword ptr [edx]
0108:    D8 C8                   fmul     st(0)
010A:    DE C1                   faddp    st(1)
010C:    90                      nop
010D:    90                      nop
010E:    DD 01                   fld      qword ptr [ecx]
0110:    D8 C8                   fmul     st(0)
0112:    DD 41 08                fld      qword ptr [ecx + 8]
0115:    D8 C8                   fmul     st(0)
0117:    DE C1                   faddp    st(1)
0119:    90                      nop
011A:    90                      nop
011B:    DE C1                   faddp    st(1)
011D:    90                      nop
011E:    DD 03                   fld      qword ptr [ebx]
0120:    DC 09                   fmul     qword ptr [ecx]
0122:    DD 02                   fld      qword ptr [edx]
0124:    DC 49 08                fmul     qword ptr [ecx + 8]
0127:    DE E9                   fsubp    st(1)
0129:    D8 C0                   fadd     st(0)
012B:    90                      nop
012C:    DD 03                   fld      qword ptr [ebx]
012E:    DC 49 08                fmul     qword ptr [ecx + 8]
0131:    DD 02                   fld      qword ptr [edx]
0133:    DC 09                   fmul     qword ptr [ecx]
0135:    DE C1                   faddp    st(1)
0137:    D8 C0                   fadd     st(0)
0139:    90                      nop
013A:    DD 03                   fld      qword ptr [ebx]
013C:    D8 C8                   fmul     st(0)
013E:    DD 02                   fld      qword ptr [edx]
0140:    D8 C8                   fmul     st(0)
0142:    DE E9                   fsubp    st(1)
0144:    D8 C0                   fadd     st(0)
0146:    90                      nop
0147:    DD 03                   fld      qword ptr [ebx]
0149:    DC 0A                   fmul     qword ptr [edx]
014B:    D8 C0                   fadd     st(0)
014D:    D8 C0                   fadd     st(0)
014F:    DD 1A                   fstp     qword ptr [edx]
0151:    DD 1B                   fstp     qword ptr [ebx]
0153:    DD 03                   fld      qword ptr [ebx]
0155:    D8 C8                   fmul     st(0)
0157:    DD 02                   fld      qword ptr [edx]
0159:    D8 C8                   fmul     st(0)
015B:    DE C1                   faddp    st(1)
015D:    D9 FA                   fsqrt    ; sqrt(ST(0))
015F:    D8 FB                   fdivr    st(3)
0161:    D9 E0                   fchs    ; -ST(0)
0163:    D9 E8                   fld1    ; Load 1.0
0165:    DE C1                   faddp    st(1)
0167:    DD 03                   fld      qword ptr [ebx]
0169:    D8 C9                   fmul     st(1)
016B:    DD 1B                   fstp     qword ptr [ebx]
016D:    DD 02                   fld      qword ptr [edx]
016F:    DE C9                   fmulp    st(1)
0171:    DD 1A                   fstp     qword ptr [edx]
0173:    DD 59 08                fstp     qword ptr [ecx + 8]
0176:    DD 19                   fstp     qword ptr [ecx]
0178:    DD D8                   fstp     st(0)
017A:    D9 D0                   fnop
017C:    DD 46 18                fld      qword ptr [esi + 0x18]
017F:    DC 03                   fadd     qword ptr [ebx]
0181:    DD 1B                   fstp     qword ptr [ebx]
0183:    DD 46 20                fld      qword ptr [esi + 0x20]
0186:    DC 02                   fadd     qword ptr [edx]
0188:    DD 1A                   fstp     qword ptr [edx]
018A:    DD 46 28                fld      qword ptr [esi + 0x28]
018D:    DC 01                   fadd     qword ptr [ecx]
018F:    DD 19                   fstp     qword ptr [ecx]
0191:    DD 46 C8                fld      qword ptr [esi - 0x38]
0194:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0197:    DC 41 08                fadd     qword ptr [ecx + 8]
019A:    DD 59 08                fstp     qword ptr [ecx + 8]
019D:    D9 D0                   fnop
019F:    8B C3                   mov      eax, ebx
01A1:    5F                      pop      edi
01A2:    5E                      pop      esi
01A3:    5B                      pop      ebx
01A4:    89 EC                   mov      esp, ebp
01A6:    5D                      pop      ebp
01A7:    C2 08 00                ret      8
