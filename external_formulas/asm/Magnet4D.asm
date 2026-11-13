; Disassembly: Magnet4D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 545 bytes, decoded: 545 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 48 00 00 00       sub      esp, 0x48
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    83 BF D4 FF FF FF 00    cmp      dword ptr [edi - 0x2c], 0
001D:    74 25                   je       0x44    ; Conditional jump
001F:    83 BE D0 00 00 00 00    cmp      dword ptr [esi + 0xd0], 0
0026:    75 1C                   jne      0x44    ; Conditional jump
0028:    D9 EE                   fldz    ; Load 0.0
002A:    DD 13                   fst      qword ptr [ebx]
002C:    DD 12                   fst      qword ptr [edx]
002E:    DD 55 E0                fst      qword ptr [ebp - 0x20]
0031:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0034:    DD 01                   fld      qword ptr [ecx]
0036:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0039:    FF 86 D0 00 00 00       inc      dword ptr [esi + 0xd0]
003F:    EB 55                   jmp      0x96
0041:    90                      nop
0042:    90                      nop
0043:    90                      nop
0044:    DD 03                   fld      qword ptr [ebx]
0046:    D8 C8                   fmul     st(0)
0048:    DD 02                   fld      qword ptr [edx]
004A:    D8 C8                   fmul     st(0)
004C:    DE C1                   faddp    st(1)
004E:    90                      nop
004F:    90                      nop
0050:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0053:    90                      nop
0054:    DD 01                   fld      qword ptr [ecx]
0056:    D8 C8                   fmul     st(0)
0058:    DC 75 F0                fdiv     qword ptr [ebp - 0x10]
005B:    D9 E0                   fchs    ; -ST(0)
005D:    D9 E8                   fld1    ; Load 1.0
005F:    DE C1                   faddp    st(1)
0061:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0064:    90                      nop
0065:    DD 03                   fld      qword ptr [ebx]
0067:    D8 C8                   fmul     st(0)
0069:    DD 02                   fld      qword ptr [edx]
006B:    D8 C8                   fmul     st(0)
006D:    DE E9                   fsubp    st(1)
006F:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0072:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0075:    D9 D0                   fnop
0077:    DD 03                   fld      qword ptr [ebx]
0079:    DC 0A                   fmul     qword ptr [edx]
007B:    DC 0F                   fmul     qword ptr [edi]
007D:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0080:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0083:    D9 D0                   fnop
0085:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0088:    D9 FA                   fsqrt    ; sqrt(ST(0))
008A:    DC 09                   fmul     qword ptr [ecx]
008C:    DC 0F                   fmul     qword ptr [edi]
008E:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0091:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0094:    D9 D0                   fnop
0096:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0099:    DD 46 18                fld      qword ptr [esi + 0x18]
009C:    DC 4F E8                fmul     qword ptr [edi - 0x18]
009F:    DE C1                   faddp    st(1)
00A1:    DD 47 D8                fld      qword ptr [edi - 0x28]
00A4:    90                      nop
00A5:    DE C1                   faddp    st(1)
00A7:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00AA:    D9 D0                   fnop
00AC:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00AF:    DD 46 20                fld      qword ptr [esi + 0x20]
00B2:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00B5:    DE C1                   faddp    st(1)
00B7:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00BA:    D9 D0                   fnop
00BC:    DD 45 D0                fld      qword ptr [ebp - 0x30]
00BF:    DD 46 28                fld      qword ptr [esi + 0x28]
00C2:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00C5:    DE C1                   faddp    st(1)
00C7:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00CA:    D9 D0                   fnop
00CC:    DD 03                   fld      qword ptr [ebx]
00CE:    DC 0F                   fmul     qword ptr [edi]
00D0:    DD 47 D8                fld      qword ptr [edi - 0x28]
00D3:    D9 E8                   fld1    ; Load 1.0
00D5:    D9 E0                   fchs    ; -ST(0)
00D7:    DE C1                   faddp    st(1)
00D9:    DE C1                   faddp    st(1)
00DB:    DD 46 18                fld      qword ptr [esi + 0x18]
00DE:    DC 4F E0                fmul     qword ptr [edi - 0x20]
00E1:    DE C1                   faddp    st(1)
00E3:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
00E6:    D9 D0                   fnop
00E8:    DD 02                   fld      qword ptr [edx]
00EA:    DC 0F                   fmul     qword ptr [edi]
00EC:    DD 46 20                fld      qword ptr [esi + 0x20]
00EF:    DC 4F E0                fmul     qword ptr [edi - 0x20]
00F2:    DE C1                   faddp    st(1)
00F4:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
00F7:    D9 D0                   fnop
00F9:    DD 01                   fld      qword ptr [ecx]
00FB:    DC 0F                   fmul     qword ptr [edi]
00FD:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0100:    DD 46 28                fld      qword ptr [esi + 0x28]
0103:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0106:    DE C1                   faddp    st(1)
0108:    90                      nop
0109:    90                      nop
010A:    90                      nop
010B:    90                      nop
010C:    90                      nop
010D:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
0110:    D9 D0                   fnop
0112:    DD 45 C8                fld      qword ptr [ebp - 0x38]
0115:    D8 C8                   fmul     st(0)
0117:    DD 45 C0                fld      qword ptr [ebp - 0x40]
011A:    D8 C8                   fmul     st(0)
011C:    DD 45 B8                fld      qword ptr [ebp - 0x48]
011F:    D8 C8                   fmul     st(0)
0121:    DE C1                   faddp    st(1)
0123:    DE C1                   faddp    st(1)
0125:    DD 5D F8                fstp     qword ptr [ebp - 8]
0128:    90                      nop
0129:    DD 45 E0                fld      qword ptr [ebp - 0x20]
012C:    D8 C8                   fmul     st(0)
012E:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0131:    D8 C8                   fmul     st(0)
0133:    DE C1                   faddp    st(1)
0135:    D9 FA                   fsqrt    ; sqrt(ST(0))
0137:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
013A:    90                      nop
013B:    DD 45 C8                fld      qword ptr [ebp - 0x38]
013E:    D8 C8                   fmul     st(0)
0140:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0143:    D8 C8                   fmul     st(0)
0145:    DE C1                   faddp    st(1)
0147:    D9 FA                   fsqrt    ; sqrt(ST(0))
0149:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
014C:    90                      nop
014D:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0150:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0153:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0156:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
0159:    DE E9                   fsubp    st(1)
015B:    DC 75 F8                fdiv     qword ptr [ebp - 8]
015E:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0161:    DD 19                   fstp     qword ptr [ecx]
0163:    D9 D0                   fnop
0165:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0168:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
016B:    DC 75 F0                fdiv     qword ptr [ebp - 0x10]
016E:    DC 75 E8                fdiv     qword ptr [ebp - 0x18]
0171:    D9 E8                   fld1    ; Load 1.0
0173:    DE C1                   faddp    st(1)
0175:    DC 75 F8                fdiv     qword ptr [ebp - 8]
0178:    DD 5D F8                fstp     qword ptr [ebp - 8]
017B:    90                      nop
017C:    DD 45 E0                fld      qword ptr [ebp - 0x20]
017F:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0182:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0185:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0188:    DE C1                   faddp    st(1)
018A:    DC 4D F8                fmul     qword ptr [ebp - 8]
018D:    DD 1B                   fstp     qword ptr [ebx]
018F:    D9 D0                   fnop
0191:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0194:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0197:    DD 45 D8                fld      qword ptr [ebp - 0x28]
019A:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
019D:    DE E1                   fsubrp   st(1)
019F:    DC 4D F8                fmul     qword ptr [ebp - 8]
01A2:    DD 1A                   fstp     qword ptr [edx]
01A4:    D9 D0                   fnop
01A6:    DD 03                   fld      qword ptr [ebx]
01A8:    D8 C8                   fmul     st(0)
01AA:    DD 02                   fld      qword ptr [edx]
01AC:    D8 C8                   fmul     st(0)
01AE:    DE C1                   faddp    st(1)
01B0:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
01B3:    D9 D0                   fnop
01B5:    DD 01                   fld      qword ptr [ecx]
01B7:    D8 C8                   fmul     st(0)
01B9:    DC 75 F0                fdiv     qword ptr [ebp - 0x10]
01BC:    D9 E0                   fchs    ; -ST(0)
01BE:    D9 E8                   fld1    ; Load 1.0
01C0:    DE C1                   faddp    st(1)
01C2:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
01C5:    90                      nop
01C6:    DD 03                   fld      qword ptr [ebx]
01C8:    D8 C8                   fmul     st(0)
01CA:    DD 02                   fld      qword ptr [edx]
01CC:    D8 C8                   fmul     st(0)
01CE:    DE E9                   fsubp    st(1)
01D0:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
01D3:    D9 D0                   fnop
01D5:    DD 03                   fld      qword ptr [ebx]
01D7:    DC 0A                   fmul     qword ptr [edx]
01D9:    DC 0F                   fmul     qword ptr [edi]
01DB:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
01DE:    D9 D0                   fnop
01E0:    DD 45 F0                fld      qword ptr [ebp - 0x10]
01E3:    D9 FA                   fsqrt    ; sqrt(ST(0))
01E5:    DC 09                   fmul     qword ptr [ecx]
01E7:    DC 0F                   fmul     qword ptr [edi]
01E9:    DC 4F F0                fmul     qword ptr [edi - 0x10]
01EC:    D9 D0                   fnop
01EE:    DD 19                   fstp     qword ptr [ecx]
01F0:    DD 1A                   fstp     qword ptr [edx]
01F2:    DD 1B                   fstp     qword ptr [ebx]
01F4:    D9 D0                   fnop
01F6:    DD 03                   fld      qword ptr [ebx]
01F8:    D9 E8                   fld1    ; Load 1.0
01FA:    D9 E0                   fchs    ; -ST(0)
01FC:    DE C1                   faddp    st(1)
01FE:    D8 C8                   fmul     st(0)
0200:    DD 02                   fld      qword ptr [edx]
0202:    D8 C8                   fmul     st(0)
0204:    DD 01                   fld      qword ptr [ecx]
0206:    D8 C8                   fmul     st(0)
0208:    DE C1                   faddp    st(1)
020A:    DE C1                   faddp    st(1)
020C:    D9 E8                   fld1    ; Load 1.0
020E:    DE F1                   fdivrp   st(1)
0210:    DD 59 08                fstp     qword ptr [ecx + 8]
0213:    90                      nop
0214:    D9 D0                   fnop
0216:    8B C3                   mov      eax, ebx
0218:    5F                      pop      edi
0219:    5E                      pop      esi
021A:    5B                      pop      ebx
021B:    8B E5                   mov      esp, ebp
021D:    5D                      pop      ebp
021E:    C2 08 00                ret      8
