; Disassembly: _Fold45double.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 314 bytes, decoded: 314 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    57                      push     edi
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    8B D8                   mov      ebx, eax
000E:    D9 D0                   fnop
0010:    83 7F EC 00             cmp      dword ptr [edi - 0x14], 0
0014:    74 05                   je       0x1b    ; Conditional jump
0016:    E8 C2 00 00 00          call     0xdd
001B:    DD 03                   fld      qword ptr [ebx]
001D:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
0021:    74 02                   je       0x25    ; Conditional jump
0023:    D9 E0                   fchs    ; -ST(0)
0025:    DD 02                   fld      qword ptr [edx]
0027:    83 7F E4 00             cmp      dword ptr [edi - 0x1c], 0
002B:    74 02                   je       0x2f    ; Conditional jump
002D:    D9 E0                   fchs    ; -ST(0)
002F:    DE C1                   faddp    st(1)
0031:    DD 01                   fld      qword ptr [ecx]
0033:    83 7F E0 00             cmp      dword ptr [edi - 0x20], 0
0037:    74 02                   je       0x3b    ; Conditional jump
0039:    D9 E0                   fchs    ; -ST(0)
003B:    DE C1                   faddp    st(1)
003D:    DD 47 F0                fld      qword ptr [edi - 0x10]
0040:    D9 E1                   fabs    ; |ST(0)|
0042:    DE E9                   fsubp    st(1)
0044:    D9 C0                   fld      st(0)
0046:    D9 E4                   ftst
0048:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004A:    9E                      sahf
004B:    DD D8                   fstp     st(0)
004D:    72 37                   jb       0x86    ; Conditional jump
004F:    DC 0F                   fmul     qword ptr [edi]
0051:    DD 03                   fld      qword ptr [ebx]
0053:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
0057:    74 04                   je       0x5d    ; Conditional jump
0059:    D8 C1                   fadd     st(1)
005B:    EB 02                   jmp      0x5f
005D:    D8 E1                   fsub     st(1)
005F:    DD 1B                   fstp     qword ptr [ebx]
0061:    DD 02                   fld      qword ptr [edx]
0063:    83 7F E4 00             cmp      dword ptr [edi - 0x1c], 0
0067:    74 04                   je       0x6d    ; Conditional jump
0069:    D8 C1                   fadd     st(1)
006B:    EB 02                   jmp      0x6f
006D:    D8 E1                   fsub     st(1)
006F:    DD 1A                   fstp     qword ptr [edx]
0071:    DD 01                   fld      qword ptr [ecx]
0073:    83 7F E0 00             cmp      dword ptr [edi - 0x20], 0
0077:    74 04                   je       0x7d    ; Conditional jump
0079:    D8 C1                   fadd     st(1)
007B:    EB 02                   jmp      0x7f
007D:    D8 E1                   fsub     st(1)
007F:    DD 19                   fstp     qword ptr [ecx]
0081:    EB 4D                   jmp      0xd0
0083:    90                      nop
0084:    90                      nop
0085:    90                      nop
0086:    DD 47 F0                fld      qword ptr [edi - 0x10]
0089:    D9 E1                   fabs    ; |ST(0)|
008B:    DC C1                   fadd     st(1), st(0)
008D:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0090:    DE C1                   faddp    st(1)
0092:    D9 C0                   fld      st(0)
0094:    D9 E4                   ftst
0096:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0098:    9E                      sahf
0099:    DD D8                   fstp     st(0)
009B:    77 33                   ja       0xd0    ; Conditional jump
009D:    DC 0F                   fmul     qword ptr [edi]
009F:    DD 03                   fld      qword ptr [ebx]
00A1:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
00A5:    74 04                   je       0xab    ; Conditional jump
00A7:    D8 C1                   fadd     st(1)
00A9:    EB 02                   jmp      0xad
00AB:    D8 E1                   fsub     st(1)
00AD:    DD 1B                   fstp     qword ptr [ebx]
00AF:    DD 02                   fld      qword ptr [edx]
00B1:    83 7F E4 00             cmp      dword ptr [edi - 0x1c], 0
00B5:    74 04                   je       0xbb    ; Conditional jump
00B7:    D8 C1                   fadd     st(1)
00B9:    EB 02                   jmp      0xbd
00BB:    D8 E1                   fsub     st(1)
00BD:    DD 1A                   fstp     qword ptr [edx]
00BF:    DD 01                   fld      qword ptr [ecx]
00C1:    83 7F E0 00             cmp      dword ptr [edi - 0x20], 0
00C5:    74 04                   je       0xcb    ; Conditional jump
00C7:    D8 C1                   fadd     st(1)
00C9:    EB 02                   jmp      0xcd
00CB:    D8 E1                   fsub     st(1)
00CD:    DD 19                   fstp     qword ptr [ecx]
00CF:    90                      nop
00D0:    DD D8                   fstp     st(0)
00D2:    D9 D0                   fnop
00D4:    8B C3                   mov      eax, ebx
00D6:    5F                      pop      edi
00D7:    5E                      pop      esi
00D8:    5B                      pop      ebx
00D9:    5D                      pop      ebp
00DA:    C2 08 00                ret      8
00DD:    DD 03                   fld      qword ptr [ebx]
00DF:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
00E3:    75 02                   jne      0xe7    ; Conditional jump
00E5:    D9 E0                   fchs    ; -ST(0)
00E7:    DD 02                   fld      qword ptr [edx]
00E9:    83 7F E4 00             cmp      dword ptr [edi - 0x1c], 0
00ED:    75 02                   jne      0xf1    ; Conditional jump
00EF:    D9 E0                   fchs    ; -ST(0)
00F1:    DE C1                   faddp    st(1)
00F3:    DD 01                   fld      qword ptr [ecx]
00F5:    83 7F E0 00             cmp      dword ptr [edi - 0x20], 0
00F9:    75 02                   jne      0xfd    ; Conditional jump
00FB:    D9 E0                   fchs    ; -ST(0)
00FD:    DE C1                   faddp    st(1)
00FF:    DC 0F                   fmul     qword ptr [edi]
0101:    DD 03                   fld      qword ptr [ebx]
0103:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
0107:    75 04                   jne      0x10d    ; Conditional jump
0109:    D8 C1                   fadd     st(1)
010B:    EB 02                   jmp      0x10f
010D:    D8 E1                   fsub     st(1)
010F:    D9 E0                   fchs    ; -ST(0)
0111:    DD 1B                   fstp     qword ptr [ebx]
0113:    DD 02                   fld      qword ptr [edx]
0115:    83 7F E4 00             cmp      dword ptr [edi - 0x1c], 0
0119:    75 04                   jne      0x11f    ; Conditional jump
011B:    D8 C1                   fadd     st(1)
011D:    EB 02                   jmp      0x121
011F:    D8 E1                   fsub     st(1)
0121:    D9 E0                   fchs    ; -ST(0)
0123:    DD 1A                   fstp     qword ptr [edx]
0125:    DD 01                   fld      qword ptr [ecx]
0127:    83 7F E0 00             cmp      dword ptr [edi - 0x20], 0
012B:    75 04                   jne      0x131    ; Conditional jump
012D:    D8 C1                   fadd     st(1)
012F:    EB 02                   jmp      0x133
0131:    D8 E1                   fsub     st(1)
0133:    D9 E0                   fchs    ; -ST(0)
0135:    DD 19                   fstp     qword ptr [ecx]
0137:    DD D8                   fstp     st(0)
0139:    C3                      ret
