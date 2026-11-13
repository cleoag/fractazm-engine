; Disassembly: TetraFolding Pow2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 1062 bytes, decoded: 1062 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 3C 00 00 00       sub      esp, 0x3c
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    90                      nop
0015:    90                      nop
0016:    90                      nop
0017:    90                      nop
0018:    90                      nop
0019:    90                      nop
001A:    D9 D0                   fnop
001C:    E9 AA 00 00 00          jmp      0xcb
0021:    DD 01                   fld      qword ptr [ecx]
0023:    DD 02                   fld      qword ptr [edx]
0025:    DD 03                   fld      qword ptr [ebx]
0027:    D9 C0                   fld      st(0)
0029:    D8 4D CC                fmul     dword ptr [ebp - 0x34]
002C:    D9 C2                   fld      st(2)
002E:    D8 4D C8                fmul     dword ptr [ebp - 0x38]
0031:    DE C1                   faddp    st(1)
0033:    D9 C3                   fld      st(3)
0035:    D8 4D C4                fmul     dword ptr [ebp - 0x3c]
0038:    DE C1                   faddp    st(1)
003A:    D9 C1                   fld      st(1)
003C:    D8 4D E4                fmul     dword ptr [ebp - 0x1c]
003F:    D9 C3                   fld      st(3)
0041:    D8 4D E0                fmul     dword ptr [ebp - 0x20]
0044:    DE C1                   faddp    st(1)
0046:    D9 C4                   fld      st(4)
0048:    D8 4D DC                fmul     dword ptr [ebp - 0x24]
004B:    DE C1                   faddp    st(1)
004D:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
004F:    D8 4D D8                fmul     dword ptr [ebp - 0x28]
0052:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0054:    D8 4D D4                fmul     dword ptr [ebp - 0x2c]
0057:    DE C3                   faddp    st(3)
0059:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
005B:    D8 4D D0                fmul     dword ptr [ebp - 0x30]
005E:    DE C2                   faddp    st(2)
0060:    83 7F 9C 01             cmp      dword ptr [edi - 0x64], 1
0064:    74 0C                   je       0x72    ; Conditional jump
0066:    DC 67 C8                fsub     qword ptr [edi - 0x38]
0069:    D9 E1                   fabs    ; |ST(0)|
006B:    D9 E0                   fchs    ; -ST(0)
006D:    DC 47 C8                fadd     qword ptr [edi - 0x38]
0070:    EB 18                   jmp      0x8a
0072:    D9 C0                   fld      st(0)
0074:    DC 47 C8                fadd     qword ptr [edi - 0x38]
0077:    D9 E1                   fabs    ; |ST(0)|
0079:    D9 C1                   fld      st(1)
007B:    DC 67 C8                fsub     qword ptr [edi - 0x38]
007E:    D9 E1                   fabs    ; |ST(0)|
0080:    D9 E0                   fchs    ; -ST(0)
0082:    DE C1                   faddp    st(1)
0084:    DE E9                   fsubp    st(1)
0086:    D9 E0                   fchs    ; -ST(0)
0088:    D9 D0                   fnop
008A:    D9 C0                   fld      st(0)
008C:    D8 4D DC                fmul     dword ptr [ebp - 0x24]
008F:    D9 C2                   fld      st(2)
0091:    D8 4D D0                fmul     dword ptr [ebp - 0x30]
0094:    DE C1                   faddp    st(1)
0096:    D9 C3                   fld      st(3)
0098:    D8 4D C4                fmul     dword ptr [ebp - 0x3c]
009B:    DE C1                   faddp    st(1)
009D:    D9 C1                   fld      st(1)
009F:    D8 4D E4                fmul     dword ptr [ebp - 0x1c]
00A2:    D9 C3                   fld      st(3)
00A4:    D8 4D D8                fmul     dword ptr [ebp - 0x28]
00A7:    DE C1                   faddp    st(1)
00A9:    D9 C4                   fld      st(4)
00AB:    D8 4D CC                fmul     dword ptr [ebp - 0x34]
00AE:    DE C1                   faddp    st(1)
00B0:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00B2:    D8 4D E0                fmul     dword ptr [ebp - 0x20]
00B5:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00B7:    D8 4D D4                fmul     dword ptr [ebp - 0x2c]
00BA:    DE C3                   faddp    st(3)
00BC:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00BE:    D8 4D C8                fmul     dword ptr [ebp - 0x38]
00C1:    DE C2                   faddp    st(2)
00C3:    DD 1B                   fstp     qword ptr [ebx]
00C5:    DD 1A                   fstp     qword ptr [edx]
00C7:    DD 19                   fstp     qword ptr [ecx]
00C9:    C3                      ret
00CA:    90                      nop
00CB:    D9 47 C4                fld      dword ptr [edi - 0x3c]
00CE:    D9 5D E4                fstp     dword ptr [ebp - 0x1c]
00D1:    D9 47 C0                fld      dword ptr [edi - 0x40]
00D4:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
00D7:    D9 47 BC                fld      dword ptr [edi - 0x44]
00DA:    D9 5D DC                fstp     dword ptr [ebp - 0x24]
00DD:    D9 47 B8                fld      dword ptr [edi - 0x48]
00E0:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
00E3:    D9 47 B4                fld      dword ptr [edi - 0x4c]
00E6:    D9 5D D4                fstp     dword ptr [ebp - 0x2c]
00E9:    D9 47 B0                fld      dword ptr [edi - 0x50]
00EC:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
00EF:    D9 47 AC                fld      dword ptr [edi - 0x54]
00F2:    D9 5D CC                fstp     dword ptr [ebp - 0x34]
00F5:    D9 47 A8                fld      dword ptr [edi - 0x58]
00F8:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
00FB:    D9 47 A4                fld      dword ptr [edi - 0x5c]
00FE:    D9 5D C4                fstp     dword ptr [ebp - 0x3c]
0101:    E8 76 02 00 00          call     0x37c
0106:    83 7F A0 00             cmp      dword ptr [edi - 0x60], 0
010A:    75 74                   jne      0x180    ; Conditional jump
010C:    D9 45 DC                fld      dword ptr [ebp - 0x24]
010F:    D9 E0                   fchs    ; -ST(0)
0111:    D9 5D DC                fstp     dword ptr [ebp - 0x24]
0114:    D9 45 D0                fld      dword ptr [ebp - 0x30]
0117:    D9 E0                   fchs    ; -ST(0)
0119:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
011C:    D9 45 CC                fld      dword ptr [ebp - 0x34]
011F:    D9 E0                   fchs    ; -ST(0)
0121:    D9 5D CC                fstp     dword ptr [ebp - 0x34]
0124:    D9 45 C8                fld      dword ptr [ebp - 0x38]
0127:    D9 E0                   fchs    ; -ST(0)
0129:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
012C:    E8 F0 FE FF FF          call     0x21
0131:    D9 45 E0                fld      dword ptr [ebp - 0x20]
0134:    D9 E0                   fchs    ; -ST(0)
0136:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
0139:    D9 45 D0                fld      dword ptr [ebp - 0x30]
013C:    D9 E0                   fchs    ; -ST(0)
013E:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
0141:    D9 45 D8                fld      dword ptr [ebp - 0x28]
0144:    D9 E0                   fchs    ; -ST(0)
0146:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
0149:    D9 45 C8                fld      dword ptr [ebp - 0x38]
014C:    D9 E0                   fchs    ; -ST(0)
014E:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
0151:    E8 CB FE FF FF          call     0x21
0156:    D9 45 DC                fld      dword ptr [ebp - 0x24]
0159:    D9 E0                   fchs    ; -ST(0)
015B:    D9 5D DC                fstp     dword ptr [ebp - 0x24]
015E:    D9 45 D0                fld      dword ptr [ebp - 0x30]
0161:    D9 E0                   fchs    ; -ST(0)
0163:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
0166:    D9 45 CC                fld      dword ptr [ebp - 0x34]
0169:    D9 E0                   fchs    ; -ST(0)
016B:    D9 5D CC                fstp     dword ptr [ebp - 0x34]
016E:    D9 45 C8                fld      dword ptr [ebp - 0x38]
0171:    D9 E0                   fchs    ; -ST(0)
0173:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
0176:    E8 A6 FE FF FF          call     0x21
017B:    E9 68 01 00 00          jmp      0x2e8
0180:    83 7F A0 01             cmp      dword ptr [edi - 0x60], 1
0184:    75 74                   jne      0x1fa    ; Conditional jump
0186:    D9 45 E0                fld      dword ptr [ebp - 0x20]
0189:    D9 E0                   fchs    ; -ST(0)
018B:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
018E:    D9 45 D0                fld      dword ptr [ebp - 0x30]
0191:    D9 E0                   fchs    ; -ST(0)
0193:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
0196:    D9 45 D8                fld      dword ptr [ebp - 0x28]
0199:    D9 E0                   fchs    ; -ST(0)
019B:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
019E:    D9 45 C8                fld      dword ptr [ebp - 0x38]
01A1:    D9 E0                   fchs    ; -ST(0)
01A3:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
01A6:    E8 76 FE FF FF          call     0x21
01AB:    D9 45 DC                fld      dword ptr [ebp - 0x24]
01AE:    D9 E0                   fchs    ; -ST(0)
01B0:    D9 5D DC                fstp     dword ptr [ebp - 0x24]
01B3:    D9 45 D0                fld      dword ptr [ebp - 0x30]
01B6:    D9 E0                   fchs    ; -ST(0)
01B8:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
01BB:    D9 45 CC                fld      dword ptr [ebp - 0x34]
01BE:    D9 E0                   fchs    ; -ST(0)
01C0:    D9 5D CC                fstp     dword ptr [ebp - 0x34]
01C3:    D9 45 C8                fld      dword ptr [ebp - 0x38]
01C6:    D9 E0                   fchs    ; -ST(0)
01C8:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
01CB:    E8 51 FE FF FF          call     0x21
01D0:    D9 45 E0                fld      dword ptr [ebp - 0x20]
01D3:    D9 E0                   fchs    ; -ST(0)
01D5:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
01D8:    D9 45 D0                fld      dword ptr [ebp - 0x30]
01DB:    D9 E0                   fchs    ; -ST(0)
01DD:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
01E0:    D9 45 D8                fld      dword ptr [ebp - 0x28]
01E3:    D9 E0                   fchs    ; -ST(0)
01E5:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
01E8:    D9 45 C8                fld      dword ptr [ebp - 0x38]
01EB:    D9 E0                   fchs    ; -ST(0)
01ED:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
01F0:    E8 2C FE FF FF          call     0x21
01F5:    E9 EE 00 00 00          jmp      0x2e8
01FA:    83 7F A0 02             cmp      dword ptr [edi - 0x60], 2
01FE:    75 71                   jne      0x271    ; Conditional jump
0200:    D9 45 DC                fld      dword ptr [ebp - 0x24]
0203:    D9 E0                   fchs    ; -ST(0)
0205:    D9 5D DC                fstp     dword ptr [ebp - 0x24]
0208:    D9 45 E0                fld      dword ptr [ebp - 0x20]
020B:    D9 E0                   fchs    ; -ST(0)
020D:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
0210:    D9 45 CC                fld      dword ptr [ebp - 0x34]
0213:    D9 E0                   fchs    ; -ST(0)
0215:    D9 5D CC                fstp     dword ptr [ebp - 0x34]
0218:    D9 45 D8                fld      dword ptr [ebp - 0x28]
021B:    D9 E0                   fchs    ; -ST(0)
021D:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
0220:    E8 FC FD FF FF          call     0x21
0225:    D9 45 E0                fld      dword ptr [ebp - 0x20]
0228:    D9 E0                   fchs    ; -ST(0)
022A:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
022D:    D9 45 D0                fld      dword ptr [ebp - 0x30]
0230:    D9 E0                   fchs    ; -ST(0)
0232:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
0235:    D9 45 D8                fld      dword ptr [ebp - 0x28]
0238:    D9 E0                   fchs    ; -ST(0)
023A:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
023D:    D9 45 C8                fld      dword ptr [ebp - 0x38]
0240:    D9 E0                   fchs    ; -ST(0)
0242:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
0245:    E8 D7 FD FF FF          call     0x21
024A:    D9 45 DC                fld      dword ptr [ebp - 0x24]
024D:    D9 E0                   fchs    ; -ST(0)
024F:    D9 5D DC                fstp     dword ptr [ebp - 0x24]
0252:    D9 45 E0                fld      dword ptr [ebp - 0x20]
0255:    D9 E0                   fchs    ; -ST(0)
0257:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
025A:    D9 45 CC                fld      dword ptr [ebp - 0x34]
025D:    D9 E0                   fchs    ; -ST(0)
025F:    D9 5D CC                fstp     dword ptr [ebp - 0x34]
0262:    D9 45 D8                fld      dword ptr [ebp - 0x28]
0265:    D9 E0                   fchs    ; -ST(0)
0267:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
026A:    E8 B2 FD FF FF          call     0x21
026F:    EB 77                   jmp      0x2e8
0271:    90                      nop
0272:    90                      nop
0273:    90                      nop
0274:    90                      nop
0275:    90                      nop
0276:    90                      nop
0277:    D9 45 E0                fld      dword ptr [ebp - 0x20]
027A:    D9 E0                   fchs    ; -ST(0)
027C:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
027F:    D9 45 D0                fld      dword ptr [ebp - 0x30]
0282:    D9 E0                   fchs    ; -ST(0)
0284:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
0287:    D9 45 D8                fld      dword ptr [ebp - 0x28]
028A:    D9 E0                   fchs    ; -ST(0)
028C:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
028F:    D9 45 C8                fld      dword ptr [ebp - 0x38]
0292:    D9 E0                   fchs    ; -ST(0)
0294:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
0297:    E8 85 FD FF FF          call     0x21
029C:    D9 45 DC                fld      dword ptr [ebp - 0x24]
029F:    D9 E0                   fchs    ; -ST(0)
02A1:    D9 5D DC                fstp     dword ptr [ebp - 0x24]
02A4:    D9 45 E0                fld      dword ptr [ebp - 0x20]
02A7:    D9 E0                   fchs    ; -ST(0)
02A9:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
02AC:    D9 45 CC                fld      dword ptr [ebp - 0x34]
02AF:    D9 E0                   fchs    ; -ST(0)
02B1:    D9 5D CC                fstp     dword ptr [ebp - 0x34]
02B4:    D9 45 D8                fld      dword ptr [ebp - 0x28]
02B7:    D9 E0                   fchs    ; -ST(0)
02B9:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
02BC:    E8 60 FD FF FF          call     0x21
02C1:    D9 45 E0                fld      dword ptr [ebp - 0x20]
02C4:    D9 E0                   fchs    ; -ST(0)
02C6:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
02C9:    D9 45 D0                fld      dword ptr [ebp - 0x30]
02CC:    D9 E0                   fchs    ; -ST(0)
02CE:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
02D1:    D9 45 D8                fld      dword ptr [ebp - 0x28]
02D4:    D9 E0                   fchs    ; -ST(0)
02D6:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
02D9:    D9 45 C8                fld      dword ptr [ebp - 0x38]
02DC:    D9 E0                   fchs    ; -ST(0)
02DE:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
02E1:    E8 3B FD FF FF          call     0x21
02E6:    D9 D0                   fnop
02E8:    D9 D0                   fnop
02EA:    DD 03                   fld      qword ptr [ebx]
02EC:    D8 C8                   fmul     st(0)
02EE:    DD 02                   fld      qword ptr [edx]
02F0:    D8 C8                   fmul     st(0)
02F2:    DD 47 D8                fld      qword ptr [edi - 0x28]
02F5:    DE C1                   faddp    st(1)
02F7:    DE C1                   faddp    st(1)
02F9:    D9 EE                   fldz    ; Load 0.0
02FB:    D8 D9                   fcomp    st(1)
02FD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
02FF:    9E                      sahf
0300:    77 66                   ja       0x368    ; Conditional jump
0302:    74 6C                   je       0x370    ; Conditional jump
0304:    90                      nop
0305:    90                      nop
0306:    DD 03                   fld      qword ptr [ebx]
0308:    D8 C8                   fmul     st(0)
030A:    DD 02                   fld      qword ptr [edx]
030C:    D8 C8                   fmul     st(0)
030E:    D9 E0                   fchs    ; -ST(0)
0310:    DE C1                   faddp    st(1)
0312:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0315:    DD 47 D0                fld      qword ptr [edi - 0x30]
0318:    DD 01                   fld      qword ptr [ecx]
031A:    D8 C8                   fmul     st(0)
031C:    D8 F3                   fdiv     st(3)
031E:    D9 E0                   fchs    ; -ST(0)
0320:    DE C1                   faddp    st(1)
0322:    DE C9                   fmulp    st(1)
0324:    DC 46 18                fadd     qword ptr [esi + 0x18]
0327:    D9 D0                   fnop
0329:    DD 03                   fld      qword ptr [ebx]
032B:    DC 0A                   fmul     qword ptr [edx]
032D:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0330:    DD 47 D0                fld      qword ptr [edi - 0x30]
0333:    DD 01                   fld      qword ptr [ecx]
0335:    D8 C8                   fmul     st(0)
0337:    D8 F4                   fdiv     st(4)
0339:    D9 E0                   fchs    ; -ST(0)
033B:    DE C1                   faddp    st(1)
033D:    DE C9                   fmulp    st(1)
033F:    DC 46 20                fadd     qword ptr [esi + 0x20]
0342:    D9 D0                   fnop
0344:    DD 01                   fld      qword ptr [ecx]
0346:    D9 C3                   fld      st(3)
0348:    D9 FA                   fsqrt    ; sqrt(ST(0))
034A:    DE C9                   fmulp    st(1)
034C:    DC 4F E0                fmul     qword ptr [edi - 0x20]
034F:    DC 46 28                fadd     qword ptr [esi + 0x28]
0352:    D9 D0                   fnop
0354:    DD 19                   fstp     qword ptr [ecx]
0356:    DD 1A                   fstp     qword ptr [edx]
0358:    DD 1B                   fstp     qword ptr [ebx]
035A:    DD D8                   fstp     st(0)
035C:    8B C3                   mov      eax, ebx
035E:    5F                      pop      edi
035F:    5E                      pop      esi
0360:    5B                      pop      ebx
0361:    8B E5                   mov      esp, ebp
0363:    5D                      pop      ebp
0364:    C2 08 00                ret      8
0367:    90                      nop
0368:    D9 EE                   fldz    ; Load 0.0
036A:    D9 EE                   fldz    ; Load 0.0
036C:    D9 EE                   fldz    ; Load 0.0
036E:    EB E2                   jmp      0x352
0370:    DD 46 18                fld      qword ptr [esi + 0x18]
0373:    DD 46 20                fld      qword ptr [esi + 0x20]
0376:    DD 46 28                fld      qword ptr [esi + 0x28]
0379:    EB D7                   jmp      0x352
037B:    90                      nop
037C:    DD 01                   fld      qword ptr [ecx]
037E:    DD 02                   fld      qword ptr [edx]
0380:    DD 03                   fld      qword ptr [ebx]
0382:    D9 C0                   fld      st(0)
0384:    D8 4D CC                fmul     dword ptr [ebp - 0x34]
0387:    D9 C2                   fld      st(2)
0389:    D8 4D C8                fmul     dword ptr [ebp - 0x38]
038C:    DE C1                   faddp    st(1)
038E:    D9 C3                   fld      st(3)
0390:    D8 4D C4                fmul     dword ptr [ebp - 0x3c]
0393:    DE C1                   faddp    st(1)
0395:    D9 C1                   fld      st(1)
0397:    D8 4D E4                fmul     dword ptr [ebp - 0x1c]
039A:    D9 C3                   fld      st(3)
039C:    D8 4D E0                fmul     dword ptr [ebp - 0x20]
039F:    DE C1                   faddp    st(1)
03A1:    D9 C4                   fld      st(4)
03A3:    D8 4D DC                fmul     dword ptr [ebp - 0x24]
03A6:    DE C1                   faddp    st(1)
03A8:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
03AA:    D8 4D D8                fmul     dword ptr [ebp - 0x28]
03AD:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
03AF:    D8 4D D4                fmul     dword ptr [ebp - 0x2c]
03B2:    DE C3                   faddp    st(3)
03B4:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
03B6:    D8 4D D0                fmul     dword ptr [ebp - 0x30]
03B9:    DE C2                   faddp    st(2)
03BB:    83 7F 9C 01             cmp      dword ptr [edi - 0x64], 1
03BF:    74 0C                   je       0x3cd    ; Conditional jump
03C1:    DC 67 C8                fsub     qword ptr [edi - 0x38]
03C4:    D9 E1                   fabs    ; |ST(0)|
03C6:    D9 E0                   fchs    ; -ST(0)
03C8:    DC 47 C8                fadd     qword ptr [edi - 0x38]
03CB:    EB 16                   jmp      0x3e3
03CD:    D9 C0                   fld      st(0)
03CF:    DC 47 C8                fadd     qword ptr [edi - 0x38]
03D2:    D9 E1                   fabs    ; |ST(0)|
03D4:    D9 C1                   fld      st(1)
03D6:    DC 67 C8                fsub     qword ptr [edi - 0x38]
03D9:    D9 E1                   fabs    ; |ST(0)|
03DB:    D9 E0                   fchs    ; -ST(0)
03DD:    DE C1                   faddp    st(1)
03DF:    DE E9                   fsubp    st(1)
03E1:    D9 E0                   fchs    ; -ST(0)
03E3:    DC 4F 94                fmul     qword ptr [edi - 0x6c]
03E6:    D9 C0                   fld      st(0)
03E8:    D8 4D DC                fmul     dword ptr [ebp - 0x24]
03EB:    D9 C2                   fld      st(2)
03ED:    D8 4D D0                fmul     dword ptr [ebp - 0x30]
03F0:    DE C1                   faddp    st(1)
03F2:    D9 C3                   fld      st(3)
03F4:    D8 4D C4                fmul     dword ptr [ebp - 0x3c]
03F7:    DE C1                   faddp    st(1)
03F9:    D9 C1                   fld      st(1)
03FB:    D8 4D E4                fmul     dword ptr [ebp - 0x1c]
03FE:    D9 C3                   fld      st(3)
0400:    D8 4D D8                fmul     dword ptr [ebp - 0x28]
0403:    DE C1                   faddp    st(1)
0405:    D9 C4                   fld      st(4)
0407:    D8 4D CC                fmul     dword ptr [ebp - 0x34]
040A:    DE C1                   faddp    st(1)
040C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
040E:    D8 4D E0                fmul     dword ptr [ebp - 0x20]
0411:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0413:    D8 4D D4                fmul     dword ptr [ebp - 0x2c]
0416:    DE C3                   faddp    st(3)
0418:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
041A:    D8 4D C8                fmul     dword ptr [ebp - 0x38]
041D:    DE C2                   faddp    st(2)
041F:    DD 1B                   fstp     qword ptr [ebx]
0421:    DD 1A                   fstp     qword ptr [edx]
0423:    DD 19                   fstp     qword ptr [ecx]
0425:    C3                      ret
