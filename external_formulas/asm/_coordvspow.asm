; Disassembly: _coordvspow.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 790 bytes, decoded: 790 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 48 00 00 00       sub      esp, 0x48
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    90                      nop
000D:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0010:    8B D8                   mov      ebx, eax
0012:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0015:    90                      nop
0016:    90                      nop
0017:    83 7F F4 00             cmp      dword ptr [edi - 0xc], 0
001B:    75 13                   jne      0x30    ; Conditional jump
001D:    D9 EE                   fldz    ; Load 0.0
001F:    DD 55 F8                fst      qword ptr [ebp - 8]
0022:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0025:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0028:    E9 F7 00 00 00          jmp      0x124
002D:    90                      nop
002E:    90                      nop
002F:    90                      nop
0030:    DD 47 EC                fld      qword ptr [edi - 0x14]
0033:    D9 E8                   fld1    ; Load 1.0
0035:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0037:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0039:    80 E4 41                and      ah, 0x41
003C:    80 F4 40                xor      ah, 0x40
003F:    0F 84 BE 00 00 00       je       0x103    ; Conditional jump
0045:    DD 47 EC                fld      qword ptr [edi - 0x14]
0048:    D9 EE                   fldz    ; Load 0.0
004A:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
004C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004E:    80 E4 41                and      ah, 0x41
0051:    80 F4 40                xor      ah, 0x40
0054:    0F 84 A9 00 00 00       je       0x103    ; Conditional jump
005A:    D9 D0                   fnop
005C:    DD 03                   fld      qword ptr [ebx]
005E:    D8 C8                   fmul     st(0)
0060:    DD 02                   fld      qword ptr [edx]
0062:    D8 C8                   fmul     st(0)
0064:    DD 01                   fld      qword ptr [ecx]
0066:    D8 C8                   fmul     st(0)
0068:    DE C1                   faddp    st(1)
006A:    DE C1                   faddp    st(1)
006C:    D9 FA                   fsqrt    ; sqrt(ST(0))
006E:    DD 55 E0                fst      qword ptr [ebp - 0x20]
0071:    DD 47 EC                fld      qword ptr [edi - 0x14]
0074:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0076:    D9 E1                   fabs    ; |ST(0)|
0078:    D9 F1                   fyl2x
007A:    D9 E8                   fld1    ; Load 1.0
007C:    D9 C1                   fld      st(1)
007E:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
0080:    D9 F0                   f2xm1
0082:    DE C1                   faddp    st(1)
0084:    D9 FD                   fscale
0086:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0088:    DD D8                   fstp     st(0)
008A:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
008D:    DD 02                   fld      qword ptr [edx]
008F:    DD 03                   fld      qword ptr [ebx]
0091:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0093:    DC 4F EC                fmul     qword ptr [edi - 0x14]
0096:    90                      nop
0097:    90                      nop
0098:    90                      nop
0099:    DD 01                   fld      qword ptr [ecx]
009B:    DC 75 E0                fdiv     qword ptr [ebp - 0x20]
009E:    D9 C0                   fld      st(0)
00A0:    D8 C8                   fmul     st(0)
00A2:    D9 E0                   fchs    ; -ST(0)
00A4:    D9 E8                   fld1    ; Load 1.0
00A6:    DE C1                   faddp    st(1)
00A8:    D9 FA                   fsqrt    ; sqrt(ST(0))
00AA:    D9 E8                   fld1    ; Load 1.0
00AC:    DE C1                   faddp    st(1)
00AE:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00B0:    90                      nop
00B1:    90                      nop
00B2:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00B4:    D8 C0                   fadd     st(0)
00B6:    DC 4F EC                fmul     qword ptr [edi - 0x14]
00B9:    90                      nop
00BA:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00BC:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
00BF:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
00C2:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00C4:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
00C7:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00CA:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00CD:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
00D0:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
00D3:    DA 4F F4                fimul    dword ptr [edi - 0xc]
00D6:    DC 0F                   fmul     qword ptr [edi]
00D8:    DD 5D F8                fstp     qword ptr [ebp - 8]
00DB:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00DE:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00E1:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
00E4:    DA 4F F4                fimul    dword ptr [edi - 0xc]
00E7:    DC 0F                   fmul     qword ptr [edi]
00E9:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00EC:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00EF:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
00F2:    90                      nop
00F3:    90                      nop
00F4:    90                      nop
00F5:    DA 4F F4                fimul    dword ptr [edi - 0xc]
00F8:    DC 0F                   fmul     qword ptr [edi]
00FA:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00FD:    EB 22                   jmp      0x121
00FF:    90                      nop
0100:    90                      nop
0101:    90                      nop
0102:    90                      nop
0103:    DD 03                   fld      qword ptr [ebx]
0105:    DA 4F F4                fimul    dword ptr [edi - 0xc]
0108:    DC 0F                   fmul     qword ptr [edi]
010A:    DD 5D F8                fstp     qword ptr [ebp - 8]
010D:    DD 02                   fld      qword ptr [edx]
010F:    DA 4F F4                fimul    dword ptr [edi - 0xc]
0112:    DC 0F                   fmul     qword ptr [edi]
0114:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0117:    DD 01                   fld      qword ptr [ecx]
0119:    DA 4F F4                fimul    dword ptr [edi - 0xc]
011C:    DC 0F                   fmul     qword ptr [edi]
011E:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0121:    90                      nop
0122:    90                      nop
0123:    90                      nop
0124:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
0128:    74 2C                   je       0x156    ; Conditional jump
012A:    90                      nop
012B:    90                      nop
012C:    90                      nop
012D:    90                      nop
012E:    DD 03                   fld      qword ptr [ebx]
0130:    D8 C8                   fmul     st(0)
0132:    DD 02                   fld      qword ptr [edx]
0134:    D8 C8                   fmul     st(0)
0136:    DE C1                   faddp    st(1)
0138:    D9 FA                   fsqrt    ; sqrt(ST(0))
013A:    DD 02                   fld      qword ptr [edx]
013C:    DD 03                   fld      qword ptr [ebx]
013E:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0140:    DA 4F E8                fimul    dword ptr [edi - 0x18]
0143:    DC 0F                   fmul     qword ptr [edi]
0145:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0148:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
014B:    DA 4F E8                fimul    dword ptr [edi - 0x18]
014E:    DC 0F                   fmul     qword ptr [edi]
0150:    DC 45 F8                fadd     qword ptr [ebp - 8]
0153:    DD 5D F8                fstp     qword ptr [ebp - 8]
0156:    83 7F E4 00             cmp      dword ptr [edi - 0x1c], 0
015A:    74 45                   je       0x1a1    ; Conditional jump
015C:    90                      nop
015D:    90                      nop
015E:    90                      nop
015F:    90                      nop
0160:    DD 03                   fld      qword ptr [ebx]
0162:    D8 C8                   fmul     st(0)
0164:    DD 02                   fld      qword ptr [edx]
0166:    D8 C8                   fmul     st(0)
0168:    DE C1                   faddp    st(1)
016A:    DD 01                   fld      qword ptr [ecx]
016C:    D8 C8                   fmul     st(0)
016E:    D8 C1                   fadd     st(1)
0170:    D9 FA                   fsqrt    ; sqrt(ST(0))
0172:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0174:    D9 FA                   fsqrt    ; sqrt(ST(0))
0176:    DD 01                   fld      qword ptr [ecx]
0178:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
017A:    DD 02                   fld      qword ptr [edx]
017C:    DD 03                   fld      qword ptr [ebx]
017E:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0180:    DA 4F E4                fimul    dword ptr [edi - 0x1c]
0183:    DC 0F                   fmul     qword ptr [edi]
0185:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0188:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
018B:    DA 4F E4                fimul    dword ptr [edi - 0x1c]
018E:    DC 0F                   fmul     qword ptr [edi]
0190:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0193:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0196:    DA 4F E4                fimul    dword ptr [edi - 0x1c]
0199:    DC 0F                   fmul     qword ptr [edi]
019B:    DC 45 F8                fadd     qword ptr [ebp - 8]
019E:    DD 5D F8                fstp     qword ptr [ebp - 8]
01A1:    90                      nop
01A2:    83 7F E0 00             cmp      dword ptr [edi - 0x20], 0
01A6:    74 2A                   je       0x1d2    ; Conditional jump
01A8:    90                      nop
01A9:    90                      nop
01AA:    90                      nop
01AB:    90                      nop
01AC:    DD 03                   fld      qword ptr [ebx]
01AE:    DD 02                   fld      qword ptr [edx]
01B0:    D9 FF                   fcos    ; cos(ST(0))
01B2:    DE C9                   fmulp    st(1)
01B4:    DD 03                   fld      qword ptr [ebx]
01B6:    DD 02                   fld      qword ptr [edx]
01B8:    D9 FE                   fsin    ; sin(ST(0))
01BA:    DE C9                   fmulp    st(1)
01BC:    DA 4F E0                fimul    dword ptr [edi - 0x20]
01BF:    DC 0F                   fmul     qword ptr [edi]
01C1:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
01C4:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
01C7:    DA 4F E0                fimul    dword ptr [edi - 0x20]
01CA:    DC 0F                   fmul     qword ptr [edi]
01CC:    DC 45 F8                fadd     qword ptr [ebp - 8]
01CF:    DD 5D F8                fstp     qword ptr [ebp - 8]
01D2:    90                      nop
01D3:    83 7F DC 00             cmp      dword ptr [edi - 0x24], 0
01D7:    74 41                   je       0x21a    ; Conditional jump
01D9:    90                      nop
01DA:    90                      nop
01DB:    90                      nop
01DC:    90                      nop
01DD:    DD 02                   fld      qword ptr [edx]
01DF:    D9 FB                   fsincos    ; ST(0)=cos, push sin
01E1:    DD 01                   fld      qword ptr [ecx]
01E3:    D9 FB                   fsincos    ; ST(0)=cos, push sin
01E5:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
01E7:    DC 0B                   fmul     qword ptr [ebx]
01E9:    DA 4F DC                fimul    dword ptr [edi - 0x24]
01EC:    DC 0F                   fmul     qword ptr [edi]
01EE:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
01F1:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
01F4:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
01F6:    DC C9                   fmul     st(1), st(0)
01F8:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
01FA:    DC 0B                   fmul     qword ptr [ebx]
01FC:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
01FE:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0200:    DC 0B                   fmul     qword ptr [ebx]
0202:    DE C9                   fmulp    st(1)
0204:    DA 4F DC                fimul    dword ptr [edi - 0x24]
0207:    DC 0F                   fmul     qword ptr [edi]
0209:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
020C:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
020F:    DA 4F DC                fimul    dword ptr [edi - 0x24]
0212:    DC 0F                   fmul     qword ptr [edi]
0214:    DC 45 F8                fadd     qword ptr [ebp - 8]
0217:    DD 5D F8                fstp     qword ptr [ebp - 8]
021A:    90                      nop
021B:    83 7F D8 00             cmp      dword ptr [edi - 0x28], 0
021F:    0F 84 D6 00 00 00       je       0x2fb    ; Conditional jump
0225:    DD 03                   fld      qword ptr [ebx]
0227:    D8 C8                   fmul     st(0)
0229:    DD 02                   fld      qword ptr [edx]
022B:    D8 C8                   fmul     st(0)
022D:    DE C1                   faddp    st(1)
022F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0231:    DD 01                   fld      qword ptr [ecx]
0233:    DE F1                   fdivrp   st(1)
0235:    DD 55 E0                fst      qword ptr [ebp - 0x20]
0238:    90                      nop
0239:    DC 0A                   fmul     qword ptr [edx]
023B:    D9 E0                   fchs    ; -ST(0)
023D:    DC 03                   fadd     qword ptr [ebx]
023F:    D9 EA                   fldl2e    ; Load log2(e)
0241:    DE C9                   fmulp    st(1)
0243:    D9 E8                   fld1    ; Load 1.0
0245:    D9 C1                   fld      st(1)
0247:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
0249:    D9 F0                   f2xm1
024B:    DE C1                   faddp    st(1)
024D:    D9 FD                   fscale
024F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0251:    DD D8                   fstp     st(0)
0253:    DC 4F 08                fmul     qword ptr [edi + 8]
0256:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0259:    90                      nop
025A:    DD 45 E0                fld      qword ptr [ebp - 0x20]
025D:    DC 0B                   fmul     qword ptr [ebx]
025F:    DC 02                   fadd     qword ptr [edx]
0261:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0263:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0266:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0269:    DD 45 E0                fld      qword ptr [ebp - 0x20]
026C:    DC 0B                   fmul     qword ptr [ebx]
026E:    DC 22                   fsub     qword ptr [edx]
0270:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0272:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
0275:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0278:    90                      nop
0279:    DD 45 E0                fld      qword ptr [ebp - 0x20]
027C:    DC 0A                   fmul     qword ptr [edx]
027E:    D8 C0                   fadd     st(0)
0280:    D9 EA                   fldl2e    ; Load log2(e)
0282:    DE C9                   fmulp    st(1)
0284:    D9 E8                   fld1    ; Load 1.0
0286:    D9 C1                   fld      st(1)
0288:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
028A:    D9 F0                   f2xm1
028C:    DE C1                   faddp    st(1)
028E:    D9 FD                   fscale
0290:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0292:    DD D8                   fstp     st(0)
0294:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0297:    90                      nop
0298:    DD 45 E0                fld      qword ptr [ebp - 0x20]
029B:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
029E:    DC 45 C8                fadd     qword ptr [ebp - 0x38]
02A1:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
02A4:    DA 4F D8                fimul    dword ptr [edi - 0x28]
02A7:    DC 0F                   fmul     qword ptr [edi]
02A9:    DC 45 F8                fadd     qword ptr [ebp - 8]
02AC:    DD 5D F8                fstp     qword ptr [ebp - 8]
02AF:    90                      nop
02B0:    DD 45 E0                fld      qword ptr [ebp - 0x20]
02B3:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
02B6:    D9 E0                   fchs    ; -ST(0)
02B8:    DC 45 D0                fadd     qword ptr [ebp - 0x30]
02BB:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
02BE:    DA 4F D8                fimul    dword ptr [edi - 0x28]
02C1:    DC 0F                   fmul     qword ptr [edi]
02C3:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
02C6:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
02C9:    90                      nop
02CA:    DD 03                   fld      qword ptr [ebx]
02CC:    D8 C8                   fmul     st(0)
02CE:    DD 02                   fld      qword ptr [edx]
02D0:    D8 C8                   fmul     st(0)
02D2:    DE C1                   faddp    st(1)
02D4:    D9 FA                   fsqrt    ; sqrt(ST(0))
02D6:    D9 EA                   fldl2e    ; Load log2(e)
02D8:    DE C9                   fmulp    st(1)
02DA:    D9 E8                   fld1    ; Load 1.0
02DC:    D9 C1                   fld      st(1)
02DE:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
02E0:    D9 F0                   f2xm1
02E2:    DE C1                   faddp    st(1)
02E4:    D9 FD                   fscale
02E6:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
02E8:    DD D8                   fstp     st(0)
02EA:    DD 01                   fld      qword ptr [ecx]
02EC:    D9 FE                   fsin    ; sin(ST(0))
02EE:    DE C9                   fmulp    st(1)
02F0:    DA 4F D8                fimul    dword ptr [edi - 0x28]
02F3:    DC 0F                   fmul     qword ptr [edi]
02F5:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
02F8:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
02FB:    90                      nop
02FC:    DD 45 E8                fld      qword ptr [ebp - 0x18]
02FF:    DD 19                   fstp     qword ptr [ecx]
0301:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0304:    DD 1A                   fstp     qword ptr [edx]
0306:    DD 45 F8                fld      qword ptr [ebp - 8]
0309:    DD 1B                   fstp     qword ptr [ebx]
030B:    8B C3                   mov      eax, ebx
030D:    5F                      pop      edi
030E:    5E                      pop      esi
030F:    5B                      pop      ebx
0310:    89 EC                   mov      esp, ebp
0312:    5D                      pop      ebp
0313:    C2 08 00                ret      8
