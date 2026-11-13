; Disassembly: ABoxPlatinumB.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 574 bytes, decoded: 574 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    89 C3                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    83 C4 D0                add      esp, -0x30
0010:    DD 46 D8                fld      qword ptr [esi - 0x28]
0013:    DD 03                   fld      qword ptr [ebx]
0015:    DC 8E 74 FF FF FF       fmul     qword ptr [esi - 0x8c]
001B:    DD 01                   fld      qword ptr [ecx]
001D:    DC 8E 7C FF FF FF       fmul     qword ptr [esi - 0x84]
0023:    DE E9                   fsubp    st(1)
0025:    DD 1C 24                fstp     qword ptr [esp]
0028:    DD 03                   fld      qword ptr [ebx]
002A:    DC 8E 7C FF FF FF       fmul     qword ptr [esi - 0x84]
0030:    DD 01                   fld      qword ptr [ecx]
0032:    DC 8E 74 FF FF FF       fmul     qword ptr [esi - 0x8c]
0038:    DE C1                   faddp    st(1)
003A:    DD 5C 24 08             fstp     qword ptr [esp + 8]
003E:    90                      nop
003F:    90                      nop
0040:    DD 04 24                fld      qword ptr [esp]
0043:    DC 4E 84                fmul     qword ptr [esi - 0x7c]
0046:    DD 02                   fld      qword ptr [edx]
0048:    DC 4E 8C                fmul     qword ptr [esi - 0x74]
004B:    DE E9                   fsubp    st(1)
004D:    90                      nop
004E:    DD 04 24                fld      qword ptr [esp]
0051:    DC 4E 8C                fmul     qword ptr [esi - 0x74]
0054:    DD 02                   fld      qword ptr [edx]
0056:    DC 4E 84                fmul     qword ptr [esi - 0x7c]
0059:    DE C1                   faddp    st(1)
005B:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
005F:    90                      nop
0060:    D9 C0                   fld      st(0)
0062:    D8 E2                   fsub     st(2)
0064:    D9 E1                   fabs    ; |ST(0)|
0066:    D8 C1                   fadd     st(1)
0068:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006A:    D8 C2                   fadd     st(2)
006C:    D9 E1                   fabs    ; |ST(0)|
006E:    DE E1                   fsubrp   st(1)
0070:    DC 4E 84                fmul     qword ptr [esi - 0x7c]
0073:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0077:    DC 4E 8C                fmul     qword ptr [esi - 0x74]
007A:    DE E9                   fsubp    st(1)
007C:    90                      nop
007D:    DC 8E 74 FF FF FF       fmul     qword ptr [esi - 0x8c]
0083:    DD 44 24 08             fld      qword ptr [esp + 8]
0087:    DC 8E 7C FF FF FF       fmul     qword ptr [esi - 0x84]
008D:    DE E9                   fsubp    st(1)
008F:    90                      nop
0090:    DD 02                   fld      qword ptr [edx]
0092:    DC 8E 54 FF FF FF       fmul     qword ptr [esi - 0xac]
0098:    DD 01                   fld      qword ptr [ecx]
009A:    DC 8E 5C FF FF FF       fmul     qword ptr [esi - 0xa4]
00A0:    DE E9                   fsubp    st(1)
00A2:    DD 1C 24                fstp     qword ptr [esp]
00A5:    DD 02                   fld      qword ptr [edx]
00A7:    DC 8E 5C FF FF FF       fmul     qword ptr [esi - 0xa4]
00AD:    DD 01                   fld      qword ptr [ecx]
00AF:    DC 8E 54 FF FF FF       fmul     qword ptr [esi - 0xac]
00B5:    DE C1                   faddp    st(1)
00B7:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00BB:    90                      nop
00BC:    90                      nop
00BD:    DD 04 24                fld      qword ptr [esp]
00C0:    DC 8E 64 FF FF FF       fmul     qword ptr [esi - 0x9c]
00C6:    DD 03                   fld      qword ptr [ebx]
00C8:    DC 8E 6C FF FF FF       fmul     qword ptr [esi - 0x94]
00CE:    DE E9                   fsubp    st(1)
00D0:    90                      nop
00D1:    DD 04 24                fld      qword ptr [esp]
00D4:    DC 8E 6C FF FF FF       fmul     qword ptr [esi - 0x94]
00DA:    DD 03                   fld      qword ptr [ebx]
00DC:    DC 8E 64 FF FF FF       fmul     qword ptr [esi - 0x9c]
00E2:    DE C1                   faddp    st(1)
00E4:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00E8:    D9 C0                   fld      st(0)
00EA:    D8 E3                   fsub     st(3)
00EC:    D9 E1                   fabs    ; |ST(0)|
00EE:    D8 C1                   fadd     st(1)
00F0:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00F2:    D8 C3                   fadd     st(3)
00F4:    D9 E1                   fabs    ; |ST(0)|
00F6:    DE E1                   fsubrp   st(1)
00F8:    DC 8E 64 FF FF FF       fmul     qword ptr [esi - 0x9c]
00FE:    90                      nop
00FF:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0103:    DC 8E 6C FF FF FF       fmul     qword ptr [esi - 0x94]
0109:    DE E9                   fsubp    st(1)
010B:    90                      nop
010C:    DC 8E 54 FF FF FF       fmul     qword ptr [esi - 0xac]
0112:    DD 44 24 08             fld      qword ptr [esp + 8]
0116:    DC 8E 5C FF FF FF       fmul     qword ptr [esi - 0xa4]
011C:    DE E9                   fsubp    st(1)
011E:    DD 01                   fld      qword ptr [ecx]
0120:    D9 C0                   fld      st(0)
0122:    D8 E4                   fsub     st(4)
0124:    D9 E1                   fabs    ; |ST(0)|
0126:    D8 C1                   fadd     st(1)
0128:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
012A:    D8 C4                   fadd     st(4)
012C:    D9 E1                   fabs    ; |ST(0)|
012E:    DE E1                   fsubrp   st(1)
0130:    90                      nop
0131:    8B 86 50 FF FF FF       mov      eax, dword ptr [esi - 0xb0]
0137:    83 E0 04                and      eax, 4
013A:    74 02                   je       0x13e    ; Conditional jump
013C:    D9 E1                   fabs    ; |ST(0)|
013E:    8B 86 50 FF FF FF       mov      eax, dword ptr [esi - 0xb0]
0144:    83 E0 02                and      eax, 2
0147:    74 06                   je       0x14f    ; Conditional jump
0149:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
014B:    D9 E1                   fabs    ; |ST(0)|
014D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
014F:    8B 86 50 FF FF FF       mov      eax, dword ptr [esi - 0xb0]
0155:    83 E0 01                and      eax, 1
0158:    74 06                   je       0x160    ; Conditional jump
015A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
015C:    D9 E1                   fabs    ; |ST(0)|
015E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0160:    90                      nop
0161:    DD 46 9C                fld      qword ptr [esi - 0x64]
0164:    DC C1                   fadd     st(1), st(0)
0166:    DD D8                   fstp     st(0)
0168:    90                      nop
0169:    DD 46 A4                fld      qword ptr [esi - 0x5c]
016C:    DC C2                   fadd     st(2), st(0)
016E:    DD D8                   fstp     st(0)
0170:    90                      nop
0171:    DD 46 AC                fld      qword ptr [esi - 0x54]
0174:    DC C3                   fadd     st(3), st(0)
0176:    DD D8                   fstp     st(0)
0178:    D9 C0                   fld      st(0)
017A:    D8 C8                   fmul     st(0)
017C:    D9 C2                   fld      st(2)
017E:    D8 C8                   fmul     st(0)
0180:    DE C1                   faddp    st(1)
0182:    D9 C3                   fld      st(3)
0184:    D8 C8                   fmul     st(0)
0186:    DE C1                   faddp    st(1)
0188:    DC 4E 94                fmul     qword ptr [esi - 0x6c]
018B:    90                      nop
018C:    DC 56 E0                fcom     qword ptr [esi - 0x20]
018F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0191:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0193:    73 07                   jae      0x19c    ; Conditional jump
0195:    DD D8                   fstp     st(0)
0197:    DD 46 E8                fld      qword ptr [esi - 0x18]
019A:    EB 16                   jmp      0x1b2
019C:    D9 E8                   fld1    ; Load 1.0
019E:    D8 D1                   fcom     st(1)
01A0:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01A2:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01A4:    72 07                   jb       0x1ad    ; Conditional jump
01A6:    DD D8                   fstp     st(0)
01A8:    DC 7E F0                fdivr    qword ptr [esi - 0x10]
01AB:    EB 05                   jmp      0x1b2
01AD:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
01AF:    DD 46 F0                fld      qword ptr [esi - 0x10]
01B2:    DD 41 08                fld      qword ptr [ecx + 8]
01B5:    D8 C9                   fmul     st(1)
01B7:    DD 59 08                fstp     qword ptr [ecx + 8]
01BA:    DC CB                   fmul     st(3), st(0)
01BC:    DC CA                   fmul     st(2), st(0)
01BE:    DE C9                   fmulp    st(1)
01C0:    90                      nop
01C1:    DD 46 9C                fld      qword ptr [esi - 0x64]
01C4:    DC 4E F0                fmul     qword ptr [esi - 0x10]
01C7:    DC E9                   fsub     st(1), st(0)
01C9:    DD D8                   fstp     st(0)
01CB:    90                      nop
01CC:    DD 46 A4                fld      qword ptr [esi - 0x5c]
01CF:    DC 4E F0                fmul     qword ptr [esi - 0x10]
01D2:    DC EA                   fsub     st(2), st(0)
01D4:    DD D8                   fstp     st(0)
01D6:    90                      nop
01D7:    DD 46 AC                fld      qword ptr [esi - 0x54]
01DA:    DC 4E F0                fmul     qword ptr [esi - 0x10]
01DD:    DC EB                   fsub     st(3), st(0)
01DF:    DD D8                   fstp     st(0)
01E1:    90                      nop
01E2:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
01E4:    D9 C0                   fld      st(0)
01E6:    D8 4E BC                fmul     dword ptr [esi - 0x44]
01E9:    D9 C2                   fld      st(2)
01EB:    D8 4E B8                fmul     dword ptr [esi - 0x48]
01EE:    DE C1                   faddp    st(1)
01F0:    D9 C3                   fld      st(3)
01F2:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
01F5:    DE C1                   faddp    st(1)
01F7:    D9 C1                   fld      st(1)
01F9:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
01FC:    D9 C3                   fld      st(3)
01FE:    D8 4E D0                fmul     dword ptr [esi - 0x30]
0201:    DE C1                   faddp    st(1)
0203:    D9 C4                   fld      st(4)
0205:    D8 4E CC                fmul     dword ptr [esi - 0x34]
0208:    DE C1                   faddp    st(1)
020A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
020C:    D8 4E C8                fmul     dword ptr [esi - 0x38]
020F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0211:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
0214:    DE C3                   faddp    st(3)
0216:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0218:    D8 4E C0                fmul     dword ptr [esi - 0x40]
021B:    DE C2                   faddp    st(2)
021D:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
021F:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0222:    DC 46 28                fadd     qword ptr [esi + 0x28]
0225:    DD 19                   fstp     qword ptr [ecx]
0227:    DC 46 20                fadd     qword ptr [esi + 0x20]
022A:    DD 1A                   fstp     qword ptr [edx]
022C:    DC 46 18                fadd     qword ptr [esi + 0x18]
022F:    DD 1B                   fstp     qword ptr [ebx]
0231:    DD D8                   fstp     st(0)
0233:    83 C4 30                add      esp, 0x30
0236:    89 D8                   mov      eax, ebx
0238:    5B                      pop      ebx
0239:    5E                      pop      esi
023A:    5D                      pop      ebp
023B:    C2 08 00                ret      8
