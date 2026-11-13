; Disassembly: BT1Pinegon.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 430 bytes, decoded: 430 bytes (100.0%)

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
0016:    8A 47 BC                mov      al, byte ptr [edi - 0x44]
0019:    DD 03                   fld      qword ptr [ebx]
001B:    DC 0F                   fmul     qword ptr [edi]
001D:    DD 01                   fld      qword ptr [ecx]
001F:    DC 4F 08                fmul     qword ptr [edi + 8]
0022:    D9 E0                   fchs    ; -ST(0)
0024:    DE C1                   faddp    st(1)
0026:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0029:    DD 5D F8                fstp     qword ptr [ebp - 8]
002C:    DD 03                   fld      qword ptr [ebx]
002E:    DC 4F 08                fmul     qword ptr [edi + 8]
0031:    DD 01                   fld      qword ptr [ecx]
0033:    DC 0F                   fmul     qword ptr [edi]
0035:    DE C1                   faddp    st(1)
0037:    D9 E1                   fabs    ; |ST(0)|
0039:    90                      nop
003A:    DD 45 F8                fld      qword ptr [ebp - 8]
003D:    DD 02                   fld      qword ptr [edx]
003F:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0042:    D9 E0                   fchs    ; -ST(0)
0044:    DE C1                   faddp    st(1)
0046:    D9 E1                   fabs    ; |ST(0)|
0048:    90                      nop
0049:    DD 45 F8                fld      qword ptr [ebp - 8]
004C:    DD 02                   fld      qword ptr [edx]
004E:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0051:    DE C1                   faddp    st(1)
0053:    D9 E1                   fabs    ; |ST(0)|
0055:    90                      nop
0056:    DD 1A                   fstp     qword ptr [edx]
0058:    DD 1B                   fstp     qword ptr [ebx]
005A:    DD 19                   fstp     qword ptr [ecx]
005C:    90                      nop
005D:    DD 03                   fld      qword ptr [ebx]
005F:    DC 02                   fadd     qword ptr [edx]
0061:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0064:    DD 5D F8                fstp     qword ptr [ebp - 8]
0067:    90                      nop
0068:    DD 03                   fld      qword ptr [ebx]
006A:    D9 E0                   fchs    ; -ST(0)
006C:    DC 02                   fadd     qword ptr [edx]
006E:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0071:    90                      nop
0072:    DD 45 F8                fld      qword ptr [ebp - 8]
0075:    DC 0F                   fmul     qword ptr [edi]
0077:    DD 01                   fld      qword ptr [ecx]
0079:    DC 4F 08                fmul     qword ptr [edi + 8]
007C:    DE C1                   faddp    st(1)
007E:    90                      nop
007F:    DD 45 F8                fld      qword ptr [ebp - 8]
0082:    DC 4F 08                fmul     qword ptr [edi + 8]
0085:    D9 E0                   fchs    ; -ST(0)
0087:    DD 01                   fld      qword ptr [ecx]
0089:    DC 0F                   fmul     qword ptr [edi]
008B:    DE C1                   faddp    st(1)
008D:    90                      nop
008E:    DD 19                   fstp     qword ptr [ecx]
0090:    DD 1B                   fstp     qword ptr [ebx]
0092:    DD 1A                   fstp     qword ptr [edx]
0094:    90                      nop
0095:    DD 03                   fld      qword ptr [ebx]
0097:    DC 4F F0                fmul     qword ptr [edi - 0x10]
009A:    DC 67 E8                fsub     qword ptr [edi - 0x18]
009D:    DD 1B                   fstp     qword ptr [ebx]
009F:    DD 02                   fld      qword ptr [edx]
00A1:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00A4:    DD 1A                   fstp     qword ptr [edx]
00A6:    DD 01                   fld      qword ptr [ecx]
00A8:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00AB:    DD 19                   fstp     qword ptr [ecx]
00AD:    90                      nop
00AE:    FE C8                   dec      al
00B0:    0F 85 63 FF FF FF       jne      0x19    ; Conditional jump
00B6:    D9 D0                   fnop
00B8:    8A 47 C4                mov      al, byte ptr [edi - 0x3c]
00BB:    3A 46 40                cmp      al, byte ptr [esi + 0x40]
00BE:    74 08                   je       0xc8    ; Conditional jump
00C0:    8A 47 C0                mov      al, byte ptr [edi - 0x40]
00C3:    3A 46 40                cmp      al, byte ptr [esi + 0x40]
00C6:    7F 2B                   jg       0xf3    ; Conditional jump
00C8:    DD 47 C8                fld      qword ptr [edi - 0x38]
00CB:    D9 D0                   fnop
00CD:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00CF:    DD 5D F8                fstp     qword ptr [ebp - 8]
00D2:    90                      nop
00D3:    DD 55 F0                fst      qword ptr [ebp - 0x10]
00D6:    DD 01                   fld      qword ptr [ecx]
00D8:    DE C9                   fmulp    st(1)
00DA:    DD 02                   fld      qword ptr [edx]
00DC:    DC 4D F8                fmul     qword ptr [ebp - 8]
00DF:    DE C1                   faddp    st(1)
00E1:    DD 02                   fld      qword ptr [edx]
00E3:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00E6:    DD 01                   fld      qword ptr [ecx]
00E8:    DC 4D F8                fmul     qword ptr [ebp - 8]
00EB:    D9 E0                   fchs    ; -ST(0)
00ED:    DE C1                   faddp    st(1)
00EF:    DD 1A                   fstp     qword ptr [edx]
00F1:    DD 19                   fstp     qword ptr [ecx]
00F3:    D9 D0                   fnop
00F5:    8A 47 B8                mov      al, byte ptr [edi - 0x48]
00F8:    3A 46 40                cmp      al, byte ptr [esi + 0x40]
00FB:    74 08                   je       0x105    ; Conditional jump
00FD:    8A 47 B4                mov      al, byte ptr [edi - 0x4c]
0100:    3A 46 40                cmp      al, byte ptr [esi + 0x40]
0103:    7F 33                   jg       0x138    ; Conditional jump
0105:    DD 47 18                fld      qword ptr [edi + 0x18]
0108:    DC 77 AC                fdiv     qword ptr [edi - 0x54]
010B:    DD 55 F8                fst      qword ptr [ebp - 8]
010E:    D8 C0                   fadd     st(0)
0110:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0113:    DD 01                   fld      qword ptr [ecx]
0115:    DD 02                   fld      qword ptr [edx]
0117:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0119:    D9 E1                   fabs    ; |ST(0)|
011B:    DC 55 F8                fcom     qword ptr [ebp - 8]
011E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0120:    80 E4 41                and      ah, 0x41
0123:    7F 05                   jg       0x12a    ; Conditional jump
0125:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
0128:    EB F1                   jmp      0x11b
012A:    D9 FF                   fcos    ; cos(ST(0))
012C:    D9 C0                   fld      st(0)
012E:    DC 0A                   fmul     qword ptr [edx]
0130:    DD 1A                   fstp     qword ptr [edx]
0132:    D9 D0                   fnop
0134:    DC 09                   fmul     qword ptr [ecx]
0136:    DD 19                   fstp     qword ptr [ecx]
0138:    D9 D0                   fnop
013A:    DD 03                   fld      qword ptr [ebx]
013C:    D8 C8                   fmul     st(0)
013E:    DD 5D F8                fstp     qword ptr [ebp - 8]
0141:    DD 02                   fld      qword ptr [edx]
0143:    D8 C8                   fmul     st(0)
0145:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0148:    DD 01                   fld      qword ptr [ecx]
014A:    D8 C8                   fmul     st(0)
014C:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
014F:    DD 03                   fld      qword ptr [ebx]
0151:    D8 C0                   fadd     st(0)
0153:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0156:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0159:    D9 FA                   fsqrt    ; sqrt(ST(0))
015B:    DE F9                   fdivp    st(1)
015D:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0160:    90                      nop
0161:    DD 45 F8                fld      qword ptr [ebp - 8]
0164:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
0167:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
016A:    90                      nop
016B:    DD 45 E0                fld      qword ptr [ebp - 0x20]
016E:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0171:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
0174:    DE C9                   fmulp    st(1)
0176:    90                      nop
0177:    DD 45 E0                fld      qword ptr [ebp - 0x20]
017A:    D8 C0                   fadd     st(0)
017C:    DC 09                   fmul     qword ptr [ecx]
017E:    DC 0A                   fmul     qword ptr [edx]
0180:    90                      nop
0181:    DD 46 20                fld      qword ptr [esi + 0x20]
0184:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0187:    DE C1                   faddp    st(1)
0189:    DD 1A                   fstp     qword ptr [edx]
018B:    90                      nop
018C:    DD 46 28                fld      qword ptr [esi + 0x28]
018F:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0192:    DE C1                   faddp    st(1)
0194:    DD 19                   fstp     qword ptr [ecx]
0196:    90                      nop
0197:    DD 46 18                fld      qword ptr [esi + 0x18]
019A:    DC 4F E0                fmul     qword ptr [edi - 0x20]
019D:    DE C1                   faddp    st(1)
019F:    DD 1B                   fstp     qword ptr [ebx]
01A1:    D9 D0                   fnop
01A3:    8B C3                   mov      eax, ebx
01A5:    5F                      pop      edi
01A6:    5E                      pop      esi
01A7:    5B                      pop      ebx
01A8:    89 EC                   mov      esp, ebp
01AA:    5D                      pop      ebp
01AB:    C2 08 00                ret      8
