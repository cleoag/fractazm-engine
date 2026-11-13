; Disassembly: _BT1_4D_clampXYZ.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 310 bytes, decoded: 310 bytes (100.0%)
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
0018:    D8 C8                   fmul     st(0)
001A:    DD 02                   fld      qword ptr [edx]
001C:    D8 C8                   fmul     st(0)
001E:    DD 01                   fld      qword ptr [ecx]
0020:    D8 C8                   fmul     st(0)
0022:    DE C1                   faddp    st(1)
0024:    DE C1                   faddp    st(1)
0026:    D9 FA                   fsqrt    ; sqrt(ST(0))
0028:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
002B:    DD 03                   fld      qword ptr [ebx]
002D:    DC 4F 20                fmul     qword ptr [edi + 0x20]
0030:    DD 41 08                fld      qword ptr [ecx + 8]
0033:    DC 4F 18                fmul     qword ptr [edi + 0x18]
0036:    DE C1                   faddp    st(1)
0038:    D9 E1                   fabs    ; |ST(0)|
003A:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
003D:    90                      nop
003E:    DD 03                   fld      qword ptr [ebx]
0040:    DC 4F 18                fmul     qword ptr [edi + 0x18]
0043:    DD 41 08                fld      qword ptr [ecx + 8]
0046:    DC 4F 20                fmul     qword ptr [edi + 0x20]
0049:    D9 E0                   fchs    ; -ST(0)
004B:    DE C1                   faddp    st(1)
004D:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0050:    DC 0F                   fmul     qword ptr [edi]
0052:    DD 01                   fld      qword ptr [ecx]
0054:    DC 4F 08                fmul     qword ptr [edi + 8]
0057:    D9 E0                   fchs    ; -ST(0)
0059:    DE C1                   faddp    st(1)
005B:    DC 4F 10                fmul     qword ptr [edi + 0x10]
005E:    DD 5D F8                fstp     qword ptr [ebp - 8]
0061:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0064:    DC 4F 08                fmul     qword ptr [edi + 8]
0067:    DD 01                   fld      qword ptr [ecx]
0069:    DC 0F                   fmul     qword ptr [edi]
006B:    DE C1                   faddp    st(1)
006D:    D9 E1                   fabs    ; |ST(0)|
006F:    90                      nop
0070:    DD 45 F8                fld      qword ptr [ebp - 8]
0073:    DD 02                   fld      qword ptr [edx]
0075:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0078:    D9 E0                   fchs    ; -ST(0)
007A:    DE C1                   faddp    st(1)
007C:    D9 E1                   fabs    ; |ST(0)|
007E:    90                      nop
007F:    DD 45 F8                fld      qword ptr [ebp - 8]
0082:    DD 02                   fld      qword ptr [edx]
0084:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0087:    DE C1                   faddp    st(1)
0089:    D9 E1                   fabs    ; |ST(0)|
008B:    90                      nop
008C:    DD 1A                   fstp     qword ptr [edx]
008E:    DD 1B                   fstp     qword ptr [ebx]
0090:    DD 19                   fstp     qword ptr [ecx]
0092:    90                      nop
0093:    DD 03                   fld      qword ptr [ebx]
0095:    DC 02                   fadd     qword ptr [edx]
0097:    DC 4F 10                fmul     qword ptr [edi + 0x10]
009A:    DD 5D F8                fstp     qword ptr [ebp - 8]
009D:    90                      nop
009E:    DD 03                   fld      qword ptr [ebx]
00A0:    D9 E0                   fchs    ; -ST(0)
00A2:    DC 02                   fadd     qword ptr [edx]
00A4:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00A7:    90                      nop
00A8:    DD 1A                   fstp     qword ptr [edx]
00AA:    D9 D0                   fnop
00AC:    90                      nop
00AD:    DD 45 F8                fld      qword ptr [ebp - 8]
00B0:    DC 0F                   fmul     qword ptr [edi]
00B2:    DD 01                   fld      qword ptr [ecx]
00B4:    DC 4F 08                fmul     qword ptr [edi + 8]
00B7:    DE C1                   faddp    st(1)
00B9:    DD 55 F0                fst      qword ptr [ebp - 0x10]
00BC:    90                      nop
00BD:    DD 45 F8                fld      qword ptr [ebp - 8]
00C0:    DC 4F 08                fmul     qword ptr [edi + 8]
00C3:    D9 E0                   fchs    ; -ST(0)
00C5:    DD 01                   fld      qword ptr [ecx]
00C7:    DC 0F                   fmul     qword ptr [edi]
00C9:    DE C1                   faddp    st(1)
00CB:    90                      nop
00CC:    DD 19                   fstp     qword ptr [ecx]
00CE:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00D1:    D9 E0                   fchs    ; -ST(0)
00D3:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00D6:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00D9:    DE C1                   faddp    st(1)
00DB:    DD 59 08                fstp     qword ptr [ecx + 8]
00DE:    D9 D0                   fnop
00E0:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00E3:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00E6:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00E9:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00EC:    DE C1                   faddp    st(1)
00EE:    DD 55 F8                fst      qword ptr [ebp - 8]
00F1:    D8 C8                   fmul     st(0)
00F3:    DD 02                   fld      qword ptr [edx]
00F5:    D8 C8                   fmul     st(0)
00F7:    DD 01                   fld      qword ptr [ecx]
00F9:    D8 C8                   fmul     st(0)
00FB:    DE C1                   faddp    st(1)
00FD:    DE C1                   faddp    st(1)
00FF:    D9 FA                   fsqrt    ; sqrt(ST(0))
0101:    DC 7D E0                fdivr    qword ptr [ebp - 0x20]
0104:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0107:    DC 4D F8                fmul     qword ptr [ebp - 8]
010A:    DC 4F F0                fmul     qword ptr [edi - 0x10]
010D:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0110:    DD 1B                   fstp     qword ptr [ebx]
0112:    90                      nop
0113:    DD 02                   fld      qword ptr [edx]
0115:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0118:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
011B:    DD 1A                   fstp     qword ptr [edx]
011D:    90                      nop
011E:    DD 01                   fld      qword ptr [ecx]
0120:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0123:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0126:    DD 19                   fstp     qword ptr [ecx]
0128:    90                      nop
0129:    D9 D0                   fnop
012B:    8B C3                   mov      eax, ebx
012D:    5F                      pop      edi
012E:    5E                      pop      esi
012F:    5B                      pop      ebx
0130:    89 EC                   mov      esp, ebp
0132:    5D                      pop      ebp
0133:    C2 08 00                ret      8
