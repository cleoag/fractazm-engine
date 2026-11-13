; Disassembly: _MobiusTpx.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 953 bytes, decoded: 953 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC C4 00 00 00       sub      esp, 0xc4
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    DD 03                   fld      qword ptr [ebx]
0016:    DD 9D 44 FF FF FF       fstp     qword ptr [ebp - 0xbc]
001C:    DD 02                   fld      qword ptr [edx]
001E:    DD 9D 4C FF FF FF       fstp     qword ptr [ebp - 0xb4]
0024:    DD 01                   fld      qword ptr [ecx]
0026:    DD 9D 54 FF FF FF       fstp     qword ptr [ebp - 0xac]
002C:    D9 D0                   fnop
002E:    DD 47 F0                fld      qword ptr [edi - 0x10]
0031:    DD 9D 5C FF FF FF       fstp     qword ptr [ebp - 0xa4]
0037:    DD 47 E8                fld      qword ptr [edi - 0x18]
003A:    DD 9D 64 FF FF FF       fstp     qword ptr [ebp - 0x9c]
0040:    DD 47 E0                fld      qword ptr [edi - 0x20]
0043:    DD 9D 6C FF FF FF       fstp     qword ptr [ebp - 0x94]
0049:    DD 47 D8                fld      qword ptr [edi - 0x28]
004C:    DD 9D 74 FF FF FF       fstp     qword ptr [ebp - 0x8c]
0052:    DD 47 D0                fld      qword ptr [edi - 0x30]
0055:    DD 9D 7C FF FF FF       fstp     qword ptr [ebp - 0x84]
005B:    DD 47 C8                fld      qword ptr [edi - 0x38]
005E:    DD 9D 84 FF FF FF       fstp     qword ptr [ebp - 0x7c]
0064:    DD 47 C0                fld      qword ptr [edi - 0x40]
0067:    DD 9D 8C FF FF FF       fstp     qword ptr [ebp - 0x74]
006D:    DD 47 B8                fld      qword ptr [edi - 0x48]
0070:    DD 9D 94 FF FF FF       fstp     qword ptr [ebp - 0x6c]
0076:    DD 47 B0                fld      qword ptr [edi - 0x50]
0079:    DD 9D 9C FF FF FF       fstp     qword ptr [ebp - 0x64]
007F:    DD 47 A8                fld      qword ptr [edi - 0x58]
0082:    DD 9D A4 FF FF FF       fstp     qword ptr [ebp - 0x5c]
0088:    DD 47 A0                fld      qword ptr [edi - 0x60]
008B:    DD 9D AC FF FF FF       fstp     qword ptr [ebp - 0x54]
0091:    DD 47 98                fld      qword ptr [edi - 0x68]
0094:    DD 9D B4 FF FF FF       fstp     qword ptr [ebp - 0x4c]
009A:    D9 D0                   fnop
009C:    DD 85 44 FF FF FF       fld      qword ptr [ebp - 0xbc]
00A2:    DC 8D 44 FF FF FF       fmul     qword ptr [ebp - 0xbc]
00A8:    DD 85 4C FF FF FF       fld      qword ptr [ebp - 0xb4]
00AE:    DC 8D 4C FF FF FF       fmul     qword ptr [ebp - 0xb4]
00B4:    DE C1                   faddp    st(1)
00B6:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B8:    DD 5D EC                fstp     qword ptr [ebp - 0x14]
00BB:    DD 45 8C                fld      qword ptr [ebp - 0x74]
00BE:    DC 4D 8C                fmul     qword ptr [ebp - 0x74]
00C1:    DD 45 94                fld      qword ptr [ebp - 0x6c]
00C4:    DC 4D 94                fmul     qword ptr [ebp - 0x6c]
00C7:    DE C1                   faddp    st(1)
00C9:    D9 FA                   fsqrt    ; sqrt(ST(0))
00CB:    DD 5D F4                fstp     qword ptr [ebp - 0xc]
00CE:    DD 45 EC                fld      qword ptr [ebp - 0x14]
00D1:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
00D4:    D9 EE                   fldz    ; Load 0.0
00D6:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00D8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DA:    9E                      sahf
00DB:    75 1C                   jne      0xf9    ; Conditional jump
00DD:    DD 85 54 FF FF FF       fld      qword ptr [ebp - 0xac]
00E3:    D9 E0                   fchs    ; -ST(0)
00E5:    DC 4D 9C                fmul     qword ptr [ebp - 0x64]
00E8:    DD 5D CC                fstp     qword ptr [ebp - 0x34]
00EB:    DD 85 54 FF FF FF       fld      qword ptr [ebp - 0xac]
00F1:    DC 4D 9C                fmul     qword ptr [ebp - 0x64]
00F4:    DD 5D D4                fstp     qword ptr [ebp - 0x2c]
00F7:    EB 5C                   jmp      0x155
00F9:    DD 85 54 FF FF FF       fld      qword ptr [ebp - 0xac]
00FF:    DC 4D 9C                fmul     qword ptr [ebp - 0x64]
0102:    DD 45 EC                fld      qword ptr [ebp - 0x14]
0105:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
0108:    DE F9                   fdivp    st(1)
010A:    D9 E8                   fld1    ; Load 1.0
010C:    DE E1                   fsubrp   st(1)
010E:    DD 85 44 FF FF FF       fld      qword ptr [ebp - 0xbc]
0114:    DC 4D 8C                fmul     qword ptr [ebp - 0x74]
0117:    DD 85 4C FF FF FF       fld      qword ptr [ebp - 0xb4]
011D:    DC 4D 94                fmul     qword ptr [ebp - 0x6c]
0120:    DE E9                   fsubp    st(1)
0122:    DE C9                   fmulp    st(1)
0124:    DD 5D CC                fstp     qword ptr [ebp - 0x34]
0127:    DD 85 54 FF FF FF       fld      qword ptr [ebp - 0xac]
012D:    DC 4D 9C                fmul     qword ptr [ebp - 0x64]
0130:    DD 45 EC                fld      qword ptr [ebp - 0x14]
0133:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
0136:    DE F9                   fdivp    st(1)
0138:    D9 E8                   fld1    ; Load 1.0
013A:    DE E1                   fsubrp   st(1)
013C:    DD 45 8C                fld      qword ptr [ebp - 0x74]
013F:    DC 8D 4C FF FF FF       fmul     qword ptr [ebp - 0xb4]
0145:    DD 85 44 FF FF FF       fld      qword ptr [ebp - 0xbc]
014B:    DC 4D 94                fmul     qword ptr [ebp - 0x6c]
014E:    DE C1                   faddp    st(1)
0150:    DE C9                   fmulp    st(1)
0152:    DD 5D D4                fstp     qword ptr [ebp - 0x2c]
0155:    DD 45 EC                fld      qword ptr [ebp - 0x14]
0158:    DC 4D 9C                fmul     qword ptr [ebp - 0x64]
015B:    DD 45 F4                fld      qword ptr [ebp - 0xc]
015E:    DC 8D 54 FF FF FF       fmul     qword ptr [ebp - 0xac]
0164:    DE C1                   faddp    st(1)
0166:    DD 5D DC                fstp     qword ptr [ebp - 0x24]
0169:    DD 45 CC                fld      qword ptr [ebp - 0x34]
016C:    DC 45 A4                fadd     qword ptr [ebp - 0x5c]
016F:    DD 5D CC                fstp     qword ptr [ebp - 0x34]
0172:    DD 45 D4                fld      qword ptr [ebp - 0x2c]
0175:    DC 45 AC                fadd     qword ptr [ebp - 0x54]
0178:    DD 5D D4                fstp     qword ptr [ebp - 0x2c]
017B:    DD 45 DC                fld      qword ptr [ebp - 0x24]
017E:    DC 45 B4                fadd     qword ptr [ebp - 0x4c]
0181:    DD 5D DC                fstp     qword ptr [ebp - 0x24]
0184:    DD 45 CC                fld      qword ptr [ebp - 0x34]
0187:    DC 4D CC                fmul     qword ptr [ebp - 0x34]
018A:    DD 45 D4                fld      qword ptr [ebp - 0x2c]
018D:    DC 4D D4                fmul     qword ptr [ebp - 0x2c]
0190:    DE C1                   faddp    st(1)
0192:    DD 45 DC                fld      qword ptr [ebp - 0x24]
0195:    DC 4D DC                fmul     qword ptr [ebp - 0x24]
0198:    DE C1                   faddp    st(1)
019A:    DD 55 E4                fst      qword ptr [ebp - 0x1c]
019D:    DC 1F                   fcomp    qword ptr [edi]
019F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01A1:    9E                      sahf
01A2:    77 1A                   ja       0x1be    ; Conditional jump
01A4:    DD 47 08                fld      qword ptr [edi + 8]
01A7:    DD 95 44 FF FF FF       fst      qword ptr [ebp - 0xbc]
01AD:    DD 95 4C FF FF FF       fst      qword ptr [ebp - 0xb4]
01B3:    DD 9D 54 FF FF FF       fstp     qword ptr [ebp - 0xac]
01B9:    E9 D6 01 00 00          jmp      0x394
01BE:    DD 85 5C FF FF FF       fld      qword ptr [ebp - 0xa4]
01C4:    DC 8D 5C FF FF FF       fmul     qword ptr [ebp - 0xa4]
01CA:    DD 85 64 FF FF FF       fld      qword ptr [ebp - 0x9c]
01D0:    DC 8D 64 FF FF FF       fmul     qword ptr [ebp - 0x9c]
01D6:    DE C1                   faddp    st(1)
01D8:    D9 FA                   fsqrt    ; sqrt(ST(0))
01DA:    DD 5D F4                fstp     qword ptr [ebp - 0xc]
01DD:    DD 45 EC                fld      qword ptr [ebp - 0x14]
01E0:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
01E3:    D9 EE                   fldz    ; Load 0.0
01E5:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
01E7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01E9:    9E                      sahf
01EA:    75 25                   jne      0x211    ; Conditional jump
01EC:    DD 85 54 FF FF FF       fld      qword ptr [ebp - 0xac]
01F2:    D9 E0                   fchs    ; -ST(0)
01F4:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
01FA:    DD 9D 3C FF FF FF       fstp     qword ptr [ebp - 0xc4]
0200:    DD 85 54 FF FF FF       fld      qword ptr [ebp - 0xac]
0206:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
020C:    DD 5D BC                fstp     qword ptr [ebp - 0x44]
020F:    EB 71                   jmp      0x282
0211:    DD 85 54 FF FF FF       fld      qword ptr [ebp - 0xac]
0217:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
021D:    DD 45 EC                fld      qword ptr [ebp - 0x14]
0220:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
0223:    DE F9                   fdivp    st(1)
0225:    D9 E8                   fld1    ; Load 1.0
0227:    DE E1                   fsubrp   st(1)
0229:    DD 85 44 FF FF FF       fld      qword ptr [ebp - 0xbc]
022F:    DC 8D 5C FF FF FF       fmul     qword ptr [ebp - 0xa4]
0235:    DD 85 4C FF FF FF       fld      qword ptr [ebp - 0xb4]
023B:    DC 8D 64 FF FF FF       fmul     qword ptr [ebp - 0x9c]
0241:    DE E9                   fsubp    st(1)
0243:    DE C9                   fmulp    st(1)
0245:    DD 9D 3C FF FF FF       fstp     qword ptr [ebp - 0xc4]
024B:    DD 85 54 FF FF FF       fld      qword ptr [ebp - 0xac]
0251:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
0257:    DD 45 EC                fld      qword ptr [ebp - 0x14]
025A:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
025D:    DE F9                   fdivp    st(1)
025F:    D9 E8                   fld1    ; Load 1.0
0261:    DE E1                   fsubrp   st(1)
0263:    DD 85 5C FF FF FF       fld      qword ptr [ebp - 0xa4]
0269:    DC 8D 4C FF FF FF       fmul     qword ptr [ebp - 0xb4]
026F:    DD 85 44 FF FF FF       fld      qword ptr [ebp - 0xbc]
0275:    DC 8D 64 FF FF FF       fmul     qword ptr [ebp - 0x9c]
027B:    DE C1                   faddp    st(1)
027D:    DE C9                   fmulp    st(1)
027F:    DD 5D BC                fstp     qword ptr [ebp - 0x44]
0282:    DD 45 EC                fld      qword ptr [ebp - 0x14]
0285:    DC 8D 6C FF FF FF       fmul     qword ptr [ebp - 0x94]
028B:    DD 45 F4                fld      qword ptr [ebp - 0xc]
028E:    DC 8D 54 FF FF FF       fmul     qword ptr [ebp - 0xac]
0294:    DE C1                   faddp    st(1)
0296:    DD 5D C4                fstp     qword ptr [ebp - 0x3c]
0299:    DD 85 3C FF FF FF       fld      qword ptr [ebp - 0xc4]
029F:    DC 85 74 FF FF FF       fadd     qword ptr [ebp - 0x8c]
02A5:    DD 9D 3C FF FF FF       fstp     qword ptr [ebp - 0xc4]
02AB:    DD 45 BC                fld      qword ptr [ebp - 0x44]
02AE:    DC 85 7C FF FF FF       fadd     qword ptr [ebp - 0x84]
02B4:    DD 5D BC                fstp     qword ptr [ebp - 0x44]
02B7:    DD 45 C4                fld      qword ptr [ebp - 0x3c]
02BA:    DC 45 84                fadd     qword ptr [ebp - 0x7c]
02BD:    DD 5D C4                fstp     qword ptr [ebp - 0x3c]
02C0:    D9 E8                   fld1    ; Load 1.0
02C2:    DC 75 E4                fdiv     qword ptr [ebp - 0x1c]
02C5:    DD 5D E4                fstp     qword ptr [ebp - 0x1c]
02C8:    DD 85 3C FF FF FF       fld      qword ptr [ebp - 0xc4]
02CE:    DC 8D 3C FF FF FF       fmul     qword ptr [ebp - 0xc4]
02D4:    DD 45 BC                fld      qword ptr [ebp - 0x44]
02D7:    DC 4D BC                fmul     qword ptr [ebp - 0x44]
02DA:    DE C1                   faddp    st(1)
02DC:    D9 FA                   fsqrt    ; sqrt(ST(0))
02DE:    DD 5D EC                fstp     qword ptr [ebp - 0x14]
02E1:    DD 45 CC                fld      qword ptr [ebp - 0x34]
02E4:    DC 4D CC                fmul     qword ptr [ebp - 0x34]
02E7:    DD 45 D4                fld      qword ptr [ebp - 0x2c]
02EA:    DC 4D D4                fmul     qword ptr [ebp - 0x2c]
02ED:    DE C1                   faddp    st(1)
02EF:    D9 FA                   fsqrt    ; sqrt(ST(0))
02F1:    DD 5D F4                fstp     qword ptr [ebp - 0xc]
02F4:    DD 45 EC                fld      qword ptr [ebp - 0x14]
02F7:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
02FA:    D9 EE                   fldz    ; Load 0.0
02FC:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
02FE:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0300:    9E                      sahf
0301:    75 1C                   jne      0x31f    ; Conditional jump
0303:    DD 45 C4                fld      qword ptr [ebp - 0x3c]
0306:    DC 4D DC                fmul     qword ptr [ebp - 0x24]
0309:    DD 9D 44 FF FF FF       fstp     qword ptr [ebp - 0xbc]
030F:    DD 45 C4                fld      qword ptr [ebp - 0x3c]
0312:    D9 E0                   fchs    ; -ST(0)
0314:    DC 4D DC                fmul     qword ptr [ebp - 0x24]
0317:    DD 9D 4C FF FF FF       fstp     qword ptr [ebp - 0xb4]
031D:    EB 5C                   jmp      0x37b
031F:    DD 85 3C FF FF FF       fld      qword ptr [ebp - 0xc4]
0325:    DC 4D CC                fmul     qword ptr [ebp - 0x34]
0328:    DD 45 BC                fld      qword ptr [ebp - 0x44]
032B:    DC 4D D4                fmul     qword ptr [ebp - 0x2c]
032E:    DE C1                   faddp    st(1)
0330:    DC 4D E4                fmul     qword ptr [ebp - 0x1c]
0333:    DD 45 C4                fld      qword ptr [ebp - 0x3c]
0336:    DC 4D DC                fmul     qword ptr [ebp - 0x24]
0339:    DD 45 EC                fld      qword ptr [ebp - 0x14]
033C:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
033F:    DE F9                   fdivp    st(1)
0341:    D9 E8                   fld1    ; Load 1.0
0343:    DE C1                   faddp    st(1)
0345:    DE C9                   fmulp    st(1)
0347:    DD 9D 44 FF FF FF       fstp     qword ptr [ebp - 0xbc]
034D:    DD 45 CC                fld      qword ptr [ebp - 0x34]
0350:    DC 4D BC                fmul     qword ptr [ebp - 0x44]
0353:    DD 85 3C FF FF FF       fld      qword ptr [ebp - 0xc4]
0359:    DC 4D D4                fmul     qword ptr [ebp - 0x2c]
035C:    DE E9                   fsubp    st(1)
035E:    DC 4D E4                fmul     qword ptr [ebp - 0x1c]
0361:    DD 45 C4                fld      qword ptr [ebp - 0x3c]
0364:    DC 4D DC                fmul     qword ptr [ebp - 0x24]
0367:    DD 45 EC                fld      qword ptr [ebp - 0x14]
036A:    DC 4D F4                fmul     qword ptr [ebp - 0xc]
036D:    DE F9                   fdivp    st(1)
036F:    D9 E8                   fld1    ; Load 1.0
0371:    DE C1                   faddp    st(1)
0373:    DE C9                   fmulp    st(1)
0375:    DD 9D 4C FF FF FF       fstp     qword ptr [ebp - 0xb4]
037B:    DD 45 F4                fld      qword ptr [ebp - 0xc]
037E:    DC 4D C4                fmul     qword ptr [ebp - 0x3c]
0381:    DD 45 EC                fld      qword ptr [ebp - 0x14]
0384:    DC 4D DC                fmul     qword ptr [ebp - 0x24]
0387:    DE E9                   fsubp    st(1)
0389:    DC 4D E4                fmul     qword ptr [ebp - 0x1c]
038C:    DD 9D 54 FF FF FF       fstp     qword ptr [ebp - 0xac]
0392:    D9 D0                   fnop
0394:    DD 85 44 FF FF FF       fld      qword ptr [ebp - 0xbc]
039A:    DD 1B                   fstp     qword ptr [ebx]
039C:    DD 85 4C FF FF FF       fld      qword ptr [ebp - 0xb4]
03A2:    DD 1A                   fstp     qword ptr [edx]
03A4:    DD 85 54 FF FF FF       fld      qword ptr [ebp - 0xac]
03AA:    DD 19                   fstp     qword ptr [ecx]
03AC:    D9 D0                   fnop
03AE:    8B C3                   mov      eax, ebx
03B0:    5F                      pop      edi
03B1:    5E                      pop      esi
03B2:    5B                      pop      ebx
03B3:    8B E5                   mov      esp, ebp
03B5:    5D                      pop      ebp
03B6:    C2 08 00                ret      8
