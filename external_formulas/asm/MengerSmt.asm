; Disassembly: MengerSmt.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 646 bytes, decoded: 646 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 30 00 00 00       sub      esp, 0x30
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 47 AC                fld      qword ptr [edi - 0x54]
0019:    D9 E1                   fabs    ; |ST(0)|
001B:    DD 5D F8                fstp     qword ptr [ebp - 8]
001E:    DD 47 A4                fld      qword ptr [edi - 0x5c]
0021:    D9 E1                   fabs    ; |ST(0)|
0023:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0026:    DD 47 AC                fld      qword ptr [edi - 0x54]
0029:    D9 EE                   fldz    ; Load 0.0
002B:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
002D:    9B                      wait
002E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0030:    80 E4 41                and      ah, 0x41
0033:    0F 85 45 00 00 00       jne      0x7e    ; Conditional jump
0039:    DD 03                   fld      qword ptr [ebx]
003B:    D9 E1                   fabs    ; |ST(0)|
003D:    D9 C0                   fld      st(0)
003F:    D8 C8                   fmul     st(0)
0041:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0044:    DE C9                   fmulp    st(1)
0046:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0049:    DC 45 F8                fadd     qword ptr [ebp - 8]
004C:    DE F9                   fdivp    st(1)
004E:    DD 02                   fld      qword ptr [edx]
0050:    D9 E1                   fabs    ; |ST(0)|
0052:    D9 C0                   fld      st(0)
0054:    D8 C8                   fmul     st(0)
0056:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0059:    DE C9                   fmulp    st(1)
005B:    DD 45 F0                fld      qword ptr [ebp - 0x10]
005E:    DC 45 F8                fadd     qword ptr [ebp - 8]
0061:    DE F9                   fdivp    st(1)
0063:    DD 01                   fld      qword ptr [ecx]
0065:    D9 E1                   fabs    ; |ST(0)|
0067:    D9 C0                   fld      st(0)
0069:    D8 C8                   fmul     st(0)
006B:    DD 55 F0                fst      qword ptr [ebp - 0x10]
006E:    DE C9                   fmulp    st(1)
0070:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0073:    DC 45 F8                fadd     qword ptr [ebp - 8]
0076:    DE F9                   fdivp    st(1)
0078:    E9 1C 00 00 00          jmp      0x99
007D:    90                      nop
007E:    DD 03                   fld      qword ptr [ebx]
0080:    D8 C8                   fmul     st(0)
0082:    DC 45 F8                fadd     qword ptr [ebp - 8]
0085:    D9 FA                   fsqrt    ; sqrt(ST(0))
0087:    DD 02                   fld      qword ptr [edx]
0089:    D8 C8                   fmul     st(0)
008B:    DC 45 F8                fadd     qword ptr [ebp - 8]
008E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0090:    DD 01                   fld      qword ptr [ecx]
0092:    D8 C8                   fmul     st(0)
0094:    DC 45 F8                fadd     qword ptr [ebp - 8]
0097:    D9 FA                   fsqrt    ; sqrt(ST(0))
0099:    DD 19                   fstp     qword ptr [ecx]
009B:    DD 1A                   fstp     qword ptr [edx]
009D:    DD 1B                   fstp     qword ptr [ebx]
009F:    D9 D0                   fnop
00A1:    DD 47 A4                fld      qword ptr [edi - 0x5c]
00A4:    D9 EE                   fldz    ; Load 0.0
00A6:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00A8:    9B                      wait
00A9:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AB:    80 E4 41                and      ah, 0x41
00AE:    0F 85 A2 00 00 00       jne      0x156    ; Conditional jump
00B4:    DD 03                   fld      qword ptr [ebx]
00B6:    DC 22                   fsub     qword ptr [edx]
00B8:    D9 C0                   fld      st(0)
00BA:    D9 E1                   fabs    ; |ST(0)|
00BC:    D9 C0                   fld      st(0)
00BE:    D8 C8                   fmul     st(0)
00C0:    DD 55 F0                fst      qword ptr [ebp - 0x10]
00C3:    DE C9                   fmulp    st(1)
00C5:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00C8:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
00CB:    DE F9                   fdivp    st(1)
00CD:    D9 E0                   fchs    ; -ST(0)
00CF:    DE C1                   faddp    st(1)
00D1:    DC 0F                   fmul     qword ptr [edi]
00D3:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00D6:    90                      nop
00D7:    DD 03                   fld      qword ptr [ebx]
00D9:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
00DC:    90                      nop
00DD:    DD 02                   fld      qword ptr [edx]
00DF:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
00E2:    90                      nop
00E3:    DD 1A                   fstp     qword ptr [edx]
00E5:    DD 1B                   fstp     qword ptr [ebx]
00E7:    D9 D0                   fnop
00E9:    DD 03                   fld      qword ptr [ebx]
00EB:    DC 21                   fsub     qword ptr [ecx]
00ED:    D9 C0                   fld      st(0)
00EF:    D9 E1                   fabs    ; |ST(0)|
00F1:    D9 C0                   fld      st(0)
00F3:    D8 C8                   fmul     st(0)
00F5:    DD 55 F0                fst      qword ptr [ebp - 0x10]
00F8:    DE C9                   fmulp    st(1)
00FA:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00FD:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0100:    DE F9                   fdivp    st(1)
0102:    D9 E0                   fchs    ; -ST(0)
0104:    DE C1                   faddp    st(1)
0106:    DC 0F                   fmul     qword ptr [edi]
0108:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
010B:    90                      nop
010C:    DD 03                   fld      qword ptr [ebx]
010E:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
0111:    90                      nop
0112:    DD 01                   fld      qword ptr [ecx]
0114:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0117:    90                      nop
0118:    DD 19                   fstp     qword ptr [ecx]
011A:    DD 1B                   fstp     qword ptr [ebx]
011C:    D9 D0                   fnop
011E:    DD 02                   fld      qword ptr [edx]
0120:    DC 21                   fsub     qword ptr [ecx]
0122:    D9 C0                   fld      st(0)
0124:    D9 E1                   fabs    ; |ST(0)|
0126:    D9 C0                   fld      st(0)
0128:    D8 C8                   fmul     st(0)
012A:    DD 55 F0                fst      qword ptr [ebp - 0x10]
012D:    DE C9                   fmulp    st(1)
012F:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0132:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0135:    DE F9                   fdivp    st(1)
0137:    D9 E0                   fchs    ; -ST(0)
0139:    DE C1                   faddp    st(1)
013B:    DC 0F                   fmul     qword ptr [edi]
013D:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0140:    90                      nop
0141:    DD 02                   fld      qword ptr [edx]
0143:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
0146:    90                      nop
0147:    DD 01                   fld      qword ptr [ecx]
0149:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
014C:    90                      nop
014D:    DD 19                   fstp     qword ptr [ecx]
014F:    DD 1A                   fstp     qword ptr [edx]
0151:    E9 7B 00 00 00          jmp      0x1d1
0156:    DD 03                   fld      qword ptr [ebx]
0158:    DC 22                   fsub     qword ptr [edx]
015A:    D9 C0                   fld      st(0)
015C:    D8 C8                   fmul     st(0)
015E:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0161:    D9 FA                   fsqrt    ; sqrt(ST(0))
0163:    D9 E0                   fchs    ; -ST(0)
0165:    DE C1                   faddp    st(1)
0167:    DC 0F                   fmul     qword ptr [edi]
0169:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
016C:    90                      nop
016D:    DD 03                   fld      qword ptr [ebx]
016F:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
0172:    90                      nop
0173:    DD 02                   fld      qword ptr [edx]
0175:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0178:    90                      nop
0179:    DD 1A                   fstp     qword ptr [edx]
017B:    DD 1B                   fstp     qword ptr [ebx]
017D:    D9 D0                   fnop
017F:    DD 03                   fld      qword ptr [ebx]
0181:    DC 21                   fsub     qword ptr [ecx]
0183:    D9 C0                   fld      st(0)
0185:    D8 C8                   fmul     st(0)
0187:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
018A:    D9 FA                   fsqrt    ; sqrt(ST(0))
018C:    D9 E0                   fchs    ; -ST(0)
018E:    DE C1                   faddp    st(1)
0190:    DC 0F                   fmul     qword ptr [edi]
0192:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0195:    90                      nop
0196:    DD 03                   fld      qword ptr [ebx]
0198:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
019B:    90                      nop
019C:    DD 01                   fld      qword ptr [ecx]
019E:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
01A1:    90                      nop
01A2:    DD 19                   fstp     qword ptr [ecx]
01A4:    DD 1B                   fstp     qword ptr [ebx]
01A6:    D9 D0                   fnop
01A8:    DD 02                   fld      qword ptr [edx]
01AA:    DC 21                   fsub     qword ptr [ecx]
01AC:    D9 C0                   fld      st(0)
01AE:    D8 C8                   fmul     st(0)
01B0:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
01B3:    D9 FA                   fsqrt    ; sqrt(ST(0))
01B5:    D9 E0                   fchs    ; -ST(0)
01B7:    DE C1                   faddp    st(1)
01B9:    DC 0F                   fmul     qword ptr [edi]
01BB:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
01BE:    90                      nop
01BF:    DD 02                   fld      qword ptr [edx]
01C1:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
01C4:    90                      nop
01C5:    DD 01                   fld      qword ptr [ecx]
01C7:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
01CA:    90                      nop
01CB:    DD 19                   fstp     qword ptr [ecx]
01CD:    DD 1A                   fstp     qword ptr [edx]
01CF:    D9 D0                   fnop
01D1:    DD 01                   fld      qword ptr [ecx]
01D3:    DD 03                   fld      qword ptr [ebx]
01D5:    DD 02                   fld      qword ptr [edx]
01D7:    D9 C1                   fld      st(1)
01D9:    D8 4F BC                fmul     dword ptr [edi - 0x44]
01DC:    D9 C1                   fld      st(1)
01DE:    D8 4F B8                fmul     dword ptr [edi - 0x48]
01E1:    DE C1                   faddp    st(1)
01E3:    D9 C3                   fld      st(3)
01E5:    D8 4F B4                fmul     dword ptr [edi - 0x4c]
01E8:    DE C1                   faddp    st(1)
01EA:    D9 C2                   fld      st(2)
01EC:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
01EF:    D9 C2                   fld      st(2)
01F1:    D8 4F D0                fmul     dword ptr [edi - 0x30]
01F4:    DE C1                   faddp    st(1)
01F6:    D9 C4                   fld      st(4)
01F8:    D8 4F CC                fmul     dword ptr [edi - 0x34]
01FB:    DE C1                   faddp    st(1)
01FD:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
01FF:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0202:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0204:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0207:    DE C2                   faddp    st(2)
0209:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
020B:    D8 4F C0                fmul     dword ptr [edi - 0x40]
020E:    DE C1                   faddp    st(1)
0210:    DD 47 F0                fld      qword ptr [edi - 0x10]
0213:    DD 41 08                fld      qword ptr [ecx + 8]
0216:    D8 C9                   fmul     st(1)
0218:    DD 59 08                fstp     qword ptr [ecx + 8]
021B:    DC CA                   fmul     st(2), st(0)
021D:    DC C9                   fmul     st(1), st(0)
021F:    D9 C0                   fld      st(0)
0221:    D9 E8                   fld1    ; Load 1.0
0223:    DE E9                   fsubp    st(1)
0225:    D9 C0                   fld      st(0)
0227:    D9 C0                   fld      st(0)
0229:    DC 4F E8                fmul     qword ptr [edi - 0x18]
022C:    DE ED                   fsubp    st(5)
022E:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0231:    DE EB                   fsubp    st(3)
0233:    D8 F1                   fdiv     st(1)
0235:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0238:    DC EC                   fsub     st(4), st(0)
023A:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
023C:    DD 47 AC                fld      qword ptr [edi - 0x54]
023F:    D9 EE                   fldz    ; Load 0.0
0241:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0243:    9B                      wait
0244:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0246:    80 E4 41                and      ah, 0x41
0249:    0F 85 19 00 00 00       jne      0x268    ; Conditional jump
024F:    D9 E1                   fabs    ; |ST(0)|
0251:    D9 C0                   fld      st(0)
0253:    D8 C8                   fmul     st(0)
0255:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0258:    DE C9                   fmulp    st(1)
025A:    DD 45 F0                fld      qword ptr [ebp - 0x10]
025D:    DC 45 F8                fadd     qword ptr [ebp - 8]
0260:    DE F9                   fdivp    st(1)
0262:    E9 08 00 00 00          jmp      0x26f
0267:    90                      nop
0268:    D8 C8                   fmul     st(0)
026A:    DC 45 F8                fadd     qword ptr [ebp - 8]
026D:    D9 FA                   fsqrt    ; sqrt(ST(0))
026F:    DE EC                   fsubp    st(4)
0271:    DE CB                   fmulp    st(3)
0273:    DD 1A                   fstp     qword ptr [edx]
0275:    DD 1B                   fstp     qword ptr [ebx]
0277:    DD 19                   fstp     qword ptr [ecx]
0279:    D9 D0                   fnop
027B:    8B C3                   mov      eax, ebx
027D:    5F                      pop      edi
027E:    5E                      pop      esi
027F:    5B                      pop      ebx
0280:    89 EC                   mov      esp, ebp
0282:    5D                      pop      ebp
0283:    C2 08 00                ret      8
