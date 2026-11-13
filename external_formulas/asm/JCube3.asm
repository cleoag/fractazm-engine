; Disassembly: JCube3.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 294 bytes, decoded: 294 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    89 C3                   mov      ebx, eax
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    B8 FF FF FF 7F          mov      eax, 0x7fffffff
0012:    21 41 04                and      dword ptr [ecx + 4], eax
0015:    21 42 04                and      dword ptr [edx + 4], eax
0018:    21 43 04                and      dword ptr [ebx + 4], eax
001B:    90                      nop
001C:    83 C4 B0                add      esp, -0x50
001F:    DD 46 B0                fld      qword ptr [esi - 0x50]
0022:    D9 E8                   fld1    ; Load 1.0
0024:    D8 E9                   fsubr    st(1)
0026:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0028:    D9 E0                   fchs    ; -ST(0)
002A:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
002E:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0031:    DD 14 24                fst      qword ptr [esp]
0034:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0037:    DD 54 24 08             fst      qword ptr [esp + 8]
003B:    D9 E8                   fld1    ; Load 1.0
003D:    DE F1                   fdivrp   st(1)
003F:    DD 54 24 10             fst      qword ptr [esp + 0x10]
0043:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0047:    D9 E8                   fld1    ; Load 1.0
0049:    DE C1                   faddp    st(1)
004B:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
004F:    90                      nop
0050:    DD 01                   fld      qword ptr [ecx]
0052:    DD 02                   fld      qword ptr [edx]
0054:    DD 03                   fld      qword ptr [ebx]
0056:    D8 D1                   fcom     st(1)
0058:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005C:    72 02                   jb       0x60    ; Conditional jump
005E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0060:    D8 D2                   fcom     st(2)
0062:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0064:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0066:    73 02                   jae      0x6a    ; Conditional jump
0068:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
006A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006C:    D8 D2                   fcom     st(2)
006E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0070:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0072:    72 02                   jb       0x76    ; Conditional jump
0074:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0076:    90                      nop
0077:    D9 C0                   fld      st(0)
0079:    DD 44 24 10             fld      qword ptr [esp + 0x10]
007D:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
007F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0081:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0083:    76 52                   jbe      0xd7    ; Conditional jump
0085:    90                      nop
0086:    D9 C0                   fld      st(0)
0088:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
008C:    D9 C2                   fld      st(2)
008E:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0090:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0092:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0094:    76 41                   jbe      0xd7    ; Conditional jump
0096:    90                      nop
0097:    DD 46 E0                fld      qword ptr [esi - 0x20]
009A:    DC E9                   fsub     st(1), st(0)
009C:    DD D8                   fstp     st(0)
009E:    DD 46 D8                fld      qword ptr [esi - 0x28]
00A1:    DC EA                   fsub     st(2), st(0)
00A3:    DD D8                   fstp     st(0)
00A5:    DD 46 D0                fld      qword ptr [esi - 0x30]
00A8:    DC EB                   fsub     st(3), st(0)
00AA:    DD D8                   fstp     st(0)
00AC:    DD 44 24 08             fld      qword ptr [esp + 8]
00B0:    DC CB                   fmul     st(3), st(0)
00B2:    DC CA                   fmul     st(2), st(0)
00B4:    DC C9                   fmul     st(1), st(0)
00B6:    DD 41 08                fld      qword ptr [ecx + 8]
00B9:    DE C9                   fmulp    st(1)
00BB:    DD 59 08                fstp     qword ptr [ecx + 8]
00BE:    DD 46 E0                fld      qword ptr [esi - 0x20]
00C1:    DC C1                   fadd     st(1), st(0)
00C3:    DD D8                   fstp     st(0)
00C5:    DD 46 D8                fld      qword ptr [esi - 0x28]
00C8:    DC C2                   fadd     st(2), st(0)
00CA:    DD D8                   fstp     st(0)
00CC:    DD 46 D0                fld      qword ptr [esi - 0x30]
00CF:    DC C3                   fadd     st(3), st(0)
00D1:    DD D8                   fstp     st(0)
00D3:    90                      nop
00D4:    EB 3E                   jmp      0x114
00D6:    90                      nop
00D7:    DD 46 C8                fld      qword ptr [esi - 0x38]
00DA:    DC E9                   fsub     st(1), st(0)
00DC:    DD D8                   fstp     st(0)
00DE:    DD 46 C0                fld      qword ptr [esi - 0x40]
00E1:    DC EA                   fsub     st(2), st(0)
00E3:    DD D8                   fstp     st(0)
00E5:    DD 46 B8                fld      qword ptr [esi - 0x48]
00E8:    DC EB                   fsub     st(3), st(0)
00EA:    DD D8                   fstp     st(0)
00EC:    DD 04 24                fld      qword ptr [esp]
00EF:    90                      nop
00F0:    DC CB                   fmul     st(3), st(0)
00F2:    DC CA                   fmul     st(2), st(0)
00F4:    DC C9                   fmul     st(1), st(0)
00F6:    DD 41 08                fld      qword ptr [ecx + 8]
00F9:    DE C9                   fmulp    st(1)
00FB:    DD 59 08                fstp     qword ptr [ecx + 8]
00FE:    DD 46 C8                fld      qword ptr [esi - 0x38]
0101:    DC C1                   fadd     st(1), st(0)
0103:    DD D8                   fstp     st(0)
0105:    DD 46 C0                fld      qword ptr [esi - 0x40]
0108:    DC C2                   fadd     st(2), st(0)
010A:    DD D8                   fstp     st(0)
010C:    DD 46 B8                fld      qword ptr [esi - 0x48]
010F:    DC C3                   fadd     st(3), st(0)
0111:    DD D8                   fstp     st(0)
0113:    90                      nop
0114:    90                      nop
0115:    DD 1B                   fstp     qword ptr [ebx]
0117:    DD 1A                   fstp     qword ptr [edx]
0119:    DD 19                   fstp     qword ptr [ecx]
011B:    89 D8                   mov      eax, ebx
011D:    83 C4 50                add      esp, 0x50
0120:    5E                      pop      esi
0121:    5B                      pop      ebx
0122:    5D                      pop      ebp
0123:    C2 08 00                ret      8
