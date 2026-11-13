; Disassembly: ATetraVS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 884 bytes, decoded: 884 bytes (100.0%)
; DEoption: 11

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 3C 00 00 00       sub      esp, 0x3c
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    81 C6 80 00 00 00       add      esi, 0x80
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
0060:    83 7F A4 01             cmp      dword ptr [edi - 0x5c], 1
0064:    74 0C                   je       0x72    ; Conditional jump
0066:    DC 67 D8                fsub     qword ptr [edi - 0x28]
0069:    D9 E1                   fabs    ; |ST(0)|
006B:    D9 E0                   fchs    ; -ST(0)
006D:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0070:    EB 18                   jmp      0x8a
0072:    D9 C0                   fld      st(0)
0074:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0077:    D9 E1                   fabs    ; |ST(0)|
0079:    D9 C1                   fld      st(1)
007B:    DC 67 D8                fsub     qword ptr [edi - 0x28]
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
00CB:    D9 47 CC                fld      dword ptr [edi - 0x34]
00CE:    D9 5D E4                fstp     dword ptr [ebp - 0x1c]
00D1:    D9 47 C8                fld      dword ptr [edi - 0x38]
00D4:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
00D7:    D9 47 C4                fld      dword ptr [edi - 0x3c]
00DA:    D9 5D DC                fstp     dword ptr [ebp - 0x24]
00DD:    D9 47 C0                fld      dword ptr [edi - 0x40]
00E0:    D9 5D D8                fstp     dword ptr [ebp - 0x28]
00E3:    D9 47 BC                fld      dword ptr [edi - 0x44]
00E6:    D9 5D D4                fstp     dword ptr [ebp - 0x2c]
00E9:    D9 47 B8                fld      dword ptr [edi - 0x48]
00EC:    D9 5D D0                fstp     dword ptr [ebp - 0x30]
00EF:    D9 47 B4                fld      dword ptr [edi - 0x4c]
00F2:    D9 5D CC                fstp     dword ptr [ebp - 0x34]
00F5:    D9 47 B0                fld      dword ptr [edi - 0x50]
00F8:    D9 5D C8                fstp     dword ptr [ebp - 0x38]
00FB:    D9 47 AC                fld      dword ptr [edi - 0x54]
00FE:    D9 5D C4                fstp     dword ptr [ebp - 0x3c]
0101:    E8 1B FF FF FF          call     0x21
0106:    83 7F A8 00             cmp      dword ptr [edi - 0x58], 0
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
0180:    83 7F A8 01             cmp      dword ptr [edi - 0x58], 1
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
01FA:    83 7F A8 02             cmp      dword ptr [edi - 0x58], 2
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
02E8:    83 7E 50 00             cmp      dword ptr [esi + 0x50], 0
02EC:    7F 09                   jg       0x2f7    ; Conditional jump
02EE:    FF 46 50                inc      dword ptr [esi + 0x50]
02F1:    DD 47 F0                fld      qword ptr [edi - 0x10]
02F4:    DD 5E 48                fstp     qword ptr [esi + 0x48]
02F7:    DD 46 48                fld      qword ptr [esi + 0x48]
02FA:    D9 E8                   fld1    ; Load 1.0
02FC:    D9 C1                   fld      st(1)
02FE:    D9 E1                   fabs    ; |ST(0)|
0300:    DE E1                   fsubrp   st(1)
0302:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0305:    DE C1                   faddp    st(1)
0307:    DD 5E 48                fstp     qword ptr [esi + 0x48]
030A:    90                      nop
030B:    90                      nop
030C:    DD 03                   fld      qword ptr [ebx]
030E:    DD 02                   fld      qword ptr [edx]
0310:    DD 01                   fld      qword ptr [ecx]
0312:    90                      nop
0313:    DD 03                   fld      qword ptr [ebx]
0315:    D8 C8                   fmul     st(0)
0317:    DD 02                   fld      qword ptr [edx]
0319:    D8 C8                   fmul     st(0)
031B:    DD 01                   fld      qword ptr [ecx]
031D:    D8 C8                   fmul     st(0)
031F:    DE C1                   faddp    st(1)
0321:    DE C1                   faddp    st(1)
0323:    90                      nop
0324:    DC 57 E0                fcom     qword ptr [edi - 0x20]
0327:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0329:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
032B:    73 07                   jae      0x334    ; Conditional jump
032D:    DD D8                   fstp     st(0)
032F:    DD 47 E8                fld      qword ptr [edi - 0x18]
0332:    EB 16                   jmp      0x34a
0334:    D9 E8                   fld1    ; Load 1.0
0336:    D8 D1                   fcom     st(1)
0338:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
033A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
033C:    72 07                   jb       0x345    ; Conditional jump
033E:    DD D8                   fstp     st(0)
0340:    DC 7E 48                fdivr    qword ptr [esi + 0x48]
0343:    EB 05                   jmp      0x34a
0345:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0347:    DD 46 48                fld      qword ptr [esi + 0x48]
034A:    DD 41 08                fld      qword ptr [ecx + 8]
034D:    D8 C9                   fmul     st(1)
034F:    DD 59 08                fstp     qword ptr [ecx + 8]
0352:    DC CB                   fmul     st(3), st(0)
0354:    DC CA                   fmul     st(2), st(0)
0356:    DE C9                   fmulp    st(1)
0358:    DC 46 A8                fadd     qword ptr [esi - 0x58]
035B:    DD 19                   fstp     qword ptr [ecx]
035D:    DC 46 A0                fadd     qword ptr [esi - 0x60]
0360:    DD 1A                   fstp     qword ptr [edx]
0362:    DC 46 98                fadd     qword ptr [esi - 0x68]
0365:    DD 1B                   fstp     qword ptr [ebx]
0367:    D9 D0                   fnop
0369:    8B C3                   mov      eax, ebx
036B:    5F                      pop      edi
036C:    5E                      pop      esi
036D:    5B                      pop      ebx
036E:    8B E5                   mov      esp, ebp
0370:    5D                      pop      ebp
0371:    C2 08 00                ret      8
