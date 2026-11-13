; Disassembly: _BenesiT2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 209 bytes, decoded: 209 bytes (100.0%)
; DEoption: -1

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
0016:    DD 03                   fld      qword ptr [ebx]
0018:    DC 0F                   fmul     qword ptr [edi]
001A:    DD 01                   fld      qword ptr [ecx]
001C:    DC 4F 08                fmul     qword ptr [edi + 8]
001F:    D9 E0                   fchs    ; -ST(0)
0021:    DE C1                   faddp    st(1)
0023:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0026:    DD 5D F8                fstp     qword ptr [ebp - 8]
0029:    DD 02                   fld      qword ptr [edx]
002B:    DC 4F 10                fmul     qword ptr [edi + 0x10]
002E:    DD 1A                   fstp     qword ptr [edx]
0030:    90                      nop
0031:    DD 03                   fld      qword ptr [ebx]
0033:    DC 4F 08                fmul     qword ptr [edi + 8]
0036:    DD 01                   fld      qword ptr [ecx]
0038:    DC 0F                   fmul     qword ptr [edi]
003A:    DE C1                   faddp    st(1)
003C:    90                      nop
003D:    DD 45 F8                fld      qword ptr [ebp - 8]
0040:    DC 22                   fsub     qword ptr [edx]
0042:    90                      nop
0043:    DD 45 F8                fld      qword ptr [ebp - 8]
0046:    DC 02                   fadd     qword ptr [edx]
0048:    90                      nop
0049:    D8 C8                   fmul     st(0)
004B:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
004E:    D8 C8                   fmul     st(0)
0050:    DD 5D F8                fstp     qword ptr [ebp - 8]
0053:    D8 C8                   fmul     st(0)
0055:    DD 19                   fstp     qword ptr [ecx]
0057:    D9 D0                   fnop
0059:    DD 45 F0                fld      qword ptr [ebp - 0x10]
005C:    DC 01                   fadd     qword ptr [ecx]
005E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0060:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0063:    D9 E1                   fabs    ; |ST(0)|
0065:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0068:    DC 4F 10                fmul     qword ptr [edi + 0x10]
006B:    DD 1B                   fstp     qword ptr [ebx]
006D:    90                      nop
006E:    DD 45 F8                fld      qword ptr [ebp - 8]
0071:    DC 01                   fadd     qword ptr [ecx]
0073:    D9 FA                   fsqrt    ; sqrt(ST(0))
0075:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0078:    D9 E1                   fabs    ; |ST(0)|
007A:    DC 4F F0                fmul     qword ptr [edi - 0x10]
007D:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0080:    DD 1A                   fstp     qword ptr [edx]
0082:    90                      nop
0083:    DD 45 F8                fld      qword ptr [ebp - 8]
0086:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0089:    D9 FA                   fsqrt    ; sqrt(ST(0))
008B:    DC 67 E8                fsub     qword ptr [edi - 0x18]
008E:    D9 E1                   fabs    ; |ST(0)|
0090:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0093:    DD 19                   fstp     qword ptr [ecx]
0095:    90                      nop
0096:    DD 02                   fld      qword ptr [edx]
0098:    DC 03                   fadd     qword ptr [ebx]
009A:    DD 02                   fld      qword ptr [edx]
009C:    DC 23                   fsub     qword ptr [ebx]
009E:    DD 1A                   fstp     qword ptr [edx]
00A0:    DD 5D F8                fstp     qword ptr [ebp - 8]
00A3:    90                      nop
00A4:    DD 45 F8                fld      qword ptr [ebp - 8]
00A7:    DC 0F                   fmul     qword ptr [edi]
00A9:    DD 01                   fld      qword ptr [ecx]
00AB:    DC 4F 08                fmul     qword ptr [edi + 8]
00AE:    DE C1                   faddp    st(1)
00B0:    DD 45 F8                fld      qword ptr [ebp - 8]
00B3:    DC 4F 08                fmul     qword ptr [edi + 8]
00B6:    D9 E0                   fchs    ; -ST(0)
00B8:    DD 01                   fld      qword ptr [ecx]
00BA:    DC 0F                   fmul     qword ptr [edi]
00BC:    90                      nop
00BD:    DE C1                   faddp    st(1)
00BF:    DD 19                   fstp     qword ptr [ecx]
00C1:    DD 1B                   fstp     qword ptr [ebx]
00C3:    90                      nop
00C4:    D9 D0                   fnop
00C6:    8B C3                   mov      eax, ebx
00C8:    5F                      pop      edi
00C9:    5E                      pop      esi
00CA:    5B                      pop      ebx
00CB:    89 EC                   mov      esp, ebp
00CD:    5D                      pop      ebp
00CE:    C2 08 00                ret      8
