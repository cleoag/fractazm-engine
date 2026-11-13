; Disassembly: General.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 2003 bytes, decoded: 2003 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 50 00 00 00       sub      esp, 0x50
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    83 7F CC 00             cmp      dword ptr [edi - 0x34], 0
001A:    74 0A                   je       0x26    ; Conditional jump
001C:    90                      nop
001D:    90                      nop
001E:    90                      nop
001F:    90                      nop
0020:    DD 03                   fld      qword ptr [ebx]
0022:    D9 E1                   fabs    ; |ST(0)|
0024:    DD 1B                   fstp     qword ptr [ebx]
0026:    83 7F C8 00             cmp      dword ptr [edi - 0x38], 0
002A:    74 0A                   je       0x36    ; Conditional jump
002C:    90                      nop
002D:    90                      nop
002E:    90                      nop
002F:    90                      nop
0030:    DD 02                   fld      qword ptr [edx]
0032:    D9 E1                   fabs    ; |ST(0)|
0034:    DD 1A                   fstp     qword ptr [edx]
0036:    83 7F C4 00             cmp      dword ptr [edi - 0x3c], 0
003A:    74 0A                   je       0x46    ; Conditional jump
003C:    90                      nop
003D:    90                      nop
003E:    90                      nop
003F:    90                      nop
0040:    DD 01                   fld      qword ptr [ecx]
0042:    D9 E1                   fabs    ; |ST(0)|
0044:    DD 19                   fstp     qword ptr [ecx]
0046:    83 7F DC 00             cmp      dword ptr [edi - 0x24], 0
004A:    75 0B                   jne      0x57    ; Conditional jump
004C:    DD 03                   fld      qword ptr [ebx]
004E:    DD 02                   fld      qword ptr [edx]
0050:    E9 C1 02 00 00          jmp      0x316
0055:    90                      nop
0056:    90                      nop
0057:    83 7F DC 01             cmp      dword ptr [edi - 0x24], 1
005B:    75 0B                   jne      0x68    ; Conditional jump
005D:    DD 03                   fld      qword ptr [ebx]
005F:    DD 01                   fld      qword ptr [ecx]
0061:    E9 B0 02 00 00          jmp      0x316
0066:    90                      nop
0067:    90                      nop
0068:    83 7F DC 02             cmp      dword ptr [edi - 0x24], 2
006C:    75 0B                   jne      0x79    ; Conditional jump
006E:    DD 02                   fld      qword ptr [edx]
0070:    DD 03                   fld      qword ptr [ebx]
0072:    E9 9F 02 00 00          jmp      0x316
0077:    90                      nop
0078:    90                      nop
0079:    83 7F DC 03             cmp      dword ptr [edi - 0x24], 3
007D:    75 0B                   jne      0x8a    ; Conditional jump
007F:    DD 02                   fld      qword ptr [edx]
0081:    DD 01                   fld      qword ptr [ecx]
0083:    E9 8E 02 00 00          jmp      0x316
0088:    90                      nop
0089:    90                      nop
008A:    83 7F DC 04             cmp      dword ptr [edi - 0x24], 4
008E:    75 0B                   jne      0x9b    ; Conditional jump
0090:    DD 01                   fld      qword ptr [ecx]
0092:    DD 03                   fld      qword ptr [ebx]
0094:    E9 7D 02 00 00          jmp      0x316
0099:    90                      nop
009A:    90                      nop
009B:    83 7F DC 05             cmp      dword ptr [edi - 0x24], 5
009F:    75 0B                   jne      0xac    ; Conditional jump
00A1:    DD 01                   fld      qword ptr [ecx]
00A3:    DD 02                   fld      qword ptr [edx]
00A5:    E9 6C 02 00 00          jmp      0x316
00AA:    90                      nop
00AB:    90                      nop
00AC:    83 7F DC 06             cmp      dword ptr [edi - 0x24], 6
00B0:    75 15                   jne      0xc7    ; Conditional jump
00B2:    DD 03                   fld      qword ptr [ebx]
00B4:    DD 01                   fld      qword ptr [ecx]
00B6:    D8 C8                   fmul     st(0)
00B8:    DD 02                   fld      qword ptr [edx]
00BA:    D8 C8                   fmul     st(0)
00BC:    DE C1                   faddp    st(1)
00BE:    D9 FA                   fsqrt    ; sqrt(ST(0))
00C0:    E9 51 02 00 00          jmp      0x316
00C5:    90                      nop
00C6:    90                      nop
00C7:    83 7F DC 07             cmp      dword ptr [edi - 0x24], 7
00CB:    75 15                   jne      0xe2    ; Conditional jump
00CD:    DD 02                   fld      qword ptr [edx]
00CF:    DD 03                   fld      qword ptr [ebx]
00D1:    D8 C8                   fmul     st(0)
00D3:    DD 01                   fld      qword ptr [ecx]
00D5:    D8 C8                   fmul     st(0)
00D7:    DE C1                   faddp    st(1)
00D9:    D9 FA                   fsqrt    ; sqrt(ST(0))
00DB:    E9 36 02 00 00          jmp      0x316
00E0:    90                      nop
00E1:    90                      nop
00E2:    83 7F DC 08             cmp      dword ptr [edi - 0x24], 8
00E6:    75 15                   jne      0xfd    ; Conditional jump
00E8:    DD 01                   fld      qword ptr [ecx]
00EA:    DD 03                   fld      qword ptr [ebx]
00EC:    D8 C8                   fmul     st(0)
00EE:    DD 02                   fld      qword ptr [edx]
00F0:    D8 C8                   fmul     st(0)
00F2:    DE C1                   faddp    st(1)
00F4:    D9 FA                   fsqrt    ; sqrt(ST(0))
00F6:    E9 1B 02 00 00          jmp      0x316
00FB:    90                      nop
00FC:    90                      nop
00FD:    83 7F DC 09             cmp      dword ptr [edi - 0x24], 9
0101:    75 15                   jne      0x118    ; Conditional jump
0103:    DD 03                   fld      qword ptr [ebx]
0105:    DD 01                   fld      qword ptr [ecx]
0107:    D8 C8                   fmul     st(0)
0109:    DD 02                   fld      qword ptr [edx]
010B:    D8 C8                   fmul     st(0)
010D:    DE C1                   faddp    st(1)
010F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0111:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0113:    E9 FE 01 00 00          jmp      0x316
0118:    83 7F DC 0A             cmp      dword ptr [edi - 0x24], 0xa
011C:    75 15                   jne      0x133    ; Conditional jump
011E:    DD 02                   fld      qword ptr [edx]
0120:    DD 03                   fld      qword ptr [ebx]
0122:    D8 C8                   fmul     st(0)
0124:    DD 01                   fld      qword ptr [ecx]
0126:    D8 C8                   fmul     st(0)
0128:    DE C1                   faddp    st(1)
012A:    D9 FA                   fsqrt    ; sqrt(ST(0))
012C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
012E:    E9 E3 01 00 00          jmp      0x316
0133:    83 7F DC 0B             cmp      dword ptr [edi - 0x24], 0xb
0137:    75 15                   jne      0x14e    ; Conditional jump
0139:    DD 01                   fld      qword ptr [ecx]
013B:    DD 03                   fld      qword ptr [ebx]
013D:    D8 C8                   fmul     st(0)
013F:    DD 02                   fld      qword ptr [edx]
0141:    D8 C8                   fmul     st(0)
0143:    DE C1                   faddp    st(1)
0145:    D9 FA                   fsqrt    ; sqrt(ST(0))
0147:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0149:    E9 C8 01 00 00          jmp      0x316
014E:    83 7F DC 0C             cmp      dword ptr [edi - 0x24], 0xc
0152:    75 20                   jne      0x174    ; Conditional jump
0154:    DD 03                   fld      qword ptr [ebx]
0156:    DD 01                   fld      qword ptr [ecx]
0158:    D9 E1                   fabs    ; |ST(0)|
015A:    DD 02                   fld      qword ptr [edx]
015C:    D9 E1                   fabs    ; |ST(0)|
015E:    D8 D1                   fcom     st(1)
0160:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0162:    80 E4 41                and      ah, 0x41
0165:    77 04                   ja       0x16b    ; Conditional jump
0167:    DD D9                   fstp     st(1)
0169:    EB 02                   jmp      0x16d
016B:    DD D8                   fstp     st(0)
016D:    E9 A4 01 00 00          jmp      0x316
0172:    90                      nop
0173:    90                      nop
0174:    83 7F DC 0D             cmp      dword ptr [edi - 0x24], 0xd
0178:    75 20                   jne      0x19a    ; Conditional jump
017A:    DD 02                   fld      qword ptr [edx]
017C:    DD 01                   fld      qword ptr [ecx]
017E:    D9 E1                   fabs    ; |ST(0)|
0180:    DD 03                   fld      qword ptr [ebx]
0182:    D9 E1                   fabs    ; |ST(0)|
0184:    D8 D1                   fcom     st(1)
0186:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0188:    80 E4 41                and      ah, 0x41
018B:    77 04                   ja       0x191    ; Conditional jump
018D:    DD D9                   fstp     st(1)
018F:    EB 02                   jmp      0x193
0191:    DD D8                   fstp     st(0)
0193:    E9 7E 01 00 00          jmp      0x316
0198:    90                      nop
0199:    90                      nop
019A:    83 7F DC 0E             cmp      dword ptr [edi - 0x24], 0xe
019E:    75 20                   jne      0x1c0    ; Conditional jump
01A0:    DD 01                   fld      qword ptr [ecx]
01A2:    DD 03                   fld      qword ptr [ebx]
01A4:    D9 E1                   fabs    ; |ST(0)|
01A6:    DD 02                   fld      qword ptr [edx]
01A8:    D9 E1                   fabs    ; |ST(0)|
01AA:    D8 D1                   fcom     st(1)
01AC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01AE:    80 E4 41                and      ah, 0x41
01B1:    77 04                   ja       0x1b7    ; Conditional jump
01B3:    DD D9                   fstp     st(1)
01B5:    EB 02                   jmp      0x1b9
01B7:    DD D8                   fstp     st(0)
01B9:    E9 58 01 00 00          jmp      0x316
01BE:    90                      nop
01BF:    90                      nop
01C0:    83 7F DC 0F             cmp      dword ptr [edi - 0x24], 0xf
01C4:    75 20                   jne      0x1e6    ; Conditional jump
01C6:    DD 03                   fld      qword ptr [ebx]
01C8:    DD 01                   fld      qword ptr [ecx]
01CA:    D9 E1                   fabs    ; |ST(0)|
01CC:    DD 02                   fld      qword ptr [edx]
01CE:    D9 E1                   fabs    ; |ST(0)|
01D0:    D8 D1                   fcom     st(1)
01D2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01D4:    80 E4 41                and      ah, 0x41
01D7:    77 04                   ja       0x1dd    ; Conditional jump
01D9:    DD D9                   fstp     st(1)
01DB:    EB 02                   jmp      0x1df
01DD:    DD D8                   fstp     st(0)
01DF:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
01E1:    E9 30 01 00 00          jmp      0x316
01E6:    83 7F DC 10             cmp      dword ptr [edi - 0x24], 0x10
01EA:    75 20                   jne      0x20c    ; Conditional jump
01EC:    DD 02                   fld      qword ptr [edx]
01EE:    DD 01                   fld      qword ptr [ecx]
01F0:    D9 E1                   fabs    ; |ST(0)|
01F2:    DD 03                   fld      qword ptr [ebx]
01F4:    D9 E1                   fabs    ; |ST(0)|
01F6:    D8 D1                   fcom     st(1)
01F8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01FA:    80 E4 41                and      ah, 0x41
01FD:    77 04                   ja       0x203    ; Conditional jump
01FF:    DD D9                   fstp     st(1)
0201:    EB 02                   jmp      0x205
0203:    DD D8                   fstp     st(0)
0205:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0207:    E9 0A 01 00 00          jmp      0x316
020C:    83 7F DC 11             cmp      dword ptr [edi - 0x24], 0x11
0210:    75 20                   jne      0x232    ; Conditional jump
0212:    DD 01                   fld      qword ptr [ecx]
0214:    DD 03                   fld      qword ptr [ebx]
0216:    D9 E1                   fabs    ; |ST(0)|
0218:    DD 02                   fld      qword ptr [edx]
021A:    D9 E1                   fabs    ; |ST(0)|
021C:    D8 D1                   fcom     st(1)
021E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0220:    80 E4 41                and      ah, 0x41
0223:    77 04                   ja       0x229    ; Conditional jump
0225:    DD D9                   fstp     st(1)
0227:    EB 02                   jmp      0x22b
0229:    DD D8                   fstp     st(0)
022B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
022D:    E9 E4 00 00 00          jmp      0x316
0232:    83 7F DC 12             cmp      dword ptr [edi - 0x24], 0x12
0236:    75 20                   jne      0x258    ; Conditional jump
0238:    DD 03                   fld      qword ptr [ebx]
023A:    DD 01                   fld      qword ptr [ecx]
023C:    D9 E1                   fabs    ; |ST(0)|
023E:    DD 02                   fld      qword ptr [edx]
0240:    D9 E1                   fabs    ; |ST(0)|
0242:    D8 D1                   fcom     st(1)
0244:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0246:    80 E4 41                and      ah, 0x41
0249:    72 04                   jb       0x24f    ; Conditional jump
024B:    DD D9                   fstp     st(1)
024D:    EB 02                   jmp      0x251
024F:    DD D8                   fstp     st(0)
0251:    E9 C0 00 00 00          jmp      0x316
0256:    90                      nop
0257:    90                      nop
0258:    83 7F DC 13             cmp      dword ptr [edi - 0x24], 0x13
025C:    75 20                   jne      0x27e    ; Conditional jump
025E:    DD 02                   fld      qword ptr [edx]
0260:    DD 01                   fld      qword ptr [ecx]
0262:    D9 E1                   fabs    ; |ST(0)|
0264:    DD 03                   fld      qword ptr [ebx]
0266:    D9 E1                   fabs    ; |ST(0)|
0268:    D8 D1                   fcom     st(1)
026A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
026C:    80 E4 41                and      ah, 0x41
026F:    72 04                   jb       0x275    ; Conditional jump
0271:    DD D9                   fstp     st(1)
0273:    EB 02                   jmp      0x277
0275:    DD D8                   fstp     st(0)
0277:    E9 9A 00 00 00          jmp      0x316
027C:    90                      nop
027D:    90                      nop
027E:    83 7F DC 14             cmp      dword ptr [edi - 0x24], 0x14
0282:    75 20                   jne      0x2a4    ; Conditional jump
0284:    DD 01                   fld      qword ptr [ecx]
0286:    DD 03                   fld      qword ptr [ebx]
0288:    D9 E1                   fabs    ; |ST(0)|
028A:    DD 02                   fld      qword ptr [edx]
028C:    D9 E1                   fabs    ; |ST(0)|
028E:    D8 D1                   fcom     st(1)
0290:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0292:    80 E4 41                and      ah, 0x41
0295:    72 04                   jb       0x29b    ; Conditional jump
0297:    DD D9                   fstp     st(1)
0299:    EB 02                   jmp      0x29d
029B:    DD D8                   fstp     st(0)
029D:    E9 74 00 00 00          jmp      0x316
02A2:    90                      nop
02A3:    90                      nop
02A4:    83 7F DC 15             cmp      dword ptr [edi - 0x24], 0x15
02A8:    75 20                   jne      0x2ca    ; Conditional jump
02AA:    DD 03                   fld      qword ptr [ebx]
02AC:    DD 01                   fld      qword ptr [ecx]
02AE:    D9 E1                   fabs    ; |ST(0)|
02B0:    DD 02                   fld      qword ptr [edx]
02B2:    D9 E1                   fabs    ; |ST(0)|
02B4:    D8 D1                   fcom     st(1)
02B6:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
02B8:    80 E4 41                and      ah, 0x41
02BB:    72 04                   jb       0x2c1    ; Conditional jump
02BD:    DD D9                   fstp     st(1)
02BF:    EB 02                   jmp      0x2c3
02C1:    DD D8                   fstp     st(0)
02C3:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
02C5:    E9 4C 00 00 00          jmp      0x316
02CA:    83 7F DC 16             cmp      dword ptr [edi - 0x24], 0x16
02CE:    75 20                   jne      0x2f0    ; Conditional jump
02D0:    DD 02                   fld      qword ptr [edx]
02D2:    DD 01                   fld      qword ptr [ecx]
02D4:    D9 E1                   fabs    ; |ST(0)|
02D6:    DD 03                   fld      qword ptr [ebx]
02D8:    D9 E1                   fabs    ; |ST(0)|
02DA:    D8 D1                   fcom     st(1)
02DC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
02DE:    80 E4 41                and      ah, 0x41
02E1:    72 04                   jb       0x2e7    ; Conditional jump
02E3:    DD D9                   fstp     st(1)
02E5:    EB 02                   jmp      0x2e9
02E7:    DD D8                   fstp     st(0)
02E9:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
02EB:    E9 26 00 00 00          jmp      0x316
02F0:    83 7F DC 17             cmp      dword ptr [edi - 0x24], 0x17
02F4:    75 20                   jne      0x316    ; Conditional jump
02F6:    DD 01                   fld      qword ptr [ecx]
02F8:    DD 03                   fld      qword ptr [ebx]
02FA:    D9 E1                   fabs    ; |ST(0)|
02FC:    DD 02                   fld      qword ptr [edx]
02FE:    D9 E1                   fabs    ; |ST(0)|
0300:    D8 D1                   fcom     st(1)
0302:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0304:    80 E4 41                and      ah, 0x41
0307:    72 04                   jb       0x30d    ; Conditional jump
0309:    DD D9                   fstp     st(1)
030B:    EB 02                   jmp      0x30f
030D:    DD D8                   fstp     st(0)
030F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0311:    90                      nop
0312:    90                      nop
0313:    90                      nop
0314:    90                      nop
0315:    90                      nop
0316:    DD 47 F0                fld      qword ptr [edi - 0x10]
0319:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
031C:    83 7F D8 01             cmp      dword ptr [edi - 0x28], 1
0320:    75 02                   jne      0x324    ; Conditional jump
0322:    EB 06                   jmp      0x32a
0324:    83 7F D8 03             cmp      dword ptr [edi - 0x28], 3
0328:    75 17                   jne      0x341    ; Conditional jump
032A:    D9 C0                   fld      st(0)
032C:    D9 EE                   fldz    ; Load 0.0
032E:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0330:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0332:    80 E4 41                and      ah, 0x41
0335:    72 08                   jb       0x33f    ; Conditional jump
0337:    DD 45 F0                fld      qword ptr [ebp - 0x10]
033A:    D9 E0                   fchs    ; -ST(0)
033C:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
033F:    D9 E1                   fabs    ; |ST(0)|
0341:    90                      nop
0342:    90                      nop
0343:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0345:    90                      nop
0346:    90                      nop
0347:    83 7F D8 02             cmp      dword ptr [edi - 0x28], 2
034B:    75 02                   jne      0x34f    ; Conditional jump
034D:    EB 06                   jmp      0x355
034F:    83 7F D8 03             cmp      dword ptr [edi - 0x28], 3
0353:    75 12                   jne      0x367    ; Conditional jump
0355:    DD 07                   fld      qword ptr [edi]
0357:    D8 C1                   fadd     st(1)
0359:    D9 E1                   fabs    ; |ST(0)|
035B:    DD 07                   fld      qword ptr [edi]
035D:    D8 E2                   fsub     st(2)
035F:    D9 E1                   fabs    ; |ST(0)|
0361:    DE E9                   fsubp    st(1)
0363:    DE E9                   fsubp    st(1)
0365:    90                      nop
0366:    90                      nop
0367:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
036A:    D9 FB                   fsincos    ; ST(0)=cos, push sin
036C:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
036F:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0372:    90                      nop
0373:    83 7F D4 00             cmp      dword ptr [edi - 0x2c], 0
0377:    75 0B                   jne      0x384    ; Conditional jump
0379:    DD 03                   fld      qword ptr [ebx]
037B:    DD 02                   fld      qword ptr [edx]
037D:    E9 C1 02 00 00          jmp      0x643
0382:    90                      nop
0383:    90                      nop
0384:    83 7F D4 01             cmp      dword ptr [edi - 0x2c], 1
0388:    75 0B                   jne      0x395    ; Conditional jump
038A:    DD 03                   fld      qword ptr [ebx]
038C:    DD 01                   fld      qword ptr [ecx]
038E:    E9 B0 02 00 00          jmp      0x643
0393:    90                      nop
0394:    90                      nop
0395:    83 7F D4 02             cmp      dword ptr [edi - 0x2c], 2
0399:    75 0B                   jne      0x3a6    ; Conditional jump
039B:    DD 02                   fld      qword ptr [edx]
039D:    DD 03                   fld      qword ptr [ebx]
039F:    E9 9F 02 00 00          jmp      0x643
03A4:    90                      nop
03A5:    90                      nop
03A6:    83 7F D4 03             cmp      dword ptr [edi - 0x2c], 3
03AA:    75 0B                   jne      0x3b7    ; Conditional jump
03AC:    DD 02                   fld      qword ptr [edx]
03AE:    DD 01                   fld      qword ptr [ecx]
03B0:    E9 8E 02 00 00          jmp      0x643
03B5:    90                      nop
03B6:    90                      nop
03B7:    83 7F D4 04             cmp      dword ptr [edi - 0x2c], 4
03BB:    75 0B                   jne      0x3c8    ; Conditional jump
03BD:    DD 01                   fld      qword ptr [ecx]
03BF:    DD 03                   fld      qword ptr [ebx]
03C1:    E9 7D 02 00 00          jmp      0x643
03C6:    90                      nop
03C7:    90                      nop
03C8:    83 7F D4 05             cmp      dword ptr [edi - 0x2c], 5
03CC:    75 0B                   jne      0x3d9    ; Conditional jump
03CE:    DD 01                   fld      qword ptr [ecx]
03D0:    DD 02                   fld      qword ptr [edx]
03D2:    E9 6C 02 00 00          jmp      0x643
03D7:    90                      nop
03D8:    90                      nop
03D9:    83 7F D4 06             cmp      dword ptr [edi - 0x2c], 6
03DD:    75 15                   jne      0x3f4    ; Conditional jump
03DF:    DD 03                   fld      qword ptr [ebx]
03E1:    DD 01                   fld      qword ptr [ecx]
03E3:    D8 C8                   fmul     st(0)
03E5:    DD 02                   fld      qword ptr [edx]
03E7:    D8 C8                   fmul     st(0)
03E9:    DE C1                   faddp    st(1)
03EB:    D9 FA                   fsqrt    ; sqrt(ST(0))
03ED:    E9 51 02 00 00          jmp      0x643
03F2:    90                      nop
03F3:    90                      nop
03F4:    83 7F D4 07             cmp      dword ptr [edi - 0x2c], 7
03F8:    75 15                   jne      0x40f    ; Conditional jump
03FA:    DD 02                   fld      qword ptr [edx]
03FC:    DD 03                   fld      qword ptr [ebx]
03FE:    D8 C8                   fmul     st(0)
0400:    DD 01                   fld      qword ptr [ecx]
0402:    D8 C8                   fmul     st(0)
0404:    DE C1                   faddp    st(1)
0406:    D9 FA                   fsqrt    ; sqrt(ST(0))
0408:    E9 36 02 00 00          jmp      0x643
040D:    90                      nop
040E:    90                      nop
040F:    83 7F D4 08             cmp      dword ptr [edi - 0x2c], 8
0413:    75 15                   jne      0x42a    ; Conditional jump
0415:    DD 01                   fld      qword ptr [ecx]
0417:    DD 03                   fld      qword ptr [ebx]
0419:    D8 C8                   fmul     st(0)
041B:    DD 02                   fld      qword ptr [edx]
041D:    D8 C8                   fmul     st(0)
041F:    DE C1                   faddp    st(1)
0421:    D9 FA                   fsqrt    ; sqrt(ST(0))
0423:    E9 1B 02 00 00          jmp      0x643
0428:    90                      nop
0429:    90                      nop
042A:    83 7F D4 09             cmp      dword ptr [edi - 0x2c], 9
042E:    75 15                   jne      0x445    ; Conditional jump
0430:    DD 03                   fld      qword ptr [ebx]
0432:    DD 01                   fld      qword ptr [ecx]
0434:    D8 C8                   fmul     st(0)
0436:    DD 02                   fld      qword ptr [edx]
0438:    D8 C8                   fmul     st(0)
043A:    DE C1                   faddp    st(1)
043C:    D9 FA                   fsqrt    ; sqrt(ST(0))
043E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0440:    E9 FE 01 00 00          jmp      0x643
0445:    83 7F D4 0A             cmp      dword ptr [edi - 0x2c], 0xa
0449:    75 15                   jne      0x460    ; Conditional jump
044B:    DD 02                   fld      qword ptr [edx]
044D:    DD 03                   fld      qword ptr [ebx]
044F:    D8 C8                   fmul     st(0)
0451:    DD 01                   fld      qword ptr [ecx]
0453:    D8 C8                   fmul     st(0)
0455:    DE C1                   faddp    st(1)
0457:    D9 FA                   fsqrt    ; sqrt(ST(0))
0459:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
045B:    E9 E3 01 00 00          jmp      0x643
0460:    83 7F D4 0B             cmp      dword ptr [edi - 0x2c], 0xb
0464:    75 15                   jne      0x47b    ; Conditional jump
0466:    DD 01                   fld      qword ptr [ecx]
0468:    DD 03                   fld      qword ptr [ebx]
046A:    D8 C8                   fmul     st(0)
046C:    DD 02                   fld      qword ptr [edx]
046E:    D8 C8                   fmul     st(0)
0470:    DE C1                   faddp    st(1)
0472:    D9 FA                   fsqrt    ; sqrt(ST(0))
0474:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0476:    E9 C8 01 00 00          jmp      0x643
047B:    83 7F D4 0C             cmp      dword ptr [edi - 0x2c], 0xc
047F:    75 20                   jne      0x4a1    ; Conditional jump
0481:    DD 03                   fld      qword ptr [ebx]
0483:    DD 01                   fld      qword ptr [ecx]
0485:    D9 E1                   fabs    ; |ST(0)|
0487:    DD 02                   fld      qword ptr [edx]
0489:    D9 E1                   fabs    ; |ST(0)|
048B:    D8 D1                   fcom     st(1)
048D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
048F:    80 E4 41                and      ah, 0x41
0492:    77 04                   ja       0x498    ; Conditional jump
0494:    DD D9                   fstp     st(1)
0496:    EB 02                   jmp      0x49a
0498:    DD D8                   fstp     st(0)
049A:    E9 A4 01 00 00          jmp      0x643
049F:    90                      nop
04A0:    90                      nop
04A1:    83 7F D4 0D             cmp      dword ptr [edi - 0x2c], 0xd
04A5:    75 20                   jne      0x4c7    ; Conditional jump
04A7:    DD 02                   fld      qword ptr [edx]
04A9:    DD 01                   fld      qword ptr [ecx]
04AB:    D9 E1                   fabs    ; |ST(0)|
04AD:    DD 03                   fld      qword ptr [ebx]
04AF:    D9 E1                   fabs    ; |ST(0)|
04B1:    D8 D1                   fcom     st(1)
04B3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
04B5:    80 E4 41                and      ah, 0x41
04B8:    77 04                   ja       0x4be    ; Conditional jump
04BA:    DD D9                   fstp     st(1)
04BC:    EB 02                   jmp      0x4c0
04BE:    DD D8                   fstp     st(0)
04C0:    E9 7E 01 00 00          jmp      0x643
04C5:    90                      nop
04C6:    90                      nop
04C7:    83 7F D4 0E             cmp      dword ptr [edi - 0x2c], 0xe
04CB:    75 20                   jne      0x4ed    ; Conditional jump
04CD:    DD 01                   fld      qword ptr [ecx]
04CF:    DD 03                   fld      qword ptr [ebx]
04D1:    D9 E1                   fabs    ; |ST(0)|
04D3:    DD 02                   fld      qword ptr [edx]
04D5:    D9 E1                   fabs    ; |ST(0)|
04D7:    D8 D1                   fcom     st(1)
04D9:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
04DB:    80 E4 41                and      ah, 0x41
04DE:    77 04                   ja       0x4e4    ; Conditional jump
04E0:    DD D9                   fstp     st(1)
04E2:    EB 02                   jmp      0x4e6
04E4:    DD D8                   fstp     st(0)
04E6:    E9 58 01 00 00          jmp      0x643
04EB:    90                      nop
04EC:    90                      nop
04ED:    83 7F D4 0F             cmp      dword ptr [edi - 0x2c], 0xf
04F1:    75 20                   jne      0x513    ; Conditional jump
04F3:    DD 03                   fld      qword ptr [ebx]
04F5:    DD 01                   fld      qword ptr [ecx]
04F7:    D9 E1                   fabs    ; |ST(0)|
04F9:    DD 02                   fld      qword ptr [edx]
04FB:    D9 E1                   fabs    ; |ST(0)|
04FD:    D8 D1                   fcom     st(1)
04FF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0501:    80 E4 41                and      ah, 0x41
0504:    77 04                   ja       0x50a    ; Conditional jump
0506:    DD D9                   fstp     st(1)
0508:    EB 02                   jmp      0x50c
050A:    DD D8                   fstp     st(0)
050C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
050E:    E9 30 01 00 00          jmp      0x643
0513:    83 7F D4 10             cmp      dword ptr [edi - 0x2c], 0x10
0517:    75 20                   jne      0x539    ; Conditional jump
0519:    DD 02                   fld      qword ptr [edx]
051B:    DD 01                   fld      qword ptr [ecx]
051D:    D9 E1                   fabs    ; |ST(0)|
051F:    DD 03                   fld      qword ptr [ebx]
0521:    D9 E1                   fabs    ; |ST(0)|
0523:    D8 D1                   fcom     st(1)
0525:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0527:    80 E4 41                and      ah, 0x41
052A:    77 04                   ja       0x530    ; Conditional jump
052C:    DD D9                   fstp     st(1)
052E:    EB 02                   jmp      0x532
0530:    DD D8                   fstp     st(0)
0532:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0534:    E9 0A 01 00 00          jmp      0x643
0539:    83 7F D4 11             cmp      dword ptr [edi - 0x2c], 0x11
053D:    75 20                   jne      0x55f    ; Conditional jump
053F:    DD 01                   fld      qword ptr [ecx]
0541:    DD 03                   fld      qword ptr [ebx]
0543:    D9 E1                   fabs    ; |ST(0)|
0545:    DD 02                   fld      qword ptr [edx]
0547:    D9 E1                   fabs    ; |ST(0)|
0549:    D8 D1                   fcom     st(1)
054B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
054D:    80 E4 41                and      ah, 0x41
0550:    77 04                   ja       0x556    ; Conditional jump
0552:    DD D9                   fstp     st(1)
0554:    EB 02                   jmp      0x558
0556:    DD D8                   fstp     st(0)
0558:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
055A:    E9 E4 00 00 00          jmp      0x643
055F:    83 7F D4 12             cmp      dword ptr [edi - 0x2c], 0x12
0563:    75 20                   jne      0x585    ; Conditional jump
0565:    DD 03                   fld      qword ptr [ebx]
0567:    DD 01                   fld      qword ptr [ecx]
0569:    D9 E1                   fabs    ; |ST(0)|
056B:    DD 02                   fld      qword ptr [edx]
056D:    D9 E1                   fabs    ; |ST(0)|
056F:    D8 D1                   fcom     st(1)
0571:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0573:    80 E4 41                and      ah, 0x41
0576:    72 04                   jb       0x57c    ; Conditional jump
0578:    DD D9                   fstp     st(1)
057A:    EB 02                   jmp      0x57e
057C:    DD D8                   fstp     st(0)
057E:    E9 C0 00 00 00          jmp      0x643
0583:    90                      nop
0584:    90                      nop
0585:    83 7F D4 13             cmp      dword ptr [edi - 0x2c], 0x13
0589:    75 20                   jne      0x5ab    ; Conditional jump
058B:    DD 02                   fld      qword ptr [edx]
058D:    DD 01                   fld      qword ptr [ecx]
058F:    D9 E1                   fabs    ; |ST(0)|
0591:    DD 03                   fld      qword ptr [ebx]
0593:    D9 E1                   fabs    ; |ST(0)|
0595:    D8 D1                   fcom     st(1)
0597:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0599:    80 E4 41                and      ah, 0x41
059C:    72 04                   jb       0x5a2    ; Conditional jump
059E:    DD D9                   fstp     st(1)
05A0:    EB 02                   jmp      0x5a4
05A2:    DD D8                   fstp     st(0)
05A4:    E9 9A 00 00 00          jmp      0x643
05A9:    90                      nop
05AA:    90                      nop
05AB:    83 7F D4 14             cmp      dword ptr [edi - 0x2c], 0x14
05AF:    75 20                   jne      0x5d1    ; Conditional jump
05B1:    DD 01                   fld      qword ptr [ecx]
05B3:    DD 03                   fld      qword ptr [ebx]
05B5:    D9 E1                   fabs    ; |ST(0)|
05B7:    DD 02                   fld      qword ptr [edx]
05B9:    D9 E1                   fabs    ; |ST(0)|
05BB:    D8 D1                   fcom     st(1)
05BD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
05BF:    80 E4 41                and      ah, 0x41
05C2:    72 04                   jb       0x5c8    ; Conditional jump
05C4:    DD D9                   fstp     st(1)
05C6:    EB 02                   jmp      0x5ca
05C8:    DD D8                   fstp     st(0)
05CA:    E9 74 00 00 00          jmp      0x643
05CF:    90                      nop
05D0:    90                      nop
05D1:    83 7F D4 15             cmp      dword ptr [edi - 0x2c], 0x15
05D5:    75 20                   jne      0x5f7    ; Conditional jump
05D7:    DD 03                   fld      qword ptr [ebx]
05D9:    DD 01                   fld      qword ptr [ecx]
05DB:    D9 E1                   fabs    ; |ST(0)|
05DD:    DD 02                   fld      qword ptr [edx]
05DF:    D9 E1                   fabs    ; |ST(0)|
05E1:    D8 D1                   fcom     st(1)
05E3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
05E5:    80 E4 41                and      ah, 0x41
05E8:    72 04                   jb       0x5ee    ; Conditional jump
05EA:    DD D9                   fstp     st(1)
05EC:    EB 02                   jmp      0x5f0
05EE:    DD D8                   fstp     st(0)
05F0:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
05F2:    E9 4C 00 00 00          jmp      0x643
05F7:    83 7F D4 16             cmp      dword ptr [edi - 0x2c], 0x16
05FB:    75 20                   jne      0x61d    ; Conditional jump
05FD:    DD 02                   fld      qword ptr [edx]
05FF:    DD 01                   fld      qword ptr [ecx]
0601:    D9 E1                   fabs    ; |ST(0)|
0603:    DD 03                   fld      qword ptr [ebx]
0605:    D9 E1                   fabs    ; |ST(0)|
0607:    D8 D1                   fcom     st(1)
0609:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
060B:    80 E4 41                and      ah, 0x41
060E:    72 04                   jb       0x614    ; Conditional jump
0610:    DD D9                   fstp     st(1)
0612:    EB 02                   jmp      0x616
0614:    DD D8                   fstp     st(0)
0616:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0618:    E9 26 00 00 00          jmp      0x643
061D:    83 7F D4 17             cmp      dword ptr [edi - 0x2c], 0x17
0621:    75 20                   jne      0x643    ; Conditional jump
0623:    DD 01                   fld      qword ptr [ecx]
0625:    DD 03                   fld      qword ptr [ebx]
0627:    D9 E1                   fabs    ; |ST(0)|
0629:    DD 02                   fld      qword ptr [edx]
062B:    D9 E1                   fabs    ; |ST(0)|
062D:    D8 D1                   fcom     st(1)
062F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0631:    80 E4 41                and      ah, 0x41
0634:    72 04                   jb       0x63a    ; Conditional jump
0636:    DD D9                   fstp     st(1)
0638:    EB 02                   jmp      0x63c
063A:    DD D8                   fstp     st(0)
063C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
063E:    90                      nop
063F:    90                      nop
0640:    90                      nop
0641:    90                      nop
0642:    90                      nop
0643:    DD 47 E8                fld      qword ptr [edi - 0x18]
0646:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0649:    83 7F D0 01             cmp      dword ptr [edi - 0x30], 1
064D:    75 02                   jne      0x651    ; Conditional jump
064F:    EB 06                   jmp      0x657
0651:    83 7F D0 03             cmp      dword ptr [edi - 0x30], 3
0655:    75 17                   jne      0x66e    ; Conditional jump
0657:    D9 C0                   fld      st(0)
0659:    D9 EE                   fldz    ; Load 0.0
065B:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
065D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
065F:    80 E4 41                and      ah, 0x41
0662:    72 08                   jb       0x66c    ; Conditional jump
0664:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0667:    D9 E0                   fchs    ; -ST(0)
0669:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
066C:    D9 E1                   fabs    ; |ST(0)|
066E:    90                      nop
066F:    90                      nop
0670:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0672:    83 7F D0 02             cmp      dword ptr [edi - 0x30], 2
0676:    75 02                   jne      0x67a    ; Conditional jump
0678:    EB 06                   jmp      0x680
067A:    83 7F D0 03             cmp      dword ptr [edi - 0x30], 3
067E:    75 12                   jne      0x692    ; Conditional jump
0680:    DD 07                   fld      qword ptr [edi]
0682:    D8 C1                   fadd     st(1)
0684:    D9 E1                   fabs    ; |ST(0)|
0686:    DD 07                   fld      qword ptr [edi]
0688:    D8 E2                   fsub     st(2)
068A:    D9 E1                   fabs    ; |ST(0)|
068C:    DE E9                   fsubp    st(1)
068E:    DE E9                   fsubp    st(1)
0690:    90                      nop
0691:    90                      nop
0692:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0695:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0697:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
069A:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
069D:    D9 D0                   fnop
069F:    D9 D0                   fnop
06A1:    DD 03                   fld      qword ptr [ebx]
06A3:    D8 C8                   fmul     st(0)
06A5:    DD 02                   fld      qword ptr [edx]
06A7:    D8 C8                   fmul     st(0)
06A9:    DD 01                   fld      qword ptr [ecx]
06AB:    D8 C8                   fmul     st(0)
06AD:    DE C1                   faddp    st(1)
06AF:    DE C1                   faddp    st(1)
06B1:    D9 FA                   fsqrt    ; sqrt(ST(0))
06B3:    DD 55 E8                fst      qword ptr [ebp - 0x18]
06B6:    DD 47 F0                fld      qword ptr [edi - 0x10]
06B9:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
06BB:    D9 E1                   fabs    ; |ST(0)|
06BD:    D9 F1                   fyl2x
06BF:    D9 E8                   fld1    ; Load 1.0
06C1:    D9 C1                   fld      st(1)
06C3:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
06C5:    D9 F0                   f2xm1
06C7:    DE C1                   faddp    st(1)
06C9:    D9 FD                   fscale
06CB:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
06CD:    DD D8                   fstp     st(0)
06CF:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
06D2:    D9 D0                   fnop
06D4:    83 7F C0 00             cmp      dword ptr [edi - 0x40], 0
06D8:    75 4F                   jne      0x729    ; Conditional jump
06DA:    DD 45 D0                fld      qword ptr [ebp - 0x30]
06DD:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
06E0:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
06E3:    DD 46 18                fld      qword ptr [esi + 0x18]
06E6:    83 7F CC FF             cmp      dword ptr [edi - 0x34], -1
06EA:    75 02                   jne      0x6ee    ; Conditional jump
06EC:    D9 E1                   fabs    ; |ST(0)|
06EE:    DE C1                   faddp    st(1)
06F0:    DD 1B                   fstp     qword ptr [ebx]
06F2:    DD 45 D8                fld      qword ptr [ebp - 0x28]
06F5:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
06F8:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
06FB:    DD 46 20                fld      qword ptr [esi + 0x20]
06FE:    83 7F C8 FF             cmp      dword ptr [edi - 0x38], -1
0702:    75 02                   jne      0x706    ; Conditional jump
0704:    D9 E1                   fabs    ; |ST(0)|
0706:    DE C1                   faddp    st(1)
0708:    DD 1A                   fstp     qword ptr [edx]
070A:    DD 47 E0                fld      qword ptr [edi - 0x20]
070D:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0710:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0713:    DD 46 28                fld      qword ptr [esi + 0x28]
0716:    83 7F C4 FF             cmp      dword ptr [edi - 0x3c], -1
071A:    75 02                   jne      0x71e    ; Conditional jump
071C:    D9 E1                   fabs    ; |ST(0)|
071E:    DE C1                   faddp    st(1)
0720:    DD 19                   fstp     qword ptr [ecx]
0722:    E9 A1 00 00 00          jmp      0x7c8
0727:    90                      nop
0728:    90                      nop
0729:    83 7F C0 01             cmp      dword ptr [edi - 0x40], 1
072D:    75 4F                   jne      0x77e    ; Conditional jump
072F:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0732:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0735:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0738:    DD 46 18                fld      qword ptr [esi + 0x18]
073B:    83 7F CC FF             cmp      dword ptr [edi - 0x34], -1
073F:    75 02                   jne      0x743    ; Conditional jump
0741:    D9 E1                   fabs    ; |ST(0)|
0743:    DE C1                   faddp    st(1)
0745:    DD 1B                   fstp     qword ptr [ebx]
0747:    DD 45 D8                fld      qword ptr [ebp - 0x28]
074A:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
074D:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0750:    DD 46 20                fld      qword ptr [esi + 0x20]
0753:    83 7F C8 FF             cmp      dword ptr [edi - 0x38], -1
0757:    75 02                   jne      0x75b    ; Conditional jump
0759:    D9 E1                   fabs    ; |ST(0)|
075B:    DE C1                   faddp    st(1)
075D:    DD 1A                   fstp     qword ptr [edx]
075F:    DD 47 E0                fld      qword ptr [edi - 0x20]
0762:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0765:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0768:    DD 46 28                fld      qword ptr [esi + 0x28]
076B:    83 7F C4 FF             cmp      dword ptr [edi - 0x3c], -1
076F:    75 02                   jne      0x773    ; Conditional jump
0771:    D9 E1                   fabs    ; |ST(0)|
0773:    DE C1                   faddp    st(1)
0775:    DD 19                   fstp     qword ptr [ecx]
0777:    E9 4C 00 00 00          jmp      0x7c8
077C:    90                      nop
077D:    90                      nop
077E:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0781:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0784:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0787:    DD 46 18                fld      qword ptr [esi + 0x18]
078A:    83 7F CC FF             cmp      dword ptr [edi - 0x34], -1
078E:    75 02                   jne      0x792    ; Conditional jump
0790:    D9 E1                   fabs    ; |ST(0)|
0792:    DE C1                   faddp    st(1)
0794:    DD 1B                   fstp     qword ptr [ebx]
0796:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0799:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
079C:    DD 46 20                fld      qword ptr [esi + 0x20]
079F:    83 7F C8 FF             cmp      dword ptr [edi - 0x38], -1
07A3:    75 02                   jne      0x7a7    ; Conditional jump
07A5:    D9 E1                   fabs    ; |ST(0)|
07A7:    DE C1                   faddp    st(1)
07A9:    DD 1A                   fstp     qword ptr [edx]
07AB:    DD 47 E0                fld      qword ptr [edi - 0x20]
07AE:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
07B1:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
07B4:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
07B7:    DD 46 28                fld      qword ptr [esi + 0x28]
07BA:    83 7F C4 FF             cmp      dword ptr [edi - 0x3c], -1
07BE:    75 02                   jne      0x7c2    ; Conditional jump
07C0:    D9 E1                   fabs    ; |ST(0)|
07C2:    DE C1                   faddp    st(1)
07C4:    DD 19                   fstp     qword ptr [ecx]
07C6:    D9 D0                   fnop
07C8:    8B C3                   mov      eax, ebx
07CA:    5F                      pop      edi
07CB:    5E                      pop      esi
07CC:    5B                      pop      ebx
07CD:    89 EC                   mov      esp, ebp
07CF:    5D                      pop      ebp
07D0:    C2 08 00                ret      8
