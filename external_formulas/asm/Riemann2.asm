; Disassembly: Riemann2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 371 bytes, decoded: 371 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 28 00 00 00       sub      esp, 0x28
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 01                   fld      qword ptr [ecx]
0018:    DD 02                   fld      qword ptr [edx]
001A:    DD 03                   fld      qword ptr [ebx]
001C:    D9 C0                   fld      st(0)
001E:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0021:    D9 C2                   fld      st(2)
0023:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0026:    DE C1                   faddp    st(1)
0028:    D9 C3                   fld      st(3)
002A:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
002D:    DE C1                   faddp    st(1)
002F:    D9 C1                   fld      st(1)
0031:    D8 4F F4                fmul     dword ptr [edi - 0xc]
0034:    D9 C3                   fld      st(3)
0036:    D8 4F F0                fmul     dword ptr [edi - 0x10]
0039:    DE C1                   faddp    st(1)
003B:    D9 C4                   fld      st(4)
003D:    D8 4F EC                fmul     dword ptr [edi - 0x14]
0040:    DE C1                   faddp    st(1)
0042:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0044:    D8 4F E8                fmul     dword ptr [edi - 0x18]
0047:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0049:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
004C:    DE C3                   faddp    st(3)
004E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0050:    D8 4F E0                fmul     dword ptr [edi - 0x20]
0053:    DE C2                   faddp    st(2)
0055:    DD 1B                   fstp     qword ptr [ebx]
0057:    DD 1A                   fstp     qword ptr [edx]
0059:    DD 19                   fstp     qword ptr [ecx]
005B:    D9 D0                   fnop
005D:    DD 03                   fld      qword ptr [ebx]
005F:    D8 C8                   fmul     st(0)
0061:    DD 02                   fld      qword ptr [edx]
0063:    D8 C8                   fmul     st(0)
0065:    DD 01                   fld      qword ptr [ecx]
0067:    D8 C8                   fmul     st(0)
0069:    DE C1                   faddp    st(1)
006B:    DE C1                   faddp    st(1)
006D:    D9 FA                   fsqrt    ; sqrt(ST(0))
006F:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0072:    DD 47 CC                fld      qword ptr [edi - 0x34]
0075:    DE F1                   fdivrp   st(1)
0077:    90                      nop
0078:    DD 03                   fld      qword ptr [ebx]
007A:    D8 C9                   fmul     st(1)
007C:    DD 1B                   fstp     qword ptr [ebx]
007E:    DD 02                   fld      qword ptr [edx]
0080:    D8 C9                   fmul     st(1)
0082:    DD 1A                   fstp     qword ptr [edx]
0084:    DD 01                   fld      qword ptr [ecx]
0086:    D8 C9                   fmul     st(1)
0088:    DD 19                   fstp     qword ptr [ecx]
008A:    DD D8                   fstp     st(0)
008C:    90                      nop
008D:    90                      nop
008E:    90                      nop
008F:    90                      nop
0090:    D9 E8                   fld1    ; Load 1.0
0092:    DC 23                   fsub     qword ptr [ebx]
0094:    D9 C0                   fld      st(0)
0096:    DD 47 94                fld      qword ptr [edi - 0x6c]
0099:    DA E9                   fucompp
009B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
009F:    72 09                   jb       0xaa    ; Conditional jump
00A1:    DD D8                   fstp     st(0)
00A3:    D9 E8                   fld1    ; Load 1.0
00A5:    90                      nop
00A6:    90                      nop
00A7:    90                      nop
00A8:    EB 05                   jmp      0xaf
00AA:    D9 E8                   fld1    ; Load 1.0
00AC:    DE F1                   fdivrp   st(1)
00AE:    90                      nop
00AF:    DD 01                   fld      qword ptr [ecx]
00B1:    D8 C9                   fmul     st(1)
00B3:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00B6:    DD 02                   fld      qword ptr [edx]
00B8:    D8 C9                   fmul     st(1)
00BA:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00BD:    DD D8                   fstp     st(0)
00BF:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00C2:    D8 C8                   fmul     st(0)
00C4:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00C7:    D8 C8                   fmul     st(0)
00C9:    DE C1                   faddp    st(1)
00CB:    D9 E8                   fld1    ; Load 1.0
00CD:    DE C1                   faddp    st(1)
00CF:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00D2:    90                      nop
00D3:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00D6:    DC 0F                   fmul     qword ptr [edi]
00D8:    DC 47 B4                fadd     qword ptr [edi - 0x4c]
00DB:    D9 FE                   fsin    ; sin(ST(0))
00DD:    D9 E1                   fabs    ; |ST(0)|
00DF:    DD 47 9C                fld      qword ptr [edi - 0x64]
00E2:    DE C1                   faddp    st(1)
00E4:    D9 E1                   fabs    ; |ST(0)|
00E6:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00E9:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00EC:    DC 0F                   fmul     qword ptr [edi]
00EE:    DC 47 AC                fadd     qword ptr [edi - 0x54]
00F1:    D9 FE                   fsin    ; sin(ST(0))
00F3:    D9 E1                   fabs    ; |ST(0)|
00F5:    DD 47 9C                fld      qword ptr [edi - 0x64]
00F8:    DE C1                   faddp    st(1)
00FA:    D9 E1                   fabs    ; |ST(0)|
00FC:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00FF:    90                      nop
0100:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0103:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0106:    DC 4F C4                fmul     qword ptr [edi - 0x3c]
0109:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
010B:    D9 E1                   fabs    ; |ST(0)|
010D:    D9 F1                   fyl2x
010F:    D9 E8                   fld1    ; Load 1.0
0111:    D9 C1                   fld      st(1)
0113:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
0115:    D9 F0                   f2xm1
0117:    DE C1                   faddp    st(1)
0119:    D9 FD                   fscale
011B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
011D:    DD D8                   fstp     st(0)
011F:    DC 47 BC                fadd     qword ptr [edi - 0x44]
0122:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0125:    90                      nop
0126:    DD 47 08                fld      qword ptr [edi + 8]
0129:    DD 45 E8                fld      qword ptr [ebp - 0x18]
012C:    D8 C8                   fmul     st(0)
012E:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0131:    D8 C8                   fmul     st(0)
0133:    DE C1                   faddp    st(1)
0135:    D9 E8                   fld1    ; Load 1.0
0137:    DE C1                   faddp    st(1)
0139:    DE F9                   fdivp    st(1)
013B:    DD 55 D8                fst      qword ptr [ebp - 0x28]
013E:    90                      nop
013F:    D9 E0                   fchs    ; -ST(0)
0141:    D9 E8                   fld1    ; Load 1.0
0143:    DE C1                   faddp    st(1)
0145:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0148:    90                      nop
0149:    DD 45 F0                fld      qword ptr [ebp - 0x10]
014C:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
014F:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0152:    90                      nop
0153:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0156:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0159:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
015C:    90                      nop
015D:    DD 19                   fstp     qword ptr [ecx]
015F:    DD 1A                   fstp     qword ptr [edx]
0161:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
0164:    DD 1B                   fstp     qword ptr [ebx]
0166:    D9 D0                   fnop
0168:    8B C3                   mov      eax, ebx
016A:    5F                      pop      edi
016B:    5E                      pop      esi
016C:    5B                      pop      ebx
016D:    8B E5                   mov      esp, ebp
016F:    5D                      pop      ebp
0170:    C2 08 00                ret      8
