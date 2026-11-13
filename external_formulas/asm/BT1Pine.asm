; Disassembly: BT1Pine.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 299 bytes, decoded: 299 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 40 00 00 00       sub      esp, 0x40
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    8A 47 CC                mov      al, byte ptr [edi - 0x34]
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
00B6:    DD 03                   fld      qword ptr [ebx]
00B8:    D8 C8                   fmul     st(0)
00BA:    DD 5D F8                fstp     qword ptr [ebp - 8]
00BD:    DD 02                   fld      qword ptr [edx]
00BF:    D8 C8                   fmul     st(0)
00C1:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00C4:    DD 01                   fld      qword ptr [ecx]
00C6:    D8 C8                   fmul     st(0)
00C8:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00CB:    DD 03                   fld      qword ptr [ebx]
00CD:    D8 C0                   fadd     st(0)
00CF:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00D2:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
00D5:    D9 FA                   fsqrt    ; sqrt(ST(0))
00D7:    DE F9                   fdivp    st(1)
00D9:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00DC:    90                      nop
00DD:    DD 45 F8                fld      qword ptr [ebp - 8]
00E0:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
00E3:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
00E6:    90                      nop
00E7:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00EA:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00ED:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
00F0:    DE C9                   fmulp    st(1)
00F2:    90                      nop
00F3:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00F6:    D8 C0                   fadd     st(0)
00F8:    DC 09                   fmul     qword ptr [ecx]
00FA:    DC 0A                   fmul     qword ptr [edx]
00FC:    90                      nop
00FD:    DD 46 20                fld      qword ptr [esi + 0x20]
0100:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0103:    DE C1                   faddp    st(1)
0105:    DD 1A                   fstp     qword ptr [edx]
0107:    90                      nop
0108:    DD 46 28                fld      qword ptr [esi + 0x28]
010B:    DC 4F D8                fmul     qword ptr [edi - 0x28]
010E:    DE C1                   faddp    st(1)
0110:    DD 19                   fstp     qword ptr [ecx]
0112:    90                      nop
0113:    DD 46 18                fld      qword ptr [esi + 0x18]
0116:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0119:    DE C1                   faddp    st(1)
011B:    DD 1B                   fstp     qword ptr [ebx]
011D:    90                      nop
011E:    D9 D0                   fnop
0120:    8B C3                   mov      eax, ebx
0122:    5F                      pop      edi
0123:    5E                      pop      esi
0124:    5B                      pop      ebx
0125:    89 EC                   mov      esp, ebp
0127:    5D                      pop      ebp
0128:    C2 08 00                ret      8
