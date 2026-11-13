; Disassembly: _PartlyJuliaR.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 734 bytes, decoded: 734 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    83 7F A8 01             cmp      dword ptr [edi - 0x58], 1
0012:    75 38                   jne      0x4c    ; Conditional jump
0014:    DD 03                   fld      qword ptr [ebx]
0016:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0019:    D9 E1                   fabs    ; |ST(0)|
001B:    DD 02                   fld      qword ptr [edx]
001D:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0020:    D9 E1                   fabs    ; |ST(0)|
0022:    DD 01                   fld      qword ptr [ecx]
0024:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0027:    D9 E1                   fabs    ; |ST(0)|
0029:    D8 D1                   fcom     st(1)
002B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
002D:    80 E4 41                and      ah, 0x41
0030:    77 04                   ja       0x36    ; Conditional jump
0032:    DD D9                   fstp     st(1)
0034:    EB 02                   jmp      0x38
0036:    DD D8                   fstp     st(0)
0038:    D8 D1                   fcom     st(1)
003A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
003C:    80 E4 41                and      ah, 0x41
003F:    77 04                   ja       0x45    ; Conditional jump
0041:    DD D9                   fstp     st(1)
0043:    EB 02                   jmp      0x47
0045:    DD D8                   fstp     st(0)
0047:    E9 18 01 00 00          jmp      0x164
004C:    90                      nop
004D:    83 7F A8 02             cmp      dword ptr [edi - 0x58], 2
0051:    75 20                   jne      0x73    ; Conditional jump
0053:    DD 03                   fld      qword ptr [ebx]
0055:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0058:    D8 C8                   fmul     st(0)
005A:    DD 02                   fld      qword ptr [edx]
005C:    DC 67 E8                fsub     qword ptr [edi - 0x18]
005F:    D8 C8                   fmul     st(0)
0061:    DE C1                   faddp    st(1)
0063:    DD 01                   fld      qword ptr [ecx]
0065:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0068:    D8 C8                   fmul     st(0)
006A:    DE F9                   fdivp    st(1)
006C:    D9 FA                   fsqrt    ; sqrt(ST(0))
006E:    E9 F1 00 00 00          jmp      0x164
0073:    90                      nop
0074:    83 7F A8 03             cmp      dword ptr [edi - 0x58], 3
0078:    75 17                   jne      0x91    ; Conditional jump
007A:    DD 03                   fld      qword ptr [ebx]
007C:    DC 67 F0                fsub     qword ptr [edi - 0x10]
007F:    D8 C8                   fmul     st(0)
0081:    DD 02                   fld      qword ptr [edx]
0083:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0086:    D8 C8                   fmul     st(0)
0088:    DE C1                   faddp    st(1)
008A:    D9 FA                   fsqrt    ; sqrt(ST(0))
008C:    E9 D3 00 00 00          jmp      0x164
0091:    90                      nop
0092:    83 7F A8 FE             cmp      dword ptr [edi - 0x58], -2
0096:    75 35                   jne      0xcd    ; Conditional jump
0098:    DD 01                   fld      qword ptr [ecx]
009A:    DC 67 E0                fsub     qword ptr [edi - 0x20]
009D:    D9 EE                   fldz    ; Load 0.0
009F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00A1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A3:    80 E4 41                and      ah, 0x41
00A6:    76 1E                   jbe      0xc6    ; Conditional jump
00A8:    DD 03                   fld      qword ptr [ebx]
00AA:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00AD:    D8 C8                   fmul     st(0)
00AF:    DD 02                   fld      qword ptr [edx]
00B1:    DC 67 E8                fsub     qword ptr [edi - 0x18]
00B4:    D8 C8                   fmul     st(0)
00B6:    DE C1                   faddp    st(1)
00B8:    D9 FA                   fsqrt    ; sqrt(ST(0))
00BA:    DD 01                   fld      qword ptr [ecx]
00BC:    DC 67 E0                fsub     qword ptr [edi - 0x20]
00BF:    DE F9                   fdivp    st(1)
00C1:    E9 9E 00 00 00          jmp      0x164
00C6:    D9 E8                   fld1    ; Load 1.0
00C8:    E9 9A 00 00 00          jmp      0x167
00CD:    90                      nop
00CE:    83 7F A8 04             cmp      dword ptr [edi - 0x58], 4
00D2:    75 2B                   jne      0xff    ; Conditional jump
00D4:    DD 03                   fld      qword ptr [ebx]
00D6:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00D9:    D9 E1                   fabs    ; |ST(0)|
00DB:    DD 02                   fld      qword ptr [edx]
00DD:    DC 67 E8                fsub     qword ptr [edi - 0x18]
00E0:    D9 E1                   fabs    ; |ST(0)|
00E2:    D8 D1                   fcom     st(1)
00E4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E6:    80 E4 41                and      ah, 0x41
00E9:    77 04                   ja       0xef    ; Conditional jump
00EB:    DD D9                   fstp     st(1)
00ED:    EB 02                   jmp      0xf1
00EF:    DD D8                   fstp     st(0)
00F1:    DD 01                   fld      qword ptr [ecx]
00F3:    DC 67 E0                fsub     qword ptr [edi - 0x20]
00F6:    D9 E1                   fabs    ; |ST(0)|
00F8:    DE F9                   fdivp    st(1)
00FA:    E9 65 00 00 00          jmp      0x164
00FF:    90                      nop
0100:    83 7F A8 FC             cmp      dword ptr [edi - 0x58], -4
0104:    75 43                   jne      0x149    ; Conditional jump
0106:    DD 01                   fld      qword ptr [ecx]
0108:    DC 67 E0                fsub     qword ptr [edi - 0x20]
010B:    D9 EE                   fldz    ; Load 0.0
010D:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
010F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0111:    80 E4 41                and      ah, 0x41
0114:    76 2B                   jbe      0x141    ; Conditional jump
0116:    DD 03                   fld      qword ptr [ebx]
0118:    DC 67 F0                fsub     qword ptr [edi - 0x10]
011B:    D9 E1                   fabs    ; |ST(0)|
011D:    DD 02                   fld      qword ptr [edx]
011F:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0122:    D9 E1                   fabs    ; |ST(0)|
0124:    D8 D1                   fcom     st(1)
0126:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0128:    80 E4 41                and      ah, 0x41
012B:    77 04                   ja       0x131    ; Conditional jump
012D:    DD D9                   fstp     st(1)
012F:    EB 02                   jmp      0x133
0131:    DD D8                   fstp     st(0)
0133:    DD 01                   fld      qword ptr [ecx]
0135:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0138:    D9 E1                   fabs    ; |ST(0)|
013A:    DE F9                   fdivp    st(1)
013C:    E9 23 00 00 00          jmp      0x164
0141:    D9 E8                   fld1    ; Load 1.0
0143:    E9 1F 00 00 00          jmp      0x167
0148:    90                      nop
0149:    DD 03                   fld      qword ptr [ebx]
014B:    DC 67 F0                fsub     qword ptr [edi - 0x10]
014E:    D8 C8                   fmul     st(0)
0150:    DD 02                   fld      qword ptr [edx]
0152:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0155:    D8 C8                   fmul     st(0)
0157:    DD 01                   fld      qword ptr [ecx]
0159:    DC 67 E0                fsub     qword ptr [edi - 0x20]
015C:    D8 C8                   fmul     st(0)
015E:    DE C1                   faddp    st(1)
0160:    DE C1                   faddp    st(1)
0162:    D9 FA                   fsqrt    ; sqrt(ST(0))
0164:    DC 77 D8                fdiv     qword ptr [edi - 0x28]
0167:    83 7F B4 01             cmp      dword ptr [edi - 0x4c], 1
016B:    74 05                   je       0x172    ; Conditional jump
016D:    E9 A4 00 00 00          jmp      0x216
0172:    D9 E8                   fld1    ; Load 1.0
0174:    D8 D9                   fcomp    st(1)
0176:    90                      nop
0177:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0179:    80 E4 41                and      ah, 0x41
017C:    77 6D                   ja       0x1eb    ; Conditional jump
017E:    DC 67 D0                fsub     qword ptr [edi - 0x30]
0181:    D9 EE                   fldz    ; Load 0.0
0183:    D8 D9                   fcomp    st(1)
0185:    90                      nop
0186:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0188:    80 E4 41                and      ah, 0x41
018B:    76 6B                   jbe      0x1f8    ; Conditional jump
018D:    D9 E8                   fld1    ; Load 1.0
018F:    DC 67 D0                fsub     qword ptr [edi - 0x30]
0192:    DE F9                   fdivp    st(1)
0194:    D9 C0                   fld      st(0)
0196:    DC 4E 18                fmul     qword ptr [esi + 0x18]
0199:    D9 E8                   fld1    ; Load 1.0
019B:    D8 E2                   fsub     st(2)
019D:    DC 4F C8                fmul     qword ptr [edi - 0x38]
01A0:    DE C1                   faddp    st(1)
01A2:    DD 5E 18                fstp     qword ptr [esi + 0x18]
01A5:    D9 C0                   fld      st(0)
01A7:    DC 4E 20                fmul     qword ptr [esi + 0x20]
01AA:    D9 E8                   fld1    ; Load 1.0
01AC:    D8 E2                   fsub     st(2)
01AE:    DC 4F C0                fmul     qword ptr [edi - 0x40]
01B1:    DE C1                   faddp    st(1)
01B3:    DD 5E 20                fstp     qword ptr [esi + 0x20]
01B6:    D9 C0                   fld      st(0)
01B8:    DC 4E 28                fmul     qword ptr [esi + 0x28]
01BB:    D9 E8                   fld1    ; Load 1.0
01BD:    D8 E2                   fsub     st(2)
01BF:    DC 4F B8                fmul     qword ptr [edi - 0x48]
01C2:    DE C1                   faddp    st(1)
01C4:    DD 5E 28                fstp     qword ptr [esi + 0x28]
01C7:    D9 E8                   fld1    ; Load 1.0
01C9:    DE E1                   fsubrp   st(1)
01CB:    DC 4F AC                fmul     qword ptr [edi - 0x54]
01CE:    90                      nop
01CF:    DD 47 F0                fld      qword ptr [edi - 0x10]
01D2:    D8 C9                   fmul     st(1)
01D4:    DC 2B                   fsubr    qword ptr [ebx]
01D6:    DD 1B                   fstp     qword ptr [ebx]
01D8:    DD 47 E8                fld      qword ptr [edi - 0x18]
01DB:    D8 C9                   fmul     st(1)
01DD:    DC 2A                   fsubr    qword ptr [edx]
01DF:    DD 1A                   fstp     qword ptr [edx]
01E1:    DD 47 E0                fld      qword ptr [edi - 0x20]
01E4:    D8 C9                   fmul     st(1)
01E6:    DC 29                   fsubr    qword ptr [ecx]
01E8:    DD 19                   fstp     qword ptr [ecx]
01EA:    90                      nop
01EB:    DD D8                   fstp     st(0)
01ED:    89 D8                   mov      eax, ebx
01EF:    5B                      pop      ebx
01F0:    5F                      pop      edi
01F1:    5E                      pop      esi
01F2:    89 EC                   mov      esp, ebp
01F4:    5D                      pop      ebp
01F5:    C2 08 00                ret      8
01F8:    DD 47 C8                fld      qword ptr [edi - 0x38]
01FB:    DD 5E 18                fstp     qword ptr [esi + 0x18]
01FE:    DD 47 C0                fld      qword ptr [edi - 0x40]
0201:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0204:    DD 47 B8                fld      qword ptr [edi - 0x48]
0207:    DD 5E 28                fstp     qword ptr [esi + 0x28]
020A:    DD D8                   fstp     st(0)
020C:    DD 47 AC                fld      qword ptr [edi - 0x54]
020F:    EB BE                   jmp      0x1cf
0211:    D9 D0                   fnop
0213:    D9 D0                   fnop
0215:    90                      nop
0216:    D9 E8                   fld1    ; Load 1.0
0218:    D8 D9                   fcomp    st(1)
021A:    90                      nop
021B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
021D:    80 E4 41                and      ah, 0x41
0220:    76 14                   jbe      0x236    ; Conditional jump
0222:    DD 47 C8                fld      qword ptr [edi - 0x38]
0225:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0228:    DD 47 C0                fld      qword ptr [edi - 0x40]
022B:    DD 5E 20                fstp     qword ptr [esi + 0x20]
022E:    DD 47 B8                fld      qword ptr [edi - 0x48]
0231:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0234:    EB B5                   jmp      0x1eb
0236:    DC 67 D0                fsub     qword ptr [edi - 0x30]
0239:    D9 EE                   fldz    ; Load 0.0
023B:    D8 D9                   fcomp    st(1)
023D:    90                      nop
023E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0240:    80 E4 41                and      ah, 0x41
0243:    76 63                   jbe      0x2a8    ; Conditional jump
0245:    D9 E8                   fld1    ; Load 1.0
0247:    DC 67 D0                fsub     qword ptr [edi - 0x30]
024A:    DE F9                   fdivp    st(1)
024C:    D9 C0                   fld      st(0)
024E:    DC 4F C8                fmul     qword ptr [edi - 0x38]
0251:    D9 E8                   fld1    ; Load 1.0
0253:    D8 E2                   fsub     st(2)
0255:    DD 46 18                fld      qword ptr [esi + 0x18]
0258:    DD 47 AC                fld      qword ptr [edi - 0x54]
025B:    DC 4F F0                fmul     qword ptr [edi - 0x10]
025E:    D9 E0                   fchs    ; -ST(0)
0260:    DE C1                   faddp    st(1)
0262:    DE C9                   fmulp    st(1)
0264:    DE C1                   faddp    st(1)
0266:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0269:    D9 C0                   fld      st(0)
026B:    DC 4F C0                fmul     qword ptr [edi - 0x40]
026E:    D9 E8                   fld1    ; Load 1.0
0270:    D8 E2                   fsub     st(2)
0272:    DD 46 20                fld      qword ptr [esi + 0x20]
0275:    DD 47 AC                fld      qword ptr [edi - 0x54]
0278:    DC 4F E8                fmul     qword ptr [edi - 0x18]
027B:    D9 E0                   fchs    ; -ST(0)
027D:    DE C1                   faddp    st(1)
027F:    DE C9                   fmulp    st(1)
0281:    DE C1                   faddp    st(1)
0283:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0286:    D9 C0                   fld      st(0)
0288:    DC 4F B8                fmul     qword ptr [edi - 0x48]
028B:    D9 E8                   fld1    ; Load 1.0
028D:    D8 E2                   fsub     st(2)
028F:    DD 46 28                fld      qword ptr [esi + 0x28]
0292:    DD 47 AC                fld      qword ptr [edi - 0x54]
0295:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0298:    D9 E0                   fchs    ; -ST(0)
029A:    DE C1                   faddp    st(1)
029C:    DE C9                   fmulp    st(1)
029E:    DE C1                   faddp    st(1)
02A0:    DD 5E 28                fstp     qword ptr [esi + 0x28]
02A3:    E9 1F FF FF FF          jmp      0x1c7
02A8:    90                      nop
02A9:    DD 46 18                fld      qword ptr [esi + 0x18]
02AC:    DD 47 AC                fld      qword ptr [edi - 0x54]
02AF:    DC 4F F0                fmul     qword ptr [edi - 0x10]
02B2:    D9 E0                   fchs    ; -ST(0)
02B4:    DE C1                   faddp    st(1)
02B6:    DD 5E 18                fstp     qword ptr [esi + 0x18]
02B9:    DD 46 20                fld      qword ptr [esi + 0x20]
02BC:    DD 47 AC                fld      qword ptr [edi - 0x54]
02BF:    DC 4F E8                fmul     qword ptr [edi - 0x18]
02C2:    D9 E0                   fchs    ; -ST(0)
02C4:    DE C1                   faddp    st(1)
02C6:    DD 5E 20                fstp     qword ptr [esi + 0x20]
02C9:    DD 46 28                fld      qword ptr [esi + 0x28]
02CC:    DD 47 AC                fld      qword ptr [edi - 0x54]
02CF:    DC 4F E0                fmul     qword ptr [edi - 0x20]
02D2:    D9 E0                   fchs    ; -ST(0)
02D4:    DE C1                   faddp    st(1)
02D6:    DD 5E 28                fstp     qword ptr [esi + 0x28]
02D9:    E9 2C FF FF FF          jmp      0x20a
