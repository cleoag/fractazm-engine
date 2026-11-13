; Disassembly: Quintic.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 424 bytes, decoded: 424 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 60 00 00 00       sub      esp, 0x60
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 03                   fld      qword ptr [ebx]
0018:    D8 C8                   fmul     st(0)
001A:    DD 55 F8                fst      qword ptr [ebp - 8]
001D:    DC 0B                   fmul     qword ptr [ebx]
001F:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0022:    DD 02                   fld      qword ptr [edx]
0024:    D8 C8                   fmul     st(0)
0026:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0029:    DC 0A                   fmul     qword ptr [edx]
002B:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
002E:    DD 01                   fld      qword ptr [ecx]
0030:    D8 C8                   fmul     st(0)
0032:    DD 55 E8                fst      qword ptr [ebp - 0x18]
0035:    DC 09                   fmul     qword ptr [ecx]
0037:    DD 5D B0                fstp     qword ptr [ebp - 0x50]
003A:    DD 03                   fld      qword ptr [ebx]
003C:    DC 0A                   fmul     qword ptr [edx]
003E:    DD 55 E0                fst      qword ptr [ebp - 0x20]
0041:    DC 09                   fmul     qword ptr [ecx]
0043:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0046:    DD 03                   fld      qword ptr [ebx]
0048:    DC 09                   fmul     qword ptr [ecx]
004A:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
004D:    DD 02                   fld      qword ptr [edx]
004F:    DC 09                   fmul     qword ptr [ecx]
0051:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0054:    D9 E8                   fld1    ; Load 1.0
0056:    83 7F D4 00             cmp      dword ptr [edi - 0x2c], 0
005A:    74 02                   je       0x5e    ; Conditional jump
005C:    D9 E0                   fchs    ; -ST(0)
005E:    DD 5D A8                fstp     qword ptr [ebp - 0x58]
0061:    D9 D0                   fnop
0063:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0066:    DC 4D F8                fmul     qword ptr [ebp - 8]
0069:    DD 45 F0                fld      qword ptr [ebp - 0x10]
006C:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
006F:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0072:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0075:    DE C1                   faddp    st(1)
0077:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
007A:    DC 0F                   fmul     qword ptr [edi]
007C:    DE C1                   faddp    st(1)
007E:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0081:    D8 C8                   fmul     st(0)
0083:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0086:    D8 C8                   fmul     st(0)
0088:    DE C1                   faddp    st(1)
008A:    DD 45 D0                fld      qword ptr [ebp - 0x30]
008D:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0090:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0093:    DE C1                   faddp    st(1)
0095:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0098:    D8 C8                   fmul     st(0)
009A:    90                      nop
009B:    DC 4F E0                fmul     qword ptr [edi - 0x20]
009E:    DE C1                   faddp    st(1)
00A0:    DD 45 D0                fld      qword ptr [ebp - 0x30]
00A3:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00A6:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00A9:    DE C1                   faddp    st(1)
00AB:    DC 0B                   fmul     qword ptr [ebx]
00AD:    DC 4F 08                fmul     qword ptr [edi + 8]
00B0:    DE C1                   faddp    st(1)
00B2:    DD 02                   fld      qword ptr [edx]
00B4:    DC 01                   fadd     qword ptr [ecx]
00B6:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
00B9:    DC 0B                   fmul     qword ptr [ebx]
00BB:    DC 4F D8                fmul     qword ptr [edi - 0x28]
00BE:    DE C1                   faddp    st(1)
00C0:    DC 4D A8                fmul     qword ptr [ebp - 0x58]
00C3:    DC 46 18                fadd     qword ptr [esi + 0x18]
00C6:    90                      nop
00C7:    90                      nop
00C8:    90                      nop
00C9:    DD 45 B8                fld      qword ptr [ebp - 0x48]
00CC:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00CF:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00D2:    DC 45 F8                fadd     qword ptr [ebp - 8]
00D5:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00D8:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00DB:    DE C1                   faddp    st(1)
00DD:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
00E0:    DC 0F                   fmul     qword ptr [edi]
00E2:    DE C1                   faddp    st(1)
00E4:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00E7:    D8 C8                   fmul     st(0)
00E9:    DD 45 F8                fld      qword ptr [ebp - 8]
00EC:    D8 C8                   fmul     st(0)
00EE:    DE C1                   faddp    st(1)
00F0:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00F3:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00F6:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00F9:    DE C1                   faddp    st(1)
00FB:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00FE:    D8 C8                   fmul     st(0)
0100:    90                      nop
0101:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0104:    DE C1                   faddp    st(1)
0106:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0109:    DC 4D F8                fmul     qword ptr [ebp - 8]
010C:    DC 4F E8                fmul     qword ptr [edi - 0x18]
010F:    DE C1                   faddp    st(1)
0111:    DC 0A                   fmul     qword ptr [edx]
0113:    DC 4F 08                fmul     qword ptr [edi + 8]
0116:    DE C1                   faddp    st(1)
0118:    DD 01                   fld      qword ptr [ecx]
011A:    DC 03                   fadd     qword ptr [ebx]
011C:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
011F:    DC 0A                   fmul     qword ptr [edx]
0121:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0124:    DE C1                   faddp    st(1)
0126:    DC 4D A8                fmul     qword ptr [ebp - 0x58]
0129:    DC 46 20                fadd     qword ptr [esi + 0x20]
012C:    90                      nop
012D:    90                      nop
012E:    90                      nop
012F:    DD 45 B0                fld      qword ptr [ebp - 0x50]
0132:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0135:    DD 45 F8                fld      qword ptr [ebp - 8]
0138:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
013B:    DD 45 E0                fld      qword ptr [ebp - 0x20]
013E:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0141:    DE C1                   faddp    st(1)
0143:    DC 4D B0                fmul     qword ptr [ebp - 0x50]
0146:    DC 0F                   fmul     qword ptr [edi]
0148:    DE C1                   faddp    st(1)
014A:    DD 45 F8                fld      qword ptr [ebp - 8]
014D:    D8 C8                   fmul     st(0)
014F:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0152:    D8 C8                   fmul     st(0)
0154:    DE C1                   faddp    st(1)
0156:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0159:    DC 4D F8                fmul     qword ptr [ebp - 8]
015C:    DC 4F E8                fmul     qword ptr [edi - 0x18]
015F:    DE C1                   faddp    st(1)
0161:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0164:    D8 C8                   fmul     st(0)
0166:    90                      nop
0167:    DC 4F E0                fmul     qword ptr [edi - 0x20]
016A:    DE C1                   faddp    st(1)
016C:    DD 45 E0                fld      qword ptr [ebp - 0x20]
016F:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0172:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0175:    DE C1                   faddp    st(1)
0177:    DC 09                   fmul     qword ptr [ecx]
0179:    DC 4F 08                fmul     qword ptr [edi + 8]
017C:    DE C1                   faddp    st(1)
017E:    DD 03                   fld      qword ptr [ebx]
0180:    DC 02                   fadd     qword ptr [edx]
0182:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0185:    DC 09                   fmul     qword ptr [ecx]
0187:    DC 4F D8                fmul     qword ptr [edi - 0x28]
018A:    DE C1                   faddp    st(1)
018C:    DC 4D A8                fmul     qword ptr [ebp - 0x58]
018F:    DC 46 28                fadd     qword ptr [esi + 0x28]
0192:    90                      nop
0193:    90                      nop
0194:    90                      nop
0195:    DD 19                   fstp     qword ptr [ecx]
0197:    DD 1A                   fstp     qword ptr [edx]
0199:    DD 1B                   fstp     qword ptr [ebx]
019B:    D9 D0                   fnop
019D:    8B C3                   mov      eax, ebx
019F:    5F                      pop      edi
01A0:    5E                      pop      esi
01A1:    5B                      pop      ebx
01A2:    89 EC                   mov      esp, ebp
01A4:    5D                      pop      ebp
01A5:    C2 08 00                ret      8
