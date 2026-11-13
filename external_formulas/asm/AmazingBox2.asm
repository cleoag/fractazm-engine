; Disassembly: AmazingBox2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 412 bytes, decoded: 412 bytes (100.0%)
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
0087:    90                      nop
0088:    DC 8E 7C FF FF FF       fmul     qword ptr [esi - 0x84]
008E:    DE E9                   fsubp    st(1)
0090:    90                      nop
0091:    90                      nop
0092:    90                      nop
0093:    90                      nop
0094:    90                      nop
0095:    90                      nop
0096:    90                      nop
0097:    90                      nop
0098:    90                      nop
0099:    90                      nop
009A:    DD 02                   fld      qword ptr [edx]
009C:    D9 C0                   fld      st(0)
009E:    D8 E3                   fsub     st(3)
00A0:    D9 E1                   fabs    ; |ST(0)|
00A2:    D8 C1                   fadd     st(1)
00A4:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00A6:    D8 C3                   fadd     st(3)
00A8:    D9 E1                   fabs    ; |ST(0)|
00AA:    DE E1                   fsubrp   st(1)
00AC:    DD 01                   fld      qword ptr [ecx]
00AE:    D9 C0                   fld      st(0)
00B0:    D8 E4                   fsub     st(4)
00B2:    D9 E1                   fabs    ; |ST(0)|
00B4:    D8 C1                   fadd     st(1)
00B6:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00B8:    D8 C4                   fadd     st(4)
00BA:    D9 E1                   fabs    ; |ST(0)|
00BC:    DE E1                   fsubrp   st(1)
00BE:    90                      nop
00BF:    DD 46 9C                fld      qword ptr [esi - 0x64]
00C2:    DC C1                   fadd     st(1), st(0)
00C4:    DD D8                   fstp     st(0)
00C6:    90                      nop
00C7:    DD 46 A4                fld      qword ptr [esi - 0x5c]
00CA:    DC C2                   fadd     st(2), st(0)
00CC:    DD D8                   fstp     st(0)
00CE:    90                      nop
00CF:    DD 46 AC                fld      qword ptr [esi - 0x54]
00D2:    DC C3                   fadd     st(3), st(0)
00D4:    DD D8                   fstp     st(0)
00D6:    D9 C0                   fld      st(0)
00D8:    D8 C8                   fmul     st(0)
00DA:    D9 C2                   fld      st(2)
00DC:    D8 C8                   fmul     st(0)
00DE:    DE C1                   faddp    st(1)
00E0:    D9 C3                   fld      st(3)
00E2:    D8 C8                   fmul     st(0)
00E4:    DE C1                   faddp    st(1)
00E6:    DC 4E 94                fmul     qword ptr [esi - 0x6c]
00E9:    90                      nop
00EA:    DC 56 E0                fcom     qword ptr [esi - 0x20]
00ED:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00EF:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00F1:    73 07                   jae      0xfa    ; Conditional jump
00F3:    DD D8                   fstp     st(0)
00F5:    DD 46 E8                fld      qword ptr [esi - 0x18]
00F8:    EB 16                   jmp      0x110
00FA:    D9 E8                   fld1    ; Load 1.0
00FC:    D8 D1                   fcom     st(1)
00FE:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0100:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0102:    72 07                   jb       0x10b    ; Conditional jump
0104:    DD D8                   fstp     st(0)
0106:    DC 7E F0                fdivr    qword ptr [esi - 0x10]
0109:    EB 05                   jmp      0x110
010B:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
010D:    DD 46 F0                fld      qword ptr [esi - 0x10]
0110:    DD 41 08                fld      qword ptr [ecx + 8]
0113:    D8 C9                   fmul     st(1)
0115:    DD 59 08                fstp     qword ptr [ecx + 8]
0118:    DC CB                   fmul     st(3), st(0)
011A:    DC CA                   fmul     st(2), st(0)
011C:    DE C9                   fmulp    st(1)
011E:    90                      nop
011F:    DD 46 9C                fld      qword ptr [esi - 0x64]
0122:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0125:    DC E9                   fsub     st(1), st(0)
0127:    DD D8                   fstp     st(0)
0129:    90                      nop
012A:    DD 46 A4                fld      qword ptr [esi - 0x5c]
012D:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0130:    DC EA                   fsub     st(2), st(0)
0132:    DD D8                   fstp     st(0)
0134:    90                      nop
0135:    DD 46 AC                fld      qword ptr [esi - 0x54]
0138:    DC 4E F0                fmul     qword ptr [esi - 0x10]
013B:    DC EB                   fsub     st(3), st(0)
013D:    DD D8                   fstp     st(0)
013F:    90                      nop
0140:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0142:    D9 C0                   fld      st(0)
0144:    D8 4E BC                fmul     dword ptr [esi - 0x44]
0147:    D9 C2                   fld      st(2)
0149:    D8 4E B8                fmul     dword ptr [esi - 0x48]
014C:    DE C1                   faddp    st(1)
014E:    D9 C3                   fld      st(3)
0150:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
0153:    DE C1                   faddp    st(1)
0155:    D9 C1                   fld      st(1)
0157:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
015A:    D9 C3                   fld      st(3)
015C:    D8 4E D0                fmul     dword ptr [esi - 0x30]
015F:    DE C1                   faddp    st(1)
0161:    D9 C4                   fld      st(4)
0163:    D8 4E CC                fmul     dword ptr [esi - 0x34]
0166:    DE C1                   faddp    st(1)
0168:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
016A:    D8 4E C8                fmul     dword ptr [esi - 0x38]
016D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
016F:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
0172:    DE C3                   faddp    st(3)
0174:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0176:    D8 4E C0                fmul     dword ptr [esi - 0x40]
0179:    DE C2                   faddp    st(2)
017B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
017D:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0180:    DC 46 28                fadd     qword ptr [esi + 0x28]
0183:    DD 19                   fstp     qword ptr [ecx]
0185:    DC 46 20                fadd     qword ptr [esi + 0x20]
0188:    DD 1A                   fstp     qword ptr [edx]
018A:    DC 46 18                fadd     qword ptr [esi + 0x18]
018D:    DD 1B                   fstp     qword ptr [ebx]
018F:    DD D8                   fstp     st(0)
0191:    83 C4 30                add      esp, 0x30
0194:    89 D8                   mov      eax, ebx
0196:    5B                      pop      ebx
0197:    5E                      pop      esi
0198:    5D                      pop      ebp
0199:    C2 08 00                ret      8
