; Disassembly: Ikenagabulb.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 1094 bytes, decoded: 1094 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC A4 00 00 00       sub      esp, 0xa4
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    DD 03                   fld      qword ptr [ebx]
0016:    DD 9D 64 FF FF FF       fstp     qword ptr [ebp - 0x9c]
001C:    DD 02                   fld      qword ptr [edx]
001E:    DD 9D 6C FF FF FF       fstp     qword ptr [ebp - 0x94]
0024:    DD 01                   fld      qword ptr [ecx]
0026:    DD 9D 74 FF FF FF       fstp     qword ptr [ebp - 0x8c]
002C:    D9 D0                   fnop
002E:    DD 46 18                fld      qword ptr [esi + 0x18]
0031:    DD 9D 7C FF FF FF       fstp     qword ptr [ebp - 0x84]
0037:    DD 46 20                fld      qword ptr [esi + 0x20]
003A:    DD 9D 84 FF FF FF       fstp     qword ptr [ebp - 0x7c]
0040:    DD 46 28                fld      qword ptr [esi + 0x28]
0043:    DD 9D 8C FF FF FF       fstp     qword ptr [ebp - 0x74]
0049:    D9 D0                   fnop
004B:    DD 47 F0                fld      qword ptr [edi - 0x10]
004E:    DD 9D 94 FF FF FF       fstp     qword ptr [ebp - 0x6c]
0054:    DD 47 E8                fld      qword ptr [edi - 0x18]
0057:    DD 9D 9C FF FF FF       fstp     qword ptr [ebp - 0x64]
005D:    DD 47 E0                fld      qword ptr [edi - 0x20]
0060:    DD 9D A4 FF FF FF       fstp     qword ptr [ebp - 0x5c]
0066:    D9 D0                   fnop
0068:    DD 47 D8                fld      qword ptr [edi - 0x28]
006B:    DD 9D AC FF FF FF       fstp     qword ptr [ebp - 0x54]
0071:    DD 47 D0                fld      qword ptr [edi - 0x30]
0074:    DD 9D B4 FF FF FF       fstp     qword ptr [ebp - 0x4c]
007A:    DD 47 C8                fld      qword ptr [edi - 0x38]
007D:    DD 9D BC FF FF FF       fstp     qword ptr [ebp - 0x44]
0083:    D9 D0                   fnop
0085:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
008B:    DC 8D 64 FF FF FF       fmul     qword ptr [ebp - 0x9c]
0091:    DD 85 6C FF FF FF       fld      qword ptr [ebp - 0x94]
0097:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
009D:    DE C1                   faddp    st(1)
009F:    D9 FA                   fsqrt    ; sqrt(ST(0))
00A1:    DD 5D EC                fstp     qword ptr [ebp - 0x14]
00A4:    DD 45 94                fld      qword ptr [ebp - 0x6c]
00A7:    DC 4D 94                fmul     qword ptr [ebp - 0x6c]
00AA:    DD 45 9C                fld      qword ptr [ebp - 0x64]
00AD:    DC 4D 9C                fmul     qword ptr [ebp - 0x64]
00B0:    DE C1                   faddp    st(1)
00B2:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B4:    DD 5D F4                fstp     qword ptr [ebp - 0xc]
00B7:    DD 45 EC                fld      qword ptr [ebp - 0x14]
00BA:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
00BD:    D9 EE                   fldz    ; Load 0.0
00BF:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00C1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C3:    9E                      sahf
00C4:    75 1F                   jne      0xe5    ; Conditional jump
00C6:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
00CC:    D9 E0                   fchs    ; -ST(0)
00CE:    DC 4D A4                fmul     qword ptr [ebp - 0x5c]
00D1:    DD 5D C4                fstp     qword ptr [ebp - 0x3c]
00D4:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
00DA:    DC 4D A4                fmul     qword ptr [ebp - 0x5c]
00DD:    DD 9D 5C FF FF FF       fstp     qword ptr [ebp - 0xa4]
00E3:    EB 5F                   jmp      0x144
00E5:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
00EB:    DC 4D A4                fmul     qword ptr [ebp - 0x5c]
00EE:    DD 45 EC                fld      qword ptr [ebp - 0x14]
00F1:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
00F4:    DE F9                   fdivp    st(1)
00F6:    D9 E8                   fld1    ; Load 1.0
00F8:    DE E1                   fsubrp   st(1)
00FA:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
0100:    DC 4D 94                fmul     qword ptr [ebp - 0x6c]
0103:    DD 85 6C FF FF FF       fld      qword ptr [ebp - 0x94]
0109:    DC 4D 9C                fmul     qword ptr [ebp - 0x64]
010C:    DE E9                   fsubp    st(1)
010E:    DE C9                   fmulp    st(1)
0110:    DD 5D C4                fstp     qword ptr [ebp - 0x3c]
0113:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
0119:    DC 4D A4                fmul     qword ptr [ebp - 0x5c]
011C:    DD 45 EC                fld      qword ptr [ebp - 0x14]
011F:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
0122:    DE F9                   fdivp    st(1)
0124:    D9 E8                   fld1    ; Load 1.0
0126:    DE E1                   fsubrp   st(1)
0128:    DD 45 94                fld      qword ptr [ebp - 0x6c]
012B:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
0131:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
0137:    DC 4D 9C                fmul     qword ptr [ebp - 0x64]
013A:    DE C1                   faddp    st(1)
013C:    DE C9                   fmulp    st(1)
013E:    DD 9D 5C FF FF FF       fstp     qword ptr [ebp - 0xa4]
0144:    DD 45 EC                fld      qword ptr [ebp - 0x14]
0147:    DC 4D A4                fmul     qword ptr [ebp - 0x5c]
014A:    DD 45 F4                fld      qword ptr [ebp - 0xc]
014D:    DC 8D 74 FF FF FF       fmul     qword ptr [ebp - 0x8c]
0153:    DE C1                   faddp    st(1)
0155:    DD 5D CC                fstp     qword ptr [ebp - 0x34]
0158:    DD 45 C4                fld      qword ptr [ebp - 0x3c]
015B:    DC 4D C4                fmul     qword ptr [ebp - 0x3c]
015E:    DD 85 5C FF FF FF       fld      qword ptr [ebp - 0xa4]
0164:    DC 8D 5C FF FF FF       fmul     qword ptr [ebp - 0xa4]
016A:    DE C1                   faddp    st(1)
016C:    D9 FA                   fsqrt    ; sqrt(ST(0))
016E:    DD 5D EC                fstp     qword ptr [ebp - 0x14]
0171:    DD 85 7C FF FF FF       fld      qword ptr [ebp - 0x84]
0177:    DC 8D 7C FF FF FF       fmul     qword ptr [ebp - 0x84]
017D:    DD 45 84                fld      qword ptr [ebp - 0x7c]
0180:    DC 4D 84                fmul     qword ptr [ebp - 0x7c]
0183:    DE C1                   faddp    st(1)
0185:    D9 FA                   fsqrt    ; sqrt(ST(0))
0187:    DD 5D F4                fstp     qword ptr [ebp - 0xc]
018A:    DD 45 EC                fld      qword ptr [ebp - 0x14]
018D:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
0190:    D9 EE                   fldz    ; Load 0.0
0192:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0194:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0196:    9E                      sahf
0197:    75 16                   jne      0x1af    ; Conditional jump
0199:    DD 45 CC                fld      qword ptr [ebp - 0x34]
019C:    D9 E0                   fchs    ; -ST(0)
019E:    DC 4D 8C                fmul     qword ptr [ebp - 0x74]
01A1:    DD 5D D4                fstp     qword ptr [ebp - 0x2c]
01A4:    DD 45 CC                fld      qword ptr [ebp - 0x34]
01A7:    DC 4D 8C                fmul     qword ptr [ebp - 0x74]
01AA:    DD 5D DC                fstp     qword ptr [ebp - 0x24]
01AD:    EB 56                   jmp      0x205
01AF:    DD 45 CC                fld      qword ptr [ebp - 0x34]
01B2:    DC 4D 8C                fmul     qword ptr [ebp - 0x74]
01B5:    DD 45 EC                fld      qword ptr [ebp - 0x14]
01B8:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
01BB:    DE F9                   fdivp    st(1)
01BD:    D9 E8                   fld1    ; Load 1.0
01BF:    DE E1                   fsubrp   st(1)
01C1:    DD 45 C4                fld      qword ptr [ebp - 0x3c]
01C4:    DC 8D 7C FF FF FF       fmul     qword ptr [ebp - 0x84]
01CA:    DD 85 5C FF FF FF       fld      qword ptr [ebp - 0xa4]
01D0:    DC 4D 84                fmul     qword ptr [ebp - 0x7c]
01D3:    DE E9                   fsubp    st(1)
01D5:    DE C9                   fmulp    st(1)
01D7:    DD 5D D4                fstp     qword ptr [ebp - 0x2c]
01DA:    DD 45 CC                fld      qword ptr [ebp - 0x34]
01DD:    DC 4D 8C                fmul     qword ptr [ebp - 0x74]
01E0:    DD 45 EC                fld      qword ptr [ebp - 0x14]
01E3:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
01E6:    DE F9                   fdivp    st(1)
01E8:    D9 E8                   fld1    ; Load 1.0
01EA:    DE E1                   fsubrp   st(1)
01EC:    DD 85 7C FF FF FF       fld      qword ptr [ebp - 0x84]
01F2:    DC 8D 5C FF FF FF       fmul     qword ptr [ebp - 0xa4]
01F8:    DD 45 CC                fld      qword ptr [ebp - 0x34]
01FB:    DC 4D 84                fmul     qword ptr [ebp - 0x7c]
01FE:    DE C1                   faddp    st(1)
0200:    DE C9                   fmulp    st(1)
0202:    DD 5D DC                fstp     qword ptr [ebp - 0x24]
0205:    DD 45 EC                fld      qword ptr [ebp - 0x14]
0208:    DC 4D 8C                fmul     qword ptr [ebp - 0x74]
020B:    DD 45 F4                fld      qword ptr [ebp - 0xc]
020E:    DC 4D CC                fmul     qword ptr [ebp - 0x34]
0211:    DE C1                   faddp    st(1)
0213:    DD 5D E4                fstp     qword ptr [ebp - 0x1c]
0216:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
021C:    DC 8D 64 FF FF FF       fmul     qword ptr [ebp - 0x9c]
0222:    DD 85 6C FF FF FF       fld      qword ptr [ebp - 0x94]
0228:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
022E:    DE C1                   faddp    st(1)
0230:    D9 FA                   fsqrt    ; sqrt(ST(0))
0232:    DD 5D EC                fstp     qword ptr [ebp - 0x14]
0235:    DD 45 AC                fld      qword ptr [ebp - 0x54]
0238:    DC 4D AC                fmul     qword ptr [ebp - 0x54]
023B:    DD 45 B4                fld      qword ptr [ebp - 0x4c]
023E:    DC 4D B4                fmul     qword ptr [ebp - 0x4c]
0241:    DE C1                   faddp    st(1)
0243:    D9 FA                   fsqrt    ; sqrt(ST(0))
0245:    DD 5D F4                fstp     qword ptr [ebp - 0xc]
0248:    DD 45 EC                fld      qword ptr [ebp - 0x14]
024B:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
024E:    D9 EE                   fldz    ; Load 0.0
0250:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0252:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0254:    9E                      sahf
0255:    75 1F                   jne      0x276    ; Conditional jump
0257:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
025D:    D9 E0                   fchs    ; -ST(0)
025F:    DC 4D BC                fmul     qword ptr [ebp - 0x44]
0262:    DD 5D C4                fstp     qword ptr [ebp - 0x3c]
0265:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
026B:    DC 4D BC                fmul     qword ptr [ebp - 0x44]
026E:    DD 9D 5C FF FF FF       fstp     qword ptr [ebp - 0xa4]
0274:    EB 5F                   jmp      0x2d5
0276:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
027C:    DC 4D BC                fmul     qword ptr [ebp - 0x44]
027F:    DD 45 EC                fld      qword ptr [ebp - 0x14]
0282:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
0285:    DE F9                   fdivp    st(1)
0287:    D9 E8                   fld1    ; Load 1.0
0289:    DE E1                   fsubrp   st(1)
028B:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
0291:    DC 4D AC                fmul     qword ptr [ebp - 0x54]
0294:    DD 85 6C FF FF FF       fld      qword ptr [ebp - 0x94]
029A:    DC 4D B4                fmul     qword ptr [ebp - 0x4c]
029D:    DE E9                   fsubp    st(1)
029F:    DE C9                   fmulp    st(1)
02A1:    DD 5D C4                fstp     qword ptr [ebp - 0x3c]
02A4:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
02AA:    DC 4D BC                fmul     qword ptr [ebp - 0x44]
02AD:    DD 45 EC                fld      qword ptr [ebp - 0x14]
02B0:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
02B3:    DE F9                   fdivp    st(1)
02B5:    D9 E8                   fld1    ; Load 1.0
02B7:    DE E1                   fsubrp   st(1)
02B9:    DD 45 AC                fld      qword ptr [ebp - 0x54]
02BC:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
02C2:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
02C8:    DC 4D B4                fmul     qword ptr [ebp - 0x4c]
02CB:    DE C1                   faddp    st(1)
02CD:    DE C9                   fmulp    st(1)
02CF:    DD 9D 5C FF FF FF       fstp     qword ptr [ebp - 0xa4]
02D5:    DD 45 EC                fld      qword ptr [ebp - 0x14]
02D8:    DC 4D BC                fmul     qword ptr [ebp - 0x44]
02DB:    DD 45 F4                fld      qword ptr [ebp - 0xc]
02DE:    DC 8D 74 FF FF FF       fmul     qword ptr [ebp - 0x8c]
02E4:    DE C1                   faddp    st(1)
02E6:    DD 5D CC                fstp     qword ptr [ebp - 0x34]
02E9:    DD 45 D4                fld      qword ptr [ebp - 0x2c]
02EC:    DC 65 C4                fsub     qword ptr [ebp - 0x3c]
02EF:    DC A5 7C FF FF FF       fsub     qword ptr [ebp - 0x84]
02F5:    DD 5D C4                fstp     qword ptr [ebp - 0x3c]
02F8:    DD 45 DC                fld      qword ptr [ebp - 0x24]
02FB:    DC A5 5C FF FF FF       fsub     qword ptr [ebp - 0xa4]
0301:    DC 65 84                fsub     qword ptr [ebp - 0x7c]
0304:    DD 9D 5C FF FF FF       fstp     qword ptr [ebp - 0xa4]
030A:    DD 45 E4                fld      qword ptr [ebp - 0x1c]
030D:    DC 65 CC                fsub     qword ptr [ebp - 0x34]
0310:    DC 65 8C                fsub     qword ptr [ebp - 0x74]
0313:    DD 5D CC                fstp     qword ptr [ebp - 0x34]
0316:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
031C:    DC 8D 64 FF FF FF       fmul     qword ptr [ebp - 0x9c]
0322:    DD 85 6C FF FF FF       fld      qword ptr [ebp - 0x94]
0328:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
032E:    DE C1                   faddp    st(1)
0330:    DD 5D EC                fstp     qword ptr [ebp - 0x14]
0333:    F7 45 F0 FF FF FF 7F    test     dword ptr [ebp - 0x10], 0x7fffffff
033A:    75 24                   jne      0x360    ; Conditional jump
033C:    83 7D EC 00             cmp      dword ptr [ebp - 0x14], 0
0340:    75 1E                   jne      0x360    ; Conditional jump
0342:    C7 45 D4 00 00 00 00    mov      dword ptr [ebp - 0x2c], 0
0349:    C7 45 D8 00 00 00 00    mov      dword ptr [ebp - 0x28], 0
0350:    C7 45 DC 00 00 00 00    mov      dword ptr [ebp - 0x24], 0
0357:    C7 45 E0 00 00 00 00    mov      dword ptr [ebp - 0x20], 0
035E:    EB 78                   jmp      0x3d8
0360:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
0366:    DC 0F                   fmul     qword ptr [edi]
0368:    90                      nop
0369:    90                      nop
036A:    90                      nop
036B:    90                      nop
036C:    DC 8D 74 FF FF FF       fmul     qword ptr [ebp - 0x8c]
0372:    DC 75 EC                fdiv     qword ptr [ebp - 0x14]
0375:    D9 E8                   fld1    ; Load 1.0
0377:    DE E1                   fsubrp   st(1)
0379:    DD 5D F4                fstp     qword ptr [ebp - 0xc]
037C:    DD 85 6C FF FF FF       fld      qword ptr [ebp - 0x94]
0382:    DC 0F                   fmul     qword ptr [edi]
0384:    90                      nop
0385:    90                      nop
0386:    90                      nop
0387:    90                      nop
0388:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
038E:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
0394:    DC 8D 64 FF FF FF       fmul     qword ptr [ebp - 0x9c]
039A:    DE E1                   fsubrp   st(1)
039C:    DD 45 F4                fld      qword ptr [ebp - 0xc]
039F:    DC 8D 64 FF FF FF       fmul     qword ptr [ebp - 0x9c]
03A5:    DE C9                   fmulp    st(1)
03A7:    DD 5D D4                fstp     qword ptr [ebp - 0x2c]
03AA:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
03B0:    DC 0F                   fmul     qword ptr [edi]
03B2:    90                      nop
03B3:    90                      nop
03B4:    90                      nop
03B5:    90                      nop
03B6:    DC 8D 64 FF FF FF       fmul     qword ptr [ebp - 0x9c]
03BC:    DD 85 6C FF FF FF       fld      qword ptr [ebp - 0x94]
03C2:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
03C8:    DE E9                   fsubp    st(1)
03CA:    DD 45 F4                fld      qword ptr [ebp - 0xc]
03CD:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
03D3:    DE C9                   fmulp    st(1)
03D5:    DD 5D DC                fstp     qword ptr [ebp - 0x24]
03D8:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
03DE:    DC 8D 74 FF FF FF       fmul     qword ptr [ebp - 0x8c]
03E4:    DD 45 EC                fld      qword ptr [ebp - 0x14]
03E7:    DC 0F                   fmul     qword ptr [edi]
03E9:    90                      nop
03EA:    90                      nop
03EB:    90                      nop
03EC:    90                      nop
03ED:    DE E1                   fsubrp   st(1)
03EF:    DC 8D 74 FF FF FF       fmul     qword ptr [ebp - 0x8c]
03F5:    DD 5D CC                fstp     qword ptr [ebp - 0x34]
03F8:    DD 45 C4                fld      qword ptr [ebp - 0x3c]
03FB:    DC 45 D4                fadd     qword ptr [ebp - 0x2c]
03FE:    DD 9D 64 FF FF FF       fstp     qword ptr [ebp - 0x9c]
0404:    DD 85 5C FF FF FF       fld      qword ptr [ebp - 0xa4]
040A:    DC 45 DC                fadd     qword ptr [ebp - 0x24]
040D:    DD 9D 6C FF FF FF       fstp     qword ptr [ebp - 0x94]
0413:    DD 45 CC                fld      qword ptr [ebp - 0x34]
0416:    DC 45 E4                fadd     qword ptr [ebp - 0x1c]
0419:    DD 9D 74 FF FF FF       fstp     qword ptr [ebp - 0x8c]
041F:    D9 D0                   fnop
0421:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
0427:    DD 1B                   fstp     qword ptr [ebx]
0429:    DD 85 6C FF FF FF       fld      qword ptr [ebp - 0x94]
042F:    DD 1A                   fstp     qword ptr [edx]
0431:    DD 85 74 FF FF FF       fld      qword ptr [ebp - 0x8c]
0437:    DD 19                   fstp     qword ptr [ecx]
0439:    D9 D0                   fnop
043B:    8B C3                   mov      eax, ebx
043D:    5F                      pop      edi
043E:    5E                      pop      esi
043F:    5B                      pop      ebx
0440:    8B E5                   mov      esp, ebp
0442:    5D                      pop      ebp
0443:    C2 08 00                ret      8
