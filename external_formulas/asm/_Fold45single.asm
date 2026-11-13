; Disassembly: _Fold45single.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 241 bytes, decoded: 241 bytes (100.0%)
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
0010:    83 7F DC 00             cmp      dword ptr [edi - 0x24], 0
0014:    74 05                   je       0x1b    ; Conditional jump
0016:    E8 79 00 00 00          call     0x94
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
0040:    DE E9                   fsubp    st(1)
0042:    D9 C0                   fld      st(0)
0044:    83 7F EC 00             cmp      dword ptr [edi - 0x14], 0
0048:    74 02                   je       0x4c    ; Conditional jump
004A:    D9 E0                   fchs    ; -ST(0)
004C:    D9 E4                   ftst
004E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0050:    9E                      sahf
0051:    DD D8                   fstp     st(0)
0053:    72 32                   jb       0x87    ; Conditional jump
0055:    DC 0F                   fmul     qword ptr [edi]
0057:    DD 03                   fld      qword ptr [ebx]
0059:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
005D:    74 04                   je       0x63    ; Conditional jump
005F:    D8 C1                   fadd     st(1)
0061:    EB 02                   jmp      0x65
0063:    D8 E1                   fsub     st(1)
0065:    DD 1B                   fstp     qword ptr [ebx]
0067:    DD 02                   fld      qword ptr [edx]
0069:    83 7F E4 00             cmp      dword ptr [edi - 0x1c], 0
006D:    74 04                   je       0x73    ; Conditional jump
006F:    D8 C1                   fadd     st(1)
0071:    EB 02                   jmp      0x75
0073:    D8 E1                   fsub     st(1)
0075:    DD 1A                   fstp     qword ptr [edx]
0077:    DD 01                   fld      qword ptr [ecx]
0079:    83 7F E0 00             cmp      dword ptr [edi - 0x20], 0
007D:    74 04                   je       0x83    ; Conditional jump
007F:    D8 C1                   fadd     st(1)
0081:    EB 02                   jmp      0x85
0083:    D8 E1                   fsub     st(1)
0085:    DD 19                   fstp     qword ptr [ecx]
0087:    DD D8                   fstp     st(0)
0089:    D9 D0                   fnop
008B:    8B C3                   mov      eax, ebx
008D:    5F                      pop      edi
008E:    5E                      pop      esi
008F:    5B                      pop      ebx
0090:    5D                      pop      ebp
0091:    C2 08 00                ret      8
0094:    DD 03                   fld      qword ptr [ebx]
0096:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
009A:    75 02                   jne      0x9e    ; Conditional jump
009C:    D9 E0                   fchs    ; -ST(0)
009E:    DD 02                   fld      qword ptr [edx]
00A0:    83 7F E4 00             cmp      dword ptr [edi - 0x1c], 0
00A4:    75 02                   jne      0xa8    ; Conditional jump
00A6:    D9 E0                   fchs    ; -ST(0)
00A8:    DE C1                   faddp    st(1)
00AA:    DD 01                   fld      qword ptr [ecx]
00AC:    83 7F E0 00             cmp      dword ptr [edi - 0x20], 0
00B0:    75 02                   jne      0xb4    ; Conditional jump
00B2:    D9 E0                   fchs    ; -ST(0)
00B4:    DE C1                   faddp    st(1)
00B6:    DC 0F                   fmul     qword ptr [edi]
00B8:    DD 03                   fld      qword ptr [ebx]
00BA:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
00BE:    75 04                   jne      0xc4    ; Conditional jump
00C0:    D8 C1                   fadd     st(1)
00C2:    EB 02                   jmp      0xc6
00C4:    D8 E1                   fsub     st(1)
00C6:    D9 E0                   fchs    ; -ST(0)
00C8:    DD 1B                   fstp     qword ptr [ebx]
00CA:    DD 02                   fld      qword ptr [edx]
00CC:    83 7F E4 00             cmp      dword ptr [edi - 0x1c], 0
00D0:    75 04                   jne      0xd6    ; Conditional jump
00D2:    D8 C1                   fadd     st(1)
00D4:    EB 02                   jmp      0xd8
00D6:    D8 E1                   fsub     st(1)
00D8:    D9 E0                   fchs    ; -ST(0)
00DA:    DD 1A                   fstp     qword ptr [edx]
00DC:    DD 01                   fld      qword ptr [ecx]
00DE:    83 7F E0 00             cmp      dword ptr [edi - 0x20], 0
00E2:    75 04                   jne      0xe8    ; Conditional jump
00E4:    D8 C1                   fadd     st(1)
00E6:    EB 02                   jmp      0xea
00E8:    D8 E1                   fsub     st(1)
00EA:    D9 E0                   fchs    ; -ST(0)
00EC:    DD 19                   fstp     qword ptr [ecx]
00EE:    DD D8                   fstp     st(0)
00F0:    C3                      ret
