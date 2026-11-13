; Disassembly: _GenCWarp.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 2042 bytes, decoded: 2042 bytes (100.0%)
; DEoption: -1

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
0016:    DD 46 28                fld      qword ptr [esi + 0x28]
0019:    DD 55 E8                fst      qword ptr [ebp - 0x18]
001C:    DD 46 20                fld      qword ptr [esi + 0x20]
001F:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0022:    DD 46 18                fld      qword ptr [esi + 0x18]
0025:    DD 55 F8                fst      qword ptr [ebp - 8]
0028:    D9 C0                   fld      st(0)
002A:    D8 4F DC                fmul     dword ptr [edi - 0x24]
002D:    D9 C2                   fld      st(2)
002F:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0032:    DE C1                   faddp    st(1)
0034:    D9 C3                   fld      st(3)
0036:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0039:    DE C1                   faddp    st(1)
003B:    D9 C1                   fld      st(1)
003D:    D8 4F F4                fmul     dword ptr [edi - 0xc]
0040:    D9 C3                   fld      st(3)
0042:    D8 4F F0                fmul     dword ptr [edi - 0x10]
0045:    DE C1                   faddp    st(1)
0047:    D9 C4                   fld      st(4)
0049:    D8 4F EC                fmul     dword ptr [edi - 0x14]
004C:    DE C1                   faddp    st(1)
004E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0050:    D8 4F E8                fmul     dword ptr [edi - 0x18]
0053:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0055:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
0058:    DE C3                   faddp    st(3)
005A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
005C:    D8 4F E0                fmul     dword ptr [edi - 0x20]
005F:    DE C2                   faddp    st(2)
0061:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0064:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0067:    DD 5E 28                fstp     qword ptr [esi + 0x28]
006A:    D9 D0                   fnop
006C:    83 7F C0 00             cmp      dword ptr [edi - 0x40], 0
0070:    75 0D                   jne      0x7f    ; Conditional jump
0072:    DD 46 18                fld      qword ptr [esi + 0x18]
0075:    DD 46 20                fld      qword ptr [esi + 0x20]
0078:    E9 13 03 00 00          jmp      0x390
007D:    90                      nop
007E:    90                      nop
007F:    83 7F C0 01             cmp      dword ptr [edi - 0x40], 1
0083:    75 0B                   jne      0x90    ; Conditional jump
0085:    DD 46 18                fld      qword ptr [esi + 0x18]
0088:    DD 46 28                fld      qword ptr [esi + 0x28]
008B:    E9 00 03 00 00          jmp      0x390
0090:    90                      nop
0091:    90                      nop
0092:    83 7F C0 02             cmp      dword ptr [edi - 0x40], 2
0096:    75 0B                   jne      0xa3    ; Conditional jump
0098:    DD 46 20                fld      qword ptr [esi + 0x20]
009B:    DD 46 18                fld      qword ptr [esi + 0x18]
009E:    E9 ED 02 00 00          jmp      0x390
00A3:    90                      nop
00A4:    90                      nop
00A5:    83 7F C0 03             cmp      dword ptr [edi - 0x40], 3
00A9:    75 0B                   jne      0xb6    ; Conditional jump
00AB:    DD 46 20                fld      qword ptr [esi + 0x20]
00AE:    DD 46 28                fld      qword ptr [esi + 0x28]
00B1:    E9 DA 02 00 00          jmp      0x390
00B6:    90                      nop
00B7:    90                      nop
00B8:    83 7F C0 04             cmp      dword ptr [edi - 0x40], 4
00BC:    75 0B                   jne      0xc9    ; Conditional jump
00BE:    DD 46 28                fld      qword ptr [esi + 0x28]
00C1:    DD 46 18                fld      qword ptr [esi + 0x18]
00C4:    E9 C7 02 00 00          jmp      0x390
00C9:    90                      nop
00CA:    90                      nop
00CB:    83 7F C0 05             cmp      dword ptr [edi - 0x40], 5
00CF:    75 0B                   jne      0xdc    ; Conditional jump
00D1:    DD 46 28                fld      qword ptr [esi + 0x28]
00D4:    DD 46 20                fld      qword ptr [esi + 0x20]
00D7:    E9 B4 02 00 00          jmp      0x390
00DC:    90                      nop
00DD:    90                      nop
00DE:    83 7F C0 06             cmp      dword ptr [edi - 0x40], 6
00E2:    75 18                   jne      0xfc    ; Conditional jump
00E4:    DD 46 18                fld      qword ptr [esi + 0x18]
00E7:    DD 46 20                fld      qword ptr [esi + 0x20]
00EA:    D8 C8                   fmul     st(0)
00EC:    DD 46 28                fld      qword ptr [esi + 0x28]
00EF:    D8 C8                   fmul     st(0)
00F1:    DE C1                   faddp    st(1)
00F3:    D9 FA                   fsqrt    ; sqrt(ST(0))
00F5:    E9 96 02 00 00          jmp      0x390
00FA:    90                      nop
00FB:    90                      nop
00FC:    83 7F C0 07             cmp      dword ptr [edi - 0x40], 7
0100:    75 18                   jne      0x11a    ; Conditional jump
0102:    DD 46 20                fld      qword ptr [esi + 0x20]
0105:    DD 46 18                fld      qword ptr [esi + 0x18]
0108:    D8 C8                   fmul     st(0)
010A:    DD 46 28                fld      qword ptr [esi + 0x28]
010D:    D8 C8                   fmul     st(0)
010F:    DE C1                   faddp    st(1)
0111:    D9 FA                   fsqrt    ; sqrt(ST(0))
0113:    E9 78 02 00 00          jmp      0x390
0118:    90                      nop
0119:    90                      nop
011A:    83 7F C0 08             cmp      dword ptr [edi - 0x40], 8
011E:    75 18                   jne      0x138    ; Conditional jump
0120:    DD 46 28                fld      qword ptr [esi + 0x28]
0123:    DD 46 18                fld      qword ptr [esi + 0x18]
0126:    D8 C8                   fmul     st(0)
0128:    DD 46 20                fld      qword ptr [esi + 0x20]
012B:    D8 C8                   fmul     st(0)
012D:    DE C1                   faddp    st(1)
012F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0131:    E9 5A 02 00 00          jmp      0x390
0136:    90                      nop
0137:    90                      nop
0138:    83 7F C0 09             cmp      dword ptr [edi - 0x40], 9
013C:    75 1A                   jne      0x158    ; Conditional jump
013E:    DD 46 18                fld      qword ptr [esi + 0x18]
0141:    DD 46 20                fld      qword ptr [esi + 0x20]
0144:    D8 C8                   fmul     st(0)
0146:    DD 46 28                fld      qword ptr [esi + 0x28]
0149:    D8 C8                   fmul     st(0)
014B:    DE C1                   faddp    st(1)
014D:    D9 FA                   fsqrt    ; sqrt(ST(0))
014F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0151:    E9 3A 02 00 00          jmp      0x390
0156:    90                      nop
0157:    90                      nop
0158:    83 7F C0 0A             cmp      dword ptr [edi - 0x40], 0xa
015C:    75 1A                   jne      0x178    ; Conditional jump
015E:    DD 46 20                fld      qword ptr [esi + 0x20]
0161:    DD 46 18                fld      qword ptr [esi + 0x18]
0164:    D8 C8                   fmul     st(0)
0166:    DD 46 28                fld      qword ptr [esi + 0x28]
0169:    D8 C8                   fmul     st(0)
016B:    DE C1                   faddp    st(1)
016D:    D9 FA                   fsqrt    ; sqrt(ST(0))
016F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0171:    E9 1A 02 00 00          jmp      0x390
0176:    90                      nop
0177:    90                      nop
0178:    83 7F C0 0B             cmp      dword ptr [edi - 0x40], 0xb
017C:    75 1A                   jne      0x198    ; Conditional jump
017E:    DD 46 28                fld      qword ptr [esi + 0x28]
0181:    DD 46 20                fld      qword ptr [esi + 0x20]
0184:    D8 C8                   fmul     st(0)
0186:    DD 46 18                fld      qword ptr [esi + 0x18]
0189:    D8 C8                   fmul     st(0)
018B:    DE C1                   faddp    st(1)
018D:    D9 FA                   fsqrt    ; sqrt(ST(0))
018F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0191:    E9 FA 01 00 00          jmp      0x390
0196:    90                      nop
0197:    90                      nop
0198:    83 7F C0 0C             cmp      dword ptr [edi - 0x40], 0xc
019C:    75 23                   jne      0x1c1    ; Conditional jump
019E:    DD 46 18                fld      qword ptr [esi + 0x18]
01A1:    DD 46 28                fld      qword ptr [esi + 0x28]
01A4:    D9 E1                   fabs    ; |ST(0)|
01A6:    DD 46 20                fld      qword ptr [esi + 0x20]
01A9:    D9 E1                   fabs    ; |ST(0)|
01AB:    D8 D1                   fcom     st(1)
01AD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01AF:    80 E4 41                and      ah, 0x41
01B2:    77 04                   ja       0x1b8    ; Conditional jump
01B4:    DD D9                   fstp     st(1)
01B6:    EB 02                   jmp      0x1ba
01B8:    DD D8                   fstp     st(0)
01BA:    E9 D1 01 00 00          jmp      0x390
01BF:    90                      nop
01C0:    90                      nop
01C1:    83 7F C0 0D             cmp      dword ptr [edi - 0x40], 0xd
01C5:    75 23                   jne      0x1ea    ; Conditional jump
01C7:    DD 46 20                fld      qword ptr [esi + 0x20]
01CA:    DD 46 28                fld      qword ptr [esi + 0x28]
01CD:    D9 E1                   fabs    ; |ST(0)|
01CF:    DD 46 18                fld      qword ptr [esi + 0x18]
01D2:    D9 E1                   fabs    ; |ST(0)|
01D4:    D8 D1                   fcom     st(1)
01D6:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01D8:    80 E4 41                and      ah, 0x41
01DB:    77 04                   ja       0x1e1    ; Conditional jump
01DD:    DD D9                   fstp     st(1)
01DF:    EB 02                   jmp      0x1e3
01E1:    DD D8                   fstp     st(0)
01E3:    E9 A8 01 00 00          jmp      0x390
01E8:    90                      nop
01E9:    90                      nop
01EA:    83 7F C0 0E             cmp      dword ptr [edi - 0x40], 0xe
01EE:    75 23                   jne      0x213    ; Conditional jump
01F0:    DD 46 28                fld      qword ptr [esi + 0x28]
01F3:    DD 46 18                fld      qword ptr [esi + 0x18]
01F6:    D9 E1                   fabs    ; |ST(0)|
01F8:    DD 46 20                fld      qword ptr [esi + 0x20]
01FB:    D9 E1                   fabs    ; |ST(0)|
01FD:    D8 D1                   fcom     st(1)
01FF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0201:    80 E4 41                and      ah, 0x41
0204:    77 04                   ja       0x20a    ; Conditional jump
0206:    DD D9                   fstp     st(1)
0208:    EB 02                   jmp      0x20c
020A:    DD D8                   fstp     st(0)
020C:    E9 7F 01 00 00          jmp      0x390
0211:    90                      nop
0212:    90                      nop
0213:    83 7F C0 0F             cmp      dword ptr [edi - 0x40], 0xf
0217:    75 25                   jne      0x23e    ; Conditional jump
0219:    DD 46 18                fld      qword ptr [esi + 0x18]
021C:    DD 46 28                fld      qword ptr [esi + 0x28]
021F:    D9 E1                   fabs    ; |ST(0)|
0221:    DD 46 20                fld      qword ptr [esi + 0x20]
0224:    D9 E1                   fabs    ; |ST(0)|
0226:    D8 D1                   fcom     st(1)
0228:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
022A:    80 E4 41                and      ah, 0x41
022D:    77 04                   ja       0x233    ; Conditional jump
022F:    DD D9                   fstp     st(1)
0231:    EB 02                   jmp      0x235
0233:    DD D8                   fstp     st(0)
0235:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0237:    E9 54 01 00 00          jmp      0x390
023C:    90                      nop
023D:    90                      nop
023E:    83 7F C0 10             cmp      dword ptr [edi - 0x40], 0x10
0242:    75 25                   jne      0x269    ; Conditional jump
0244:    DD 46 20                fld      qword ptr [esi + 0x20]
0247:    DD 46 28                fld      qword ptr [esi + 0x28]
024A:    D9 E1                   fabs    ; |ST(0)|
024C:    DD 46 18                fld      qword ptr [esi + 0x18]
024F:    D9 E1                   fabs    ; |ST(0)|
0251:    D8 D1                   fcom     st(1)
0253:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0255:    80 E4 41                and      ah, 0x41
0258:    77 04                   ja       0x25e    ; Conditional jump
025A:    DD D9                   fstp     st(1)
025C:    EB 02                   jmp      0x260
025E:    DD D8                   fstp     st(0)
0260:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0262:    E9 29 01 00 00          jmp      0x390
0267:    90                      nop
0268:    90                      nop
0269:    83 7F C0 11             cmp      dword ptr [edi - 0x40], 0x11
026D:    75 25                   jne      0x294    ; Conditional jump
026F:    DD 46 28                fld      qword ptr [esi + 0x28]
0272:    DD 46 20                fld      qword ptr [esi + 0x20]
0275:    D9 E1                   fabs    ; |ST(0)|
0277:    DD 46 18                fld      qword ptr [esi + 0x18]
027A:    D9 E1                   fabs    ; |ST(0)|
027C:    D8 D1                   fcom     st(1)
027E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0280:    80 E4 41                and      ah, 0x41
0283:    77 04                   ja       0x289    ; Conditional jump
0285:    DD D9                   fstp     st(1)
0287:    EB 02                   jmp      0x28b
0289:    DD D8                   fstp     st(0)
028B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
028D:    E9 FE 00 00 00          jmp      0x390
0292:    90                      nop
0293:    90                      nop
0294:    83 7F C0 12             cmp      dword ptr [edi - 0x40], 0x12
0298:    75 23                   jne      0x2bd    ; Conditional jump
029A:    DD 46 18                fld      qword ptr [esi + 0x18]
029D:    DD 46 28                fld      qword ptr [esi + 0x28]
02A0:    D9 E1                   fabs    ; |ST(0)|
02A2:    DD 46 20                fld      qword ptr [esi + 0x20]
02A5:    D9 E1                   fabs    ; |ST(0)|
02A7:    D8 D1                   fcom     st(1)
02A9:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
02AB:    80 E4 41                and      ah, 0x41
02AE:    72 04                   jb       0x2b4    ; Conditional jump
02B0:    DD D9                   fstp     st(1)
02B2:    EB 02                   jmp      0x2b6
02B4:    DD D8                   fstp     st(0)
02B6:    E9 D5 00 00 00          jmp      0x390
02BB:    90                      nop
02BC:    90                      nop
02BD:    83 7F C0 13             cmp      dword ptr [edi - 0x40], 0x13
02C1:    75 23                   jne      0x2e6    ; Conditional jump
02C3:    DD 46 20                fld      qword ptr [esi + 0x20]
02C6:    DD 46 28                fld      qword ptr [esi + 0x28]
02C9:    D9 E1                   fabs    ; |ST(0)|
02CB:    DD 46 18                fld      qword ptr [esi + 0x18]
02CE:    D9 E1                   fabs    ; |ST(0)|
02D0:    D8 D1                   fcom     st(1)
02D2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
02D4:    80 E4 41                and      ah, 0x41
02D7:    72 04                   jb       0x2dd    ; Conditional jump
02D9:    DD D9                   fstp     st(1)
02DB:    EB 02                   jmp      0x2df
02DD:    DD D8                   fstp     st(0)
02DF:    E9 AC 00 00 00          jmp      0x390
02E4:    90                      nop
02E5:    90                      nop
02E6:    83 7F C0 14             cmp      dword ptr [edi - 0x40], 0x14
02EA:    75 23                   jne      0x30f    ; Conditional jump
02EC:    DD 46 28                fld      qword ptr [esi + 0x28]
02EF:    DD 46 18                fld      qword ptr [esi + 0x18]
02F2:    D9 E1                   fabs    ; |ST(0)|
02F4:    DD 46 20                fld      qword ptr [esi + 0x20]
02F7:    D9 E1                   fabs    ; |ST(0)|
02F9:    D8 D1                   fcom     st(1)
02FB:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
02FD:    80 E4 41                and      ah, 0x41
0300:    72 04                   jb       0x306    ; Conditional jump
0302:    DD D9                   fstp     st(1)
0304:    EB 02                   jmp      0x308
0306:    DD D8                   fstp     st(0)
0308:    E9 83 00 00 00          jmp      0x390
030D:    90                      nop
030E:    90                      nop
030F:    83 7F C0 15             cmp      dword ptr [edi - 0x40], 0x15
0313:    75 25                   jne      0x33a    ; Conditional jump
0315:    DD 46 18                fld      qword ptr [esi + 0x18]
0318:    DD 46 28                fld      qword ptr [esi + 0x28]
031B:    D9 E1                   fabs    ; |ST(0)|
031D:    DD 46 20                fld      qword ptr [esi + 0x20]
0320:    D9 E1                   fabs    ; |ST(0)|
0322:    D8 D1                   fcom     st(1)
0324:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0326:    80 E4 41                and      ah, 0x41
0329:    72 04                   jb       0x32f    ; Conditional jump
032B:    DD D9                   fstp     st(1)
032D:    EB 02                   jmp      0x331
032F:    DD D8                   fstp     st(0)
0331:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0333:    E9 58 00 00 00          jmp      0x390
0338:    90                      nop
0339:    90                      nop
033A:    83 7F C0 16             cmp      dword ptr [edi - 0x40], 0x16
033E:    75 25                   jne      0x365    ; Conditional jump
0340:    DD 46 20                fld      qword ptr [esi + 0x20]
0343:    DD 46 28                fld      qword ptr [esi + 0x28]
0346:    D9 E1                   fabs    ; |ST(0)|
0348:    DD 46 18                fld      qword ptr [esi + 0x18]
034B:    D9 E1                   fabs    ; |ST(0)|
034D:    D8 D1                   fcom     st(1)
034F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0351:    80 E4 41                and      ah, 0x41
0354:    72 04                   jb       0x35a    ; Conditional jump
0356:    DD D9                   fstp     st(1)
0358:    EB 02                   jmp      0x35c
035A:    DD D8                   fstp     st(0)
035C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
035E:    E9 2D 00 00 00          jmp      0x390
0363:    90                      nop
0364:    90                      nop
0365:    83 7F C0 17             cmp      dword ptr [edi - 0x40], 0x17
0369:    75 25                   jne      0x390    ; Conditional jump
036B:    DD 46 28                fld      qword ptr [esi + 0x28]
036E:    DD 46 20                fld      qword ptr [esi + 0x20]
0371:    D9 E1                   fabs    ; |ST(0)|
0373:    DD 46 18                fld      qword ptr [esi + 0x18]
0376:    D9 E1                   fabs    ; |ST(0)|
0378:    D8 D1                   fcom     st(1)
037A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
037C:    80 E4 41                and      ah, 0x41
037F:    72 04                   jb       0x385    ; Conditional jump
0381:    DD D9                   fstp     st(1)
0383:    EB 02                   jmp      0x387
0385:    DD D8                   fstp     st(0)
0387:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0389:    E9 02 00 00 00          jmp      0x390
038E:    90                      nop
038F:    90                      nop
0390:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0392:    DC 4F CC                fmul     qword ptr [edi - 0x34]
0395:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0397:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
039A:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
039D:    D9 D0                   fnop
039F:    D9 D0                   fnop
03A1:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
03A5:    75 0D                   jne      0x3b4    ; Conditional jump
03A7:    DD 46 18                fld      qword ptr [esi + 0x18]
03AA:    DD 46 20                fld      qword ptr [esi + 0x20]
03AD:    E9 13 03 00 00          jmp      0x6c5
03B2:    90                      nop
03B3:    90                      nop
03B4:    83 7F BC 01             cmp      dword ptr [edi - 0x44], 1
03B8:    75 0B                   jne      0x3c5    ; Conditional jump
03BA:    DD 46 18                fld      qword ptr [esi + 0x18]
03BD:    DD 46 28                fld      qword ptr [esi + 0x28]
03C0:    E9 00 03 00 00          jmp      0x6c5
03C5:    90                      nop
03C6:    90                      nop
03C7:    83 7F BC 02             cmp      dword ptr [edi - 0x44], 2
03CB:    75 0B                   jne      0x3d8    ; Conditional jump
03CD:    DD 46 20                fld      qword ptr [esi + 0x20]
03D0:    DD 46 18                fld      qword ptr [esi + 0x18]
03D3:    E9 ED 02 00 00          jmp      0x6c5
03D8:    90                      nop
03D9:    90                      nop
03DA:    83 7F BC 03             cmp      dword ptr [edi - 0x44], 3
03DE:    75 0B                   jne      0x3eb    ; Conditional jump
03E0:    DD 46 20                fld      qword ptr [esi + 0x20]
03E3:    DD 46 28                fld      qword ptr [esi + 0x28]
03E6:    E9 DA 02 00 00          jmp      0x6c5
03EB:    90                      nop
03EC:    90                      nop
03ED:    83 7F BC 04             cmp      dword ptr [edi - 0x44], 4
03F1:    75 0B                   jne      0x3fe    ; Conditional jump
03F3:    DD 46 28                fld      qword ptr [esi + 0x28]
03F6:    DD 46 18                fld      qword ptr [esi + 0x18]
03F9:    E9 C7 02 00 00          jmp      0x6c5
03FE:    90                      nop
03FF:    90                      nop
0400:    83 7F BC 05             cmp      dword ptr [edi - 0x44], 5
0404:    75 0B                   jne      0x411    ; Conditional jump
0406:    DD 46 28                fld      qword ptr [esi + 0x28]
0409:    DD 46 20                fld      qword ptr [esi + 0x20]
040C:    E9 B4 02 00 00          jmp      0x6c5
0411:    90                      nop
0412:    90                      nop
0413:    83 7F BC 06             cmp      dword ptr [edi - 0x44], 6
0417:    75 18                   jne      0x431    ; Conditional jump
0419:    DD 46 18                fld      qword ptr [esi + 0x18]
041C:    DD 46 20                fld      qword ptr [esi + 0x20]
041F:    D8 C8                   fmul     st(0)
0421:    DD 46 28                fld      qword ptr [esi + 0x28]
0424:    D8 C8                   fmul     st(0)
0426:    DE C1                   faddp    st(1)
0428:    D9 FA                   fsqrt    ; sqrt(ST(0))
042A:    E9 96 02 00 00          jmp      0x6c5
042F:    90                      nop
0430:    90                      nop
0431:    83 7F BC 07             cmp      dword ptr [edi - 0x44], 7
0435:    75 18                   jne      0x44f    ; Conditional jump
0437:    DD 46 20                fld      qword ptr [esi + 0x20]
043A:    DD 46 18                fld      qword ptr [esi + 0x18]
043D:    D8 C8                   fmul     st(0)
043F:    DD 46 28                fld      qword ptr [esi + 0x28]
0442:    D8 C8                   fmul     st(0)
0444:    DE C1                   faddp    st(1)
0446:    D9 FA                   fsqrt    ; sqrt(ST(0))
0448:    E9 78 02 00 00          jmp      0x6c5
044D:    90                      nop
044E:    90                      nop
044F:    83 7F BC 08             cmp      dword ptr [edi - 0x44], 8
0453:    75 18                   jne      0x46d    ; Conditional jump
0455:    DD 46 28                fld      qword ptr [esi + 0x28]
0458:    DD 46 18                fld      qword ptr [esi + 0x18]
045B:    D8 C8                   fmul     st(0)
045D:    DD 46 20                fld      qword ptr [esi + 0x20]
0460:    D8 C8                   fmul     st(0)
0462:    DE C1                   faddp    st(1)
0464:    D9 FA                   fsqrt    ; sqrt(ST(0))
0466:    E9 5A 02 00 00          jmp      0x6c5
046B:    90                      nop
046C:    90                      nop
046D:    83 7F BC 09             cmp      dword ptr [edi - 0x44], 9
0471:    75 1A                   jne      0x48d    ; Conditional jump
0473:    DD 46 18                fld      qword ptr [esi + 0x18]
0476:    DD 46 20                fld      qword ptr [esi + 0x20]
0479:    D8 C8                   fmul     st(0)
047B:    DD 46 28                fld      qword ptr [esi + 0x28]
047E:    D8 C8                   fmul     st(0)
0480:    DE C1                   faddp    st(1)
0482:    D9 FA                   fsqrt    ; sqrt(ST(0))
0484:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0486:    E9 3A 02 00 00          jmp      0x6c5
048B:    90                      nop
048C:    90                      nop
048D:    83 7F BC 0A             cmp      dword ptr [edi - 0x44], 0xa
0491:    75 1A                   jne      0x4ad    ; Conditional jump
0493:    DD 46 20                fld      qword ptr [esi + 0x20]
0496:    DD 46 18                fld      qword ptr [esi + 0x18]
0499:    D8 C8                   fmul     st(0)
049B:    DD 46 28                fld      qword ptr [esi + 0x28]
049E:    D8 C8                   fmul     st(0)
04A0:    DE C1                   faddp    st(1)
04A2:    D9 FA                   fsqrt    ; sqrt(ST(0))
04A4:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
04A6:    E9 1A 02 00 00          jmp      0x6c5
04AB:    90                      nop
04AC:    90                      nop
04AD:    83 7F BC 0B             cmp      dword ptr [edi - 0x44], 0xb
04B1:    75 1A                   jne      0x4cd    ; Conditional jump
04B3:    DD 46 28                fld      qword ptr [esi + 0x28]
04B6:    DD 46 20                fld      qword ptr [esi + 0x20]
04B9:    D8 C8                   fmul     st(0)
04BB:    DD 46 18                fld      qword ptr [esi + 0x18]
04BE:    D8 C8                   fmul     st(0)
04C0:    DE C1                   faddp    st(1)
04C2:    D9 FA                   fsqrt    ; sqrt(ST(0))
04C4:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
04C6:    E9 FA 01 00 00          jmp      0x6c5
04CB:    90                      nop
04CC:    90                      nop
04CD:    83 7F BC 0C             cmp      dword ptr [edi - 0x44], 0xc
04D1:    75 23                   jne      0x4f6    ; Conditional jump
04D3:    DD 46 18                fld      qword ptr [esi + 0x18]
04D6:    DD 46 28                fld      qword ptr [esi + 0x28]
04D9:    D9 E1                   fabs    ; |ST(0)|
04DB:    DD 46 20                fld      qword ptr [esi + 0x20]
04DE:    D9 E1                   fabs    ; |ST(0)|
04E0:    D8 D1                   fcom     st(1)
04E2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
04E4:    80 E4 41                and      ah, 0x41
04E7:    77 04                   ja       0x4ed    ; Conditional jump
04E9:    DD D9                   fstp     st(1)
04EB:    EB 02                   jmp      0x4ef
04ED:    DD D8                   fstp     st(0)
04EF:    E9 D1 01 00 00          jmp      0x6c5
04F4:    90                      nop
04F5:    90                      nop
04F6:    83 7F BC 0D             cmp      dword ptr [edi - 0x44], 0xd
04FA:    75 23                   jne      0x51f    ; Conditional jump
04FC:    DD 46 20                fld      qword ptr [esi + 0x20]
04FF:    DD 46 28                fld      qword ptr [esi + 0x28]
0502:    D9 E1                   fabs    ; |ST(0)|
0504:    DD 46 18                fld      qword ptr [esi + 0x18]
0507:    D9 E1                   fabs    ; |ST(0)|
0509:    D8 D1                   fcom     st(1)
050B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
050D:    80 E4 41                and      ah, 0x41
0510:    77 04                   ja       0x516    ; Conditional jump
0512:    DD D9                   fstp     st(1)
0514:    EB 02                   jmp      0x518
0516:    DD D8                   fstp     st(0)
0518:    E9 A8 01 00 00          jmp      0x6c5
051D:    90                      nop
051E:    90                      nop
051F:    83 7F BC 0E             cmp      dword ptr [edi - 0x44], 0xe
0523:    75 23                   jne      0x548    ; Conditional jump
0525:    DD 46 28                fld      qword ptr [esi + 0x28]
0528:    DD 46 18                fld      qword ptr [esi + 0x18]
052B:    D9 E1                   fabs    ; |ST(0)|
052D:    DD 46 20                fld      qword ptr [esi + 0x20]
0530:    D9 E1                   fabs    ; |ST(0)|
0532:    D8 D1                   fcom     st(1)
0534:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0536:    80 E4 41                and      ah, 0x41
0539:    77 04                   ja       0x53f    ; Conditional jump
053B:    DD D9                   fstp     st(1)
053D:    EB 02                   jmp      0x541
053F:    DD D8                   fstp     st(0)
0541:    E9 7F 01 00 00          jmp      0x6c5
0546:    90                      nop
0547:    90                      nop
0548:    83 7F BC 0F             cmp      dword ptr [edi - 0x44], 0xf
054C:    75 25                   jne      0x573    ; Conditional jump
054E:    DD 46 18                fld      qword ptr [esi + 0x18]
0551:    DD 46 28                fld      qword ptr [esi + 0x28]
0554:    D9 E1                   fabs    ; |ST(0)|
0556:    DD 46 20                fld      qword ptr [esi + 0x20]
0559:    D9 E1                   fabs    ; |ST(0)|
055B:    D8 D1                   fcom     st(1)
055D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
055F:    80 E4 41                and      ah, 0x41
0562:    77 04                   ja       0x568    ; Conditional jump
0564:    DD D9                   fstp     st(1)
0566:    EB 02                   jmp      0x56a
0568:    DD D8                   fstp     st(0)
056A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
056C:    E9 54 01 00 00          jmp      0x6c5
0571:    90                      nop
0572:    90                      nop
0573:    83 7F BC 10             cmp      dword ptr [edi - 0x44], 0x10
0577:    75 25                   jne      0x59e    ; Conditional jump
0579:    DD 46 20                fld      qword ptr [esi + 0x20]
057C:    DD 46 28                fld      qword ptr [esi + 0x28]
057F:    D9 E1                   fabs    ; |ST(0)|
0581:    DD 46 18                fld      qword ptr [esi + 0x18]
0584:    D9 E1                   fabs    ; |ST(0)|
0586:    D8 D1                   fcom     st(1)
0588:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
058A:    80 E4 41                and      ah, 0x41
058D:    77 04                   ja       0x593    ; Conditional jump
058F:    DD D9                   fstp     st(1)
0591:    EB 02                   jmp      0x595
0593:    DD D8                   fstp     st(0)
0595:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0597:    E9 29 01 00 00          jmp      0x6c5
059C:    90                      nop
059D:    90                      nop
059E:    83 7F BC 11             cmp      dword ptr [edi - 0x44], 0x11
05A2:    75 25                   jne      0x5c9    ; Conditional jump
05A4:    DD 46 28                fld      qword ptr [esi + 0x28]
05A7:    DD 46 20                fld      qword ptr [esi + 0x20]
05AA:    D9 E1                   fabs    ; |ST(0)|
05AC:    DD 46 18                fld      qword ptr [esi + 0x18]
05AF:    D9 E1                   fabs    ; |ST(0)|
05B1:    D8 D1                   fcom     st(1)
05B3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
05B5:    80 E4 41                and      ah, 0x41
05B8:    77 04                   ja       0x5be    ; Conditional jump
05BA:    DD D9                   fstp     st(1)
05BC:    EB 02                   jmp      0x5c0
05BE:    DD D8                   fstp     st(0)
05C0:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
05C2:    E9 FE 00 00 00          jmp      0x6c5
05C7:    90                      nop
05C8:    90                      nop
05C9:    83 7F BC 12             cmp      dword ptr [edi - 0x44], 0x12
05CD:    75 23                   jne      0x5f2    ; Conditional jump
05CF:    DD 46 18                fld      qword ptr [esi + 0x18]
05D2:    DD 46 28                fld      qword ptr [esi + 0x28]
05D5:    D9 E1                   fabs    ; |ST(0)|
05D7:    DD 46 20                fld      qword ptr [esi + 0x20]
05DA:    D9 E1                   fabs    ; |ST(0)|
05DC:    D8 D1                   fcom     st(1)
05DE:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
05E0:    80 E4 41                and      ah, 0x41
05E3:    72 04                   jb       0x5e9    ; Conditional jump
05E5:    DD D9                   fstp     st(1)
05E7:    EB 02                   jmp      0x5eb
05E9:    DD D8                   fstp     st(0)
05EB:    E9 D5 00 00 00          jmp      0x6c5
05F0:    90                      nop
05F1:    90                      nop
05F2:    83 7F BC 13             cmp      dword ptr [edi - 0x44], 0x13
05F6:    75 23                   jne      0x61b    ; Conditional jump
05F8:    DD 46 20                fld      qword ptr [esi + 0x20]
05FB:    DD 46 28                fld      qword ptr [esi + 0x28]
05FE:    D9 E1                   fabs    ; |ST(0)|
0600:    DD 46 18                fld      qword ptr [esi + 0x18]
0603:    D9 E1                   fabs    ; |ST(0)|
0605:    D8 D1                   fcom     st(1)
0607:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0609:    80 E4 41                and      ah, 0x41
060C:    72 04                   jb       0x612    ; Conditional jump
060E:    DD D9                   fstp     st(1)
0610:    EB 02                   jmp      0x614
0612:    DD D8                   fstp     st(0)
0614:    E9 AC 00 00 00          jmp      0x6c5
0619:    90                      nop
061A:    90                      nop
061B:    83 7F BC 14             cmp      dword ptr [edi - 0x44], 0x14
061F:    75 23                   jne      0x644    ; Conditional jump
0621:    DD 46 28                fld      qword ptr [esi + 0x28]
0624:    DD 46 18                fld      qword ptr [esi + 0x18]
0627:    D9 E1                   fabs    ; |ST(0)|
0629:    DD 46 20                fld      qword ptr [esi + 0x20]
062C:    D9 E1                   fabs    ; |ST(0)|
062E:    D8 D1                   fcom     st(1)
0630:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0632:    80 E4 41                and      ah, 0x41
0635:    72 04                   jb       0x63b    ; Conditional jump
0637:    DD D9                   fstp     st(1)
0639:    EB 02                   jmp      0x63d
063B:    DD D8                   fstp     st(0)
063D:    E9 83 00 00 00          jmp      0x6c5
0642:    90                      nop
0643:    90                      nop
0644:    83 7F BC 15             cmp      dword ptr [edi - 0x44], 0x15
0648:    75 25                   jne      0x66f    ; Conditional jump
064A:    DD 46 18                fld      qword ptr [esi + 0x18]
064D:    DD 46 28                fld      qword ptr [esi + 0x28]
0650:    D9 E1                   fabs    ; |ST(0)|
0652:    DD 46 20                fld      qword ptr [esi + 0x20]
0655:    D9 E1                   fabs    ; |ST(0)|
0657:    D8 D1                   fcom     st(1)
0659:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
065B:    80 E4 41                and      ah, 0x41
065E:    72 04                   jb       0x664    ; Conditional jump
0660:    DD D9                   fstp     st(1)
0662:    EB 02                   jmp      0x666
0664:    DD D8                   fstp     st(0)
0666:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0668:    E9 58 00 00 00          jmp      0x6c5
066D:    90                      nop
066E:    90                      nop
066F:    83 7F BC 16             cmp      dword ptr [edi - 0x44], 0x16
0673:    75 25                   jne      0x69a    ; Conditional jump
0675:    DD 46 20                fld      qword ptr [esi + 0x20]
0678:    DD 46 28                fld      qword ptr [esi + 0x28]
067B:    D9 E1                   fabs    ; |ST(0)|
067D:    DD 46 18                fld      qword ptr [esi + 0x18]
0680:    D9 E1                   fabs    ; |ST(0)|
0682:    D8 D1                   fcom     st(1)
0684:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0686:    80 E4 41                and      ah, 0x41
0689:    72 04                   jb       0x68f    ; Conditional jump
068B:    DD D9                   fstp     st(1)
068D:    EB 02                   jmp      0x691
068F:    DD D8                   fstp     st(0)
0691:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0693:    E9 2D 00 00 00          jmp      0x6c5
0698:    90                      nop
0699:    90                      nop
069A:    83 7F BC 17             cmp      dword ptr [edi - 0x44], 0x17
069E:    75 25                   jne      0x6c5    ; Conditional jump
06A0:    DD 46 28                fld      qword ptr [esi + 0x28]
06A3:    DD 46 20                fld      qword ptr [esi + 0x20]
06A6:    D9 E1                   fabs    ; |ST(0)|
06A8:    DD 46 18                fld      qword ptr [esi + 0x18]
06AB:    D9 E1                   fabs    ; |ST(0)|
06AD:    D8 D1                   fcom     st(1)
06AF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
06B1:    80 E4 41                and      ah, 0x41
06B4:    72 04                   jb       0x6ba    ; Conditional jump
06B6:    DD D9                   fstp     st(1)
06B8:    EB 02                   jmp      0x6bc
06BA:    DD D8                   fstp     st(0)
06BC:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
06BE:    E9 02 00 00 00          jmp      0x6c5
06C3:    90                      nop
06C4:    90                      nop
06C5:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
06C7:    DC 4F C4                fmul     qword ptr [edi - 0x3c]
06CA:    D9 FB                   fsincos    ; ST(0)=cos, push sin
06CC:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
06CF:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
06D2:    D9 D0                   fnop
06D4:    DD 46 18                fld      qword ptr [esi + 0x18]
06D7:    D8 C8                   fmul     st(0)
06D9:    DD 46 20                fld      qword ptr [esi + 0x20]
06DC:    D8 C8                   fmul     st(0)
06DE:    DD 46 28                fld      qword ptr [esi + 0x28]
06E1:    D8 C8                   fmul     st(0)
06E3:    DE C1                   faddp    st(1)
06E5:    DE C1                   faddp    st(1)
06E7:    D9 FA                   fsqrt    ; sqrt(ST(0))
06E9:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
06EC:    90                      nop
06ED:    90                      nop
06EE:    83 7F B8 00             cmp      dword ptr [edi - 0x48], 0
06F2:    75 2B                   jne      0x71f    ; Conditional jump
06F4:    DD 45 D0                fld      qword ptr [ebp - 0x30]
06F7:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
06FA:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
06FD:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0700:    90                      nop
0701:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0704:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0707:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
070A:    DD 5E 20                fstp     qword ptr [esi + 0x20]
070D:    90                      nop
070E:    DD 45 C8                fld      qword ptr [ebp - 0x38]
0711:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0714:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0717:    90                      nop
0718:    E9 58 00 00 00          jmp      0x775
071D:    90                      nop
071E:    90                      nop
071F:    83 7F B8 01             cmp      dword ptr [edi - 0x48], 1
0723:    75 2C                   jne      0x751    ; Conditional jump
0725:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0728:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
072B:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
072E:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0731:    90                      nop
0732:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0735:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0738:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
073B:    DD 5E 20                fstp     qword ptr [esi + 0x20]
073E:    90                      nop
073F:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0742:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0745:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0748:    90                      nop
0749:    90                      nop
074A:    E9 26 00 00 00          jmp      0x775
074F:    90                      nop
0750:    90                      nop
0751:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0754:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0757:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
075A:    DD 5E 18                fstp     qword ptr [esi + 0x18]
075D:    90                      nop
075E:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0761:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0764:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0767:    90                      nop
0768:    DD 45 D0                fld      qword ptr [ebp - 0x30]
076B:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
076E:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0771:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0774:    90                      nop
0775:    90                      nop
0776:    90                      nop
0777:    DD 47 B0                fld      qword ptr [edi - 0x50]
077A:    D9 E8                   fld1    ; Load 1.0
077C:    D8 E1                   fsub     st(1)
077E:    90                      nop
077F:    90                      nop
0780:    DD 45 F8                fld      qword ptr [ebp - 8]
0783:    D8 C9                   fmul     st(1)
0785:    DD 46 18                fld      qword ptr [esi + 0x18]
0788:    D8 CB                   fmul     st(3)
078A:    DE C1                   faddp    st(1)
078C:    DD 5E 18                fstp     qword ptr [esi + 0x18]
078F:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0792:    D8 C9                   fmul     st(1)
0794:    DD 46 20                fld      qword ptr [esi + 0x20]
0797:    D8 CB                   fmul     st(3)
0799:    DE C1                   faddp    st(1)
079B:    DD 5E 20                fstp     qword ptr [esi + 0x20]
079E:    DD 45 E8                fld      qword ptr [ebp - 0x18]
07A1:    D8 C9                   fmul     st(1)
07A3:    DD 46 28                fld      qword ptr [esi + 0x28]
07A6:    D8 CB                   fmul     st(3)
07A8:    DE C1                   faddp    st(1)
07AA:    DD 5E 28                fstp     qword ptr [esi + 0x28]
07AD:    DD D8                   fstp     st(0)
07AF:    DD D8                   fstp     st(0)
07B1:    90                      nop
07B2:    90                      nop
07B3:    DD 47 A8                fld      qword ptr [edi - 0x58]
07B6:    D9 E8                   fld1    ; Load 1.0
07B8:    D8 E1                   fsub     st(1)
07BA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
07BC:    DD 03                   fld      qword ptr [ebx]
07BE:    D8 C9                   fmul     st(1)
07C0:    DD 46 18                fld      qword ptr [esi + 0x18]
07C3:    D8 CB                   fmul     st(3)
07C5:    DE C1                   faddp    st(1)
07C7:    DD 5E 18                fstp     qword ptr [esi + 0x18]
07CA:    DD 02                   fld      qword ptr [edx]
07CC:    D8 C9                   fmul     st(1)
07CE:    DD 46 20                fld      qword ptr [esi + 0x20]
07D1:    D8 CB                   fmul     st(3)
07D3:    DE C1                   faddp    st(1)
07D5:    DD 5E 20                fstp     qword ptr [esi + 0x20]
07D8:    DD 01                   fld      qword ptr [ecx]
07DA:    D8 C9                   fmul     st(1)
07DC:    DD 46 28                fld      qword ptr [esi + 0x28]
07DF:    D8 CB                   fmul     st(3)
07E1:    DE C1                   faddp    st(1)
07E3:    DD 5E 28                fstp     qword ptr [esi + 0x28]
07E6:    DD D8                   fstp     st(0)
07E8:    DD D8                   fstp     st(0)
07EA:    90                      nop
07EB:    90                      nop
07EC:    90                      nop
07ED:    D9 D0                   fnop
07EF:    8B C3                   mov      eax, ebx
07F1:    5F                      pop      edi
07F2:    5E                      pop      esi
07F3:    5B                      pop      ebx
07F4:    89 EC                   mov      esp, ebp
07F6:    5D                      pop      ebp
07F7:    C2 08 00                ret      8
