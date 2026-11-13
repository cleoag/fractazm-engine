; Disassembly: TalisTpx.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 493 bytes, decoded: 493 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 48 00 00 00       sub      esp, 0x48
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    8B 47 BC                mov      eax, dword ptr [edi - 0x44]
0019:    83 F8 16                cmp      eax, 0x16
001C:    7F 05                   jg       0x23    ; Conditional jump
001E:    83 F8 01                cmp      eax, 1
0021:    7F 05                   jg       0x28    ; Conditional jump
0023:    B8 08 00 00 00          mov      eax, 8
0028:    D9 D0                   fnop
002A:    DD 03                   fld      qword ptr [ebx]
002C:    DD 02                   fld      qword ptr [edx]
002E:    DD 01                   fld      qword ptr [ecx]
0030:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0033:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0036:    DD 5D F8                fstp     qword ptr [ebp - 8]
0039:    48                      dec      eax
003A:    D9 D0                   fnop
003C:    DD 03                   fld      qword ptr [ebx]
003E:    D8 C8                   fmul     st(0)
0040:    DD 02                   fld      qword ptr [edx]
0042:    D8 C8                   fmul     st(0)
0044:    DE C1                   faddp    st(1)
0046:    D9 FA                   fsqrt    ; sqrt(ST(0))
0048:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
004B:    90                      nop
004C:    90                      nop
004D:    D9 D0                   fnop
004F:    DD 45 F8                fld      qword ptr [ebp - 8]
0052:    D8 C8                   fmul     st(0)
0054:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0057:    D8 C8                   fmul     st(0)
0059:    DE C1                   faddp    st(1)
005B:    D9 FA                   fsqrt    ; sqrt(ST(0))
005D:    DD 55 C0                fst      qword ptr [ebp - 0x40]
0060:    D9 D0                   fnop
0062:    DC 09                   fmul     qword ptr [ecx]
0064:    DD 45 C8                fld      qword ptr [ebp - 0x38]
0067:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
006A:    DE E9                   fsubp    st(1)
006C:    DC 4F F0                fmul     qword ptr [edi - 0x10]
006F:    D9 D0                   fnop
0071:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0074:    DC 09                   fmul     qword ptr [ecx]
0076:    DC 75 C8                fdiv     qword ptr [ebp - 0x38]
0079:    DC 75 C0                fdiv     qword ptr [ebp - 0x40]
007C:    D9 E0                   fchs    ; -ST(0)
007E:    D9 E8                   fld1    ; Load 1.0
0080:    DE C1                   faddp    st(1)
0082:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0085:    D9 D0                   fnop
0087:    DD 03                   fld      qword ptr [ebx]
0089:    DC 4D F8                fmul     qword ptr [ebp - 8]
008C:    DD 02                   fld      qword ptr [edx]
008E:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0091:    DE E9                   fsubp    st(1)
0093:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0096:    D9 D0                   fnop
0098:    DD 02                   fld      qword ptr [edx]
009A:    DC 4D F8                fmul     qword ptr [ebp - 8]
009D:    DD 03                   fld      qword ptr [ebx]
009F:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00A2:    DE C1                   faddp    st(1)
00A4:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
00A7:    D9 D0                   fnop
00A9:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00AC:    DD 5D F8                fstp     qword ptr [ebp - 8]
00AF:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00B2:    48                      dec      eax
00B3:    83 F8 00                cmp      eax, 0
00B6:    7F 95                   jg       0x4d    ; Conditional jump
00B8:    90                      nop
00B9:    90                      nop
00BA:    90                      nop
00BB:    90                      nop
00BC:    90                      nop
00BD:    90                      nop
00BE:    90                      nop
00BF:    DD 45 F8                fld      qword ptr [ebp - 8]
00C2:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00C5:    DD 03                   fld      qword ptr [ebx]
00C7:    DC 4F C8                fmul     qword ptr [edi - 0x38]
00CA:    DD 46 18                fld      qword ptr [esi + 0x18]
00CD:    DC 4F C0                fmul     qword ptr [edi - 0x40]
00D0:    DE C1                   faddp    st(1)
00D2:    DE C1                   faddp    st(1)
00D4:    DD 5D F8                fstp     qword ptr [ebp - 8]
00D7:    90                      nop
00D8:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00DB:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00DE:    DD 02                   fld      qword ptr [edx]
00E0:    DC 4F C8                fmul     qword ptr [edi - 0x38]
00E3:    DD 46 20                fld      qword ptr [esi + 0x20]
00E6:    DC 4F C0                fmul     qword ptr [edi - 0x40]
00E9:    DE C1                   faddp    st(1)
00EB:    DE C1                   faddp    st(1)
00ED:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00F0:    90                      nop
00F1:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00F4:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00F7:    DD 01                   fld      qword ptr [ecx]
00F9:    DC 4F C8                fmul     qword ptr [edi - 0x38]
00FC:    DD 46 28                fld      qword ptr [esi + 0x28]
00FF:    DC 4F C0                fmul     qword ptr [edi - 0x40]
0102:    DE C1                   faddp    st(1)
0104:    DE C1                   faddp    st(1)
0106:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0109:    90                      nop
010A:    DD 47 E8                fld      qword ptr [edi - 0x18]
010D:    DC 03                   fadd     qword ptr [ebx]
010F:    90                      nop
0110:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0113:    90                      nop
0114:    DD 47 E0                fld      qword ptr [edi - 0x20]
0117:    DC 02                   fadd     qword ptr [edx]
0119:    90                      nop
011A:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
011D:    90                      nop
011E:    DD 47 D8                fld      qword ptr [edi - 0x28]
0121:    DC 01                   fadd     qword ptr [ecx]
0123:    90                      nop
0124:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0127:    90                      nop
0128:    D9 D0                   fnop
012A:    DD 45 E0                fld      qword ptr [ebp - 0x20]
012D:    D8 C8                   fmul     st(0)
012F:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0132:    D8 C8                   fmul     st(0)
0134:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0137:    D8 C8                   fmul     st(0)
0139:    DE C1                   faddp    st(1)
013B:    DE C1                   faddp    st(1)
013D:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
0140:    90                      nop
0141:    DD 45 F8                fld      qword ptr [ebp - 8]
0144:    D8 C8                   fmul     st(0)
0146:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0149:    D8 C8                   fmul     st(0)
014B:    DE C1                   faddp    st(1)
014D:    D9 FA                   fsqrt    ; sqrt(ST(0))
014F:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0152:    90                      nop
0153:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0156:    D8 C8                   fmul     st(0)
0158:    DD 45 D8                fld      qword ptr [ebp - 0x28]
015B:    D8 C8                   fmul     st(0)
015D:    DE C1                   faddp    st(1)
015F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0161:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0164:    90                      nop
0165:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0168:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
016B:    DD 45 C8                fld      qword ptr [ebp - 0x38]
016E:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0171:    DE E9                   fsubp    st(1)
0173:    DC 75 B8                fdiv     qword ptr [ebp - 0x48]
0176:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0179:    DD 19                   fstp     qword ptr [ecx]
017B:    D9 D0                   fnop
017D:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0180:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0183:    DC 75 C0                fdiv     qword ptr [ebp - 0x40]
0186:    DC 75 C8                fdiv     qword ptr [ebp - 0x38]
0189:    D9 E8                   fld1    ; Load 1.0
018B:    DE C1                   faddp    st(1)
018D:    DC 75 B8                fdiv     qword ptr [ebp - 0x48]
0190:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
0193:    90                      nop
0194:    DD 45 F8                fld      qword ptr [ebp - 8]
0197:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
019A:    DD 45 F0                fld      qword ptr [ebp - 0x10]
019D:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
01A0:    DE C1                   faddp    st(1)
01A2:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
01A5:    DD 1B                   fstp     qword ptr [ebx]
01A7:    D9 D0                   fnop
01A9:    DD 45 F8                fld      qword ptr [ebp - 8]
01AC:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
01AF:    DD 45 E0                fld      qword ptr [ebp - 0x20]
01B2:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
01B5:    DE E1                   fsubrp   st(1)
01B7:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
01BA:    DD 1A                   fstp     qword ptr [edx]
01BC:    D9 D0                   fnop
01BE:    D9 E8                   fld1    ; Load 1.0
01C0:    DC 67 C0                fsub     qword ptr [edi - 0x40]
01C3:    DD 46 18                fld      qword ptr [esi + 0x18]
01C6:    D8 C9                   fmul     st(1)
01C8:    DC 03                   fadd     qword ptr [ebx]
01CA:    DD 1B                   fstp     qword ptr [ebx]
01CC:    DD 46 20                fld      qword ptr [esi + 0x20]
01CF:    D8 C9                   fmul     st(1)
01D1:    DC 02                   fadd     qword ptr [edx]
01D3:    DD 1A                   fstp     qword ptr [edx]
01D5:    DD 46 28                fld      qword ptr [esi + 0x28]
01D8:    DE C9                   fmulp    st(1)
01DA:    DC 01                   fadd     qword ptr [ecx]
01DC:    DD 19                   fstp     qword ptr [ecx]
01DE:    90                      nop
01DF:    90                      nop
01E0:    D9 D0                   fnop
01E2:    8B C3                   mov      eax, ebx
01E4:    5F                      pop      edi
01E5:    5E                      pop      esi
01E6:    5B                      pop      ebx
01E7:    8B E5                   mov      esp, ebp
01E9:    5D                      pop      ebp
01EA:    C2 08 00                ret      8
