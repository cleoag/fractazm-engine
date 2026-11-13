; Disassembly: BT1Pinehedron.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 493 bytes, decoded: 493 bytes (100.0%)

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
00FB:    75 7A                   jne      0x177    ; Conditional jump
00FD:    DD 02                   fld      qword ptr [edx]
00FF:    D8 C8                   fmul     st(0)
0101:    DD 55 F8                fst      qword ptr [ebp - 8]
0104:    DD 01                   fld      qword ptr [ecx]
0106:    D8 C8                   fmul     st(0)
0108:    DD 55 F0                fst      qword ptr [ebp - 0x10]
010B:    DE F9                   fdivp    st(1)
010D:    D9 E8                   fld1    ; Load 1.0
010F:    D8 D9                   fcomp    st(1)
0111:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0113:    80 E4 41                and      ah, 0x41
0116:    7E 04                   jle      0x11c    ; Conditional jump
0118:    D9 E8                   fld1    ; Load 1.0
011A:    DE F1                   fdivrp   st(1)
011C:    D9 E8                   fld1    ; Load 1.0
011E:    DE C1                   faddp    st(1)
0120:    D9 FA                   fsqrt    ; sqrt(ST(0))
0122:    D9 E8                   fld1    ; Load 1.0
0124:    DE F1                   fdivrp   st(1)
0126:    D9 C0                   fld      st(0)
0128:    DC 0A                   fmul     qword ptr [edx]
012A:    DD 55 F8                fst      qword ptr [ebp - 8]
012D:    D8 C8                   fmul     st(0)
012F:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0132:    90                      nop
0133:    DC 09                   fmul     qword ptr [ecx]
0135:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0138:    D8 C8                   fmul     st(0)
013A:    DC 45 E0                fadd     qword ptr [ebp - 0x20]
013D:    DD 03                   fld      qword ptr [ebx]
013F:    D8 C8                   fmul     st(0)
0141:    DE F9                   fdivp    st(1)
0143:    D9 E8                   fld1    ; Load 1.0
0145:    D8 D9                   fcomp    st(1)
0147:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0149:    80 E4 41                and      ah, 0x41
014C:    7E 04                   jle      0x152    ; Conditional jump
014E:    D9 E8                   fld1    ; Load 1.0
0150:    DE F1                   fdivrp   st(1)
0152:    D9 E8                   fld1    ; Load 1.0
0154:    DE C1                   faddp    st(1)
0156:    D9 FA                   fsqrt    ; sqrt(ST(0))
0158:    D9 E8                   fld1    ; Load 1.0
015A:    DE F1                   fdivrp   st(1)
015C:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
015F:    90                      nop
0160:    DD 03                   fld      qword ptr [ebx]
0162:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0165:    DD 45 F8                fld      qword ptr [ebp - 8]
0168:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
016B:    DD 45 F0                fld      qword ptr [ebp - 0x10]
016E:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0171:    DD 19                   fstp     qword ptr [ecx]
0173:    DD 1A                   fstp     qword ptr [edx]
0175:    DD 1B                   fstp     qword ptr [ebx]
0177:    D9 D0                   fnop
0179:    DD 03                   fld      qword ptr [ebx]
017B:    D8 C8                   fmul     st(0)
017D:    DD 5D F8                fstp     qword ptr [ebp - 8]
0180:    DD 02                   fld      qword ptr [edx]
0182:    D8 C8                   fmul     st(0)
0184:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0187:    DD 01                   fld      qword ptr [ecx]
0189:    D8 C8                   fmul     st(0)
018B:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
018E:    DD 03                   fld      qword ptr [ebx]
0190:    D8 C0                   fadd     st(0)
0192:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0195:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0198:    D9 FA                   fsqrt    ; sqrt(ST(0))
019A:    DE F9                   fdivp    st(1)
019C:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
019F:    90                      nop
01A0:    DD 45 F8                fld      qword ptr [ebp - 8]
01A3:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
01A6:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
01A9:    90                      nop
01AA:    DD 45 E0                fld      qword ptr [ebp - 0x20]
01AD:    DD 45 F0                fld      qword ptr [ebp - 0x10]
01B0:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
01B3:    DE C9                   fmulp    st(1)
01B5:    90                      nop
01B6:    DD 45 E0                fld      qword ptr [ebp - 0x20]
01B9:    D8 C0                   fadd     st(0)
01BB:    DC 09                   fmul     qword ptr [ecx]
01BD:    DC 0A                   fmul     qword ptr [edx]
01BF:    90                      nop
01C0:    DD 46 20                fld      qword ptr [esi + 0x20]
01C3:    DC 4F D0                fmul     qword ptr [edi - 0x30]
01C6:    DE C1                   faddp    st(1)
01C8:    DD 1A                   fstp     qword ptr [edx]
01CA:    90                      nop
01CB:    DD 46 28                fld      qword ptr [esi + 0x28]
01CE:    DC 4F D8                fmul     qword ptr [edi - 0x28]
01D1:    DE C1                   faddp    st(1)
01D3:    DD 19                   fstp     qword ptr [ecx]
01D5:    90                      nop
01D6:    DD 46 18                fld      qword ptr [esi + 0x18]
01D9:    DC 4F E0                fmul     qword ptr [edi - 0x20]
01DC:    DE C1                   faddp    st(1)
01DE:    DD 1B                   fstp     qword ptr [ebx]
01E0:    D9 D0                   fnop
01E2:    8B C3                   mov      eax, ebx
01E4:    5F                      pop      edi
01E5:    5E                      pop      esi
01E6:    5B                      pop      ebx
01E7:    89 EC                   mov      esp, ebp
01E9:    5D                      pop      ebp
01EA:    C2 08 00                ret      8
