; Disassembly: _BT1_4D_Transform1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 254 bytes, decoded: 254 bytes (100.0%)
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
0018:    DC 4F 20                fmul     qword ptr [edi + 0x20]
001B:    DD 41 08                fld      qword ptr [ecx + 8]
001E:    DC 4F 18                fmul     qword ptr [edi + 0x18]
0021:    DE C1                   faddp    st(1)
0023:    D9 E1                   fabs    ; |ST(0)|
0025:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0028:    90                      nop
0029:    DD 03                   fld      qword ptr [ebx]
002B:    DC 4F 18                fmul     qword ptr [edi + 0x18]
002E:    DD 41 08                fld      qword ptr [ecx + 8]
0031:    DC 4F 20                fmul     qword ptr [edi + 0x20]
0034:    D9 E0                   fchs    ; -ST(0)
0036:    DE C1                   faddp    st(1)
0038:    DD 55 F0                fst      qword ptr [ebp - 0x10]
003B:    DC 0F                   fmul     qword ptr [edi]
003D:    DD 01                   fld      qword ptr [ecx]
003F:    DC 4F 08                fmul     qword ptr [edi + 8]
0042:    D9 E0                   fchs    ; -ST(0)
0044:    DE C1                   faddp    st(1)
0046:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0049:    DD 5D F8                fstp     qword ptr [ebp - 8]
004C:    DD 45 F0                fld      qword ptr [ebp - 0x10]
004F:    DC 4F 08                fmul     qword ptr [edi + 8]
0052:    DD 01                   fld      qword ptr [ecx]
0054:    DC 0F                   fmul     qword ptr [edi]
0056:    DE C1                   faddp    st(1)
0058:    D9 E1                   fabs    ; |ST(0)|
005A:    90                      nop
005B:    DD 45 F8                fld      qword ptr [ebp - 8]
005E:    DD 02                   fld      qword ptr [edx]
0060:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0063:    D9 E0                   fchs    ; -ST(0)
0065:    DE C1                   faddp    st(1)
0067:    D9 E1                   fabs    ; |ST(0)|
0069:    90                      nop
006A:    DD 45 F8                fld      qword ptr [ebp - 8]
006D:    DD 02                   fld      qword ptr [edx]
006F:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0072:    DE C1                   faddp    st(1)
0074:    D9 E1                   fabs    ; |ST(0)|
0076:    90                      nop
0077:    DD 1A                   fstp     qword ptr [edx]
0079:    DD 1B                   fstp     qword ptr [ebx]
007B:    DD 19                   fstp     qword ptr [ecx]
007D:    90                      nop
007E:    DD 03                   fld      qword ptr [ebx]
0080:    DC 02                   fadd     qword ptr [edx]
0082:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0085:    DD 5D F8                fstp     qword ptr [ebp - 8]
0088:    90                      nop
0089:    DD 03                   fld      qword ptr [ebx]
008B:    D9 E0                   fchs    ; -ST(0)
008D:    DC 02                   fadd     qword ptr [edx]
008F:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0092:    DD 1A                   fstp     qword ptr [edx]
0094:    D9 D0                   fnop
0096:    90                      nop
0097:    DD 45 F8                fld      qword ptr [ebp - 8]
009A:    DC 0F                   fmul     qword ptr [edi]
009C:    DD 01                   fld      qword ptr [ecx]
009E:    DC 4F 08                fmul     qword ptr [edi + 8]
00A1:    DE C1                   faddp    st(1)
00A3:    DD 55 F0                fst      qword ptr [ebp - 0x10]
00A6:    90                      nop
00A7:    DD 45 F8                fld      qword ptr [ebp - 8]
00AA:    DC 4F 08                fmul     qword ptr [edi + 8]
00AD:    D9 E0                   fchs    ; -ST(0)
00AF:    DD 01                   fld      qword ptr [ecx]
00B1:    DC 0F                   fmul     qword ptr [edi]
00B3:    DE C1                   faddp    st(1)
00B5:    90                      nop
00B6:    DD 19                   fstp     qword ptr [ecx]
00B8:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00BB:    D9 E0                   fchs    ; -ST(0)
00BD:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00C0:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00C3:    DE C1                   faddp    st(1)
00C5:    DD 59 08                fstp     qword ptr [ecx + 8]
00C8:    D9 D0                   fnop
00CA:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00CD:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00D0:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00D3:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00D6:    DE C1                   faddp    st(1)
00D8:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00DB:    DC 67 E8                fsub     qword ptr [edi - 0x18]
00DE:    DD 1B                   fstp     qword ptr [ebx]
00E0:    90                      nop
00E1:    DD 02                   fld      qword ptr [edx]
00E3:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00E6:    DD 1A                   fstp     qword ptr [edx]
00E8:    90                      nop
00E9:    DD 01                   fld      qword ptr [ecx]
00EB:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00EE:    DD 19                   fstp     qword ptr [ecx]
00F0:    90                      nop
00F1:    D9 D0                   fnop
00F3:    8B C3                   mov      eax, ebx
00F5:    5F                      pop      edi
00F6:    5E                      pop      esi
00F7:    5B                      pop      ebx
00F8:    89 EC                   mov      esp, ebp
00FA:    5D                      pop      ebp
00FB:    C2 08 00                ret      8
