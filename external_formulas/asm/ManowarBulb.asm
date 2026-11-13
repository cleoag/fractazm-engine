; Disassembly: ManowarBulb.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 352 bytes, decoded: 352 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 50 00 00 00       sub      esp, 0x50
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    81 C6 80 00 00 00       add      esi, 0x80
001C:    83 7E 50 00             cmp      dword ptr [esi + 0x50], 0
0020:    75 13                   jne      0x35    ; Conditional jump
0022:    FF 46 50                inc      dword ptr [esi + 0x50]
0025:    0F 57 C0                xorps    xmm0, xmm0
0028:    0F 57 C9                xorps    xmm1, xmm1
002B:    E9 25 01 00 00          jmp      0x155
0030:    90                      nop
0031:    90                      nop
0032:    90                      nop
0033:    90                      nop
0034:    90                      nop
0035:    81 EE 80 00 00 00       sub      esi, 0x80
003B:    66 0F 11 45 C8          movupd   xmmword ptr [ebp - 0x38], xmm0
0040:    66 0F 11 4D B8          movupd   xmmword ptr [ebp - 0x48], xmm1
0045:    DD 03                   fld      qword ptr [ebx]
0047:    D8 C8                   fmul     st(0)
0049:    DD 02                   fld      qword ptr [edx]
004B:    D8 C8                   fmul     st(0)
004D:    DD 01                   fld      qword ptr [ecx]
004F:    D8 C8                   fmul     st(0)
0051:    DE C1                   faddp    st(1)
0053:    DE C1                   faddp    st(1)
0055:    D9 FA                   fsqrt    ; sqrt(ST(0))
0057:    DD 47 F0                fld      qword ptr [edi - 0x10]
005A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
005C:    D9 E1                   fabs    ; |ST(0)|
005E:    D9 F1                   fyl2x
0060:    D9 E8                   fld1    ; Load 1.0
0062:    D9 C1                   fld      st(1)
0064:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
0066:    D9 F0                   f2xm1
0068:    DE C1                   faddp    st(1)
006A:    D9 FD                   fscale
006C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006E:    DD D8                   fstp     st(0)
0070:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0073:    DD 02                   fld      qword ptr [edx]
0075:    DD 03                   fld      qword ptr [ebx]
0077:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0079:    DC 4F F0                fmul     qword ptr [edi - 0x10]
007C:    D9 FB                   fsincos    ; ST(0)=cos, push sin
007E:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0081:    DD 5D F8                fstp     qword ptr [ebp - 8]
0084:    DD 01                   fld      qword ptr [ecx]
0086:    DD 02                   fld      qword ptr [edx]
0088:    D8 C8                   fmul     st(0)
008A:    DD 03                   fld      qword ptr [ebx]
008C:    D8 C8                   fmul     st(0)
008E:    DE C1                   faddp    st(1)
0090:    D9 FA                   fsqrt    ; sqrt(ST(0))
0092:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0094:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0097:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0099:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
009C:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
009F:    DD 47 E0                fld      qword ptr [edi - 0x20]
00A2:    DD 45 C0                fld      qword ptr [ebp - 0x40]
00A5:    D8 C9                   fmul     st(1)
00A7:    DD 45 D0                fld      qword ptr [ebp - 0x30]
00AA:    D8 CA                   fmul     st(2)
00AC:    DD 45 C8                fld      qword ptr [ebp - 0x38]
00AF:    D8 CB                   fmul     st(3)
00B1:    D9 C0                   fld      st(0)
00B3:    D8 4F BC                fmul     dword ptr [edi - 0x44]
00B6:    D9 C2                   fld      st(2)
00B8:    D8 4F B8                fmul     dword ptr [edi - 0x48]
00BB:    DE C1                   faddp    st(1)
00BD:    D9 C3                   fld      st(3)
00BF:    D8 4F B4                fmul     dword ptr [edi - 0x4c]
00C2:    DE C1                   faddp    st(1)
00C4:    D9 C1                   fld      st(1)
00C6:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00C9:    D9 C3                   fld      st(3)
00CB:    D8 4F D0                fmul     dword ptr [edi - 0x30]
00CE:    DE C1                   faddp    st(1)
00D0:    D9 C4                   fld      st(4)
00D2:    D8 4F CC                fmul     dword ptr [edi - 0x34]
00D5:    DE C1                   faddp    st(1)
00D7:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00D9:    D8 4F C8                fmul     dword ptr [edi - 0x38]
00DC:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00DE:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
00E1:    DE C3                   faddp    st(3)
00E3:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E5:    D8 4F C0                fmul     dword ptr [edi - 0x40]
00E8:    DE C2                   faddp    st(2)
00EA:    DC 46 18                fadd     qword ptr [esi + 0x18]
00ED:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00F0:    DC 46 20                fadd     qword ptr [esi + 0x20]
00F3:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
00F6:    DC 46 28                fadd     qword ptr [esi + 0x28]
00F9:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
00FC:    DD D8                   fstp     st(0)
00FE:    DD 47 D8                fld      qword ptr [edi - 0x28]
0101:    DD 03                   fld      qword ptr [ebx]
0103:    D8 C9                   fmul     st(1)
0105:    DD 02                   fld      qword ptr [edx]
0107:    D8 CA                   fmul     st(2)
0109:    DD 01                   fld      qword ptr [ecx]
010B:    D8 CB                   fmul     st(3)
010D:    DC 45 C0                fadd     qword ptr [ebp - 0x40]
0110:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0113:    DC 45 C8                fadd     qword ptr [ebp - 0x38]
0116:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0119:    DC 45 D0                fadd     qword ptr [ebp - 0x30]
011C:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
011F:    DD D8                   fstp     st(0)
0121:    66 0F 10 03             movupd   xmm0, xmmword ptr [ebx]
0125:    66 0F 10 09             movupd   xmm1, xmmword ptr [ecx]
0129:    DD 45 F0                fld      qword ptr [ebp - 0x10]
012C:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
012F:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0132:    DC 45 D0                fadd     qword ptr [ebp - 0x30]
0135:    DD 1B                   fstp     qword ptr [ebx]
0137:    DD 45 F8                fld      qword ptr [ebp - 8]
013A:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
013D:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0140:    DC 45 C8                fadd     qword ptr [ebp - 0x38]
0143:    DD 1A                   fstp     qword ptr [edx]
0145:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0148:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
014B:    DC 4F E8                fmul     qword ptr [edi - 0x18]
014E:    DC 45 C0                fadd     qword ptr [ebp - 0x40]
0151:    DD 19                   fstp     qword ptr [ecx]
0153:    D9 D0                   fnop
0155:    8B C3                   mov      eax, ebx
0157:    5F                      pop      edi
0158:    5E                      pop      esi
0159:    5B                      pop      ebx
015A:    89 EC                   mov      esp, ebp
015C:    5D                      pop      ebp
015D:    C2 08 00                ret      8
