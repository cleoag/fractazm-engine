; Disassembly: koch_surf.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 347 bytes, decoded: 347 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 20 00 00 00       sub      esp, 0x20
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    DD 02                   fld      qword ptr [edx]
0016:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0019:    DD 12                   fst      qword ptr [edx]
001B:    DD 03                   fld      qword ptr [ebx]
001D:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0020:    DD 13                   fst      qword ptr [ebx]
0022:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0024:    DD 47 E8                fld      qword ptr [edi - 0x18]
0027:    DC 4F 08                fmul     qword ptr [edi + 8]
002A:    DE C1                   faddp    st(1)
002C:    DC 37                   fdiv     qword ptr [edi]
002E:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0031:    DB 5D F8                fistp    dword ptr [ebp - 8]
0034:    DB 45 F8                fild     dword ptr [ebp - 8]
0037:    D9 E0                   fchs    ; -ST(0)
0039:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
003C:    DC 0F                   fmul     qword ptr [edi]
003E:    DD 47 E8                fld      qword ptr [edi - 0x18]
0041:    DC 4F 08                fmul     qword ptr [edi + 8]
0044:    DE C1                   faddp    st(1)
0046:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0048:    D9 D0                   fnop
004A:    DD 03                   fld      qword ptr [ebx]
004C:    D8 C9                   fmul     st(1)
004E:    DD 02                   fld      qword ptr [edx]
0050:    D8 CB                   fmul     st(3)
0052:    DE E1                   fsubrp   st(1)
0054:    D9 D0                   fnop
0056:    DD 03                   fld      qword ptr [ebx]
0058:    D8 CB                   fmul     st(3)
005A:    DD 02                   fld      qword ptr [edx]
005C:    D8 CB                   fmul     st(3)
005E:    DE C1                   faddp    st(1)
0060:    8B 47 80                mov      eax, dword ptr [edi - 0x80]
0063:    83 E0 02                and      eax, 2
0066:    74 05                   je       0x6d    ; Conditional jump
0068:    D9 E1                   fabs    ; |ST(0)|
006A:    EB 0B                   jmp      0x77
006C:    90                      nop
006D:    8B 45 F8                mov      eax, dword ptr [ebp - 8]
0070:    83 E0 01                and      eax, 1
0073:    75 02                   jne      0x77    ; Conditional jump
0075:    D9 E0                   fchs    ; -ST(0)
0077:    DC 67 D8                fsub     qword ptr [edi - 0x28]
007A:    DC 67 D0                fsub     qword ptr [edi - 0x30]
007D:    D9 E1                   fabs    ; |ST(0)|
007F:    D9 E0                   fchs    ; -ST(0)
0081:    DC 47 D0                fadd     qword ptr [edi - 0x30]
0084:    DD 1A                   fstp     qword ptr [edx]
0086:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0089:    DD 01                   fld      qword ptr [ecx]
008B:    DD 47 C8                fld      qword ptr [edi - 0x38]
008E:    DC C2                   fadd     st(2), st(0)
0090:    DD D8                   fstp     st(0)
0092:    D8 D1                   fcom     st(1)
0094:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0096:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0098:    90                      nop
0099:    76 02                   jbe      0x9d    ; Conditional jump
009B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
009D:    DD 47 C8                fld      qword ptr [edi - 0x38]
00A0:    DC EA                   fsub     st(2), st(0)
00A2:    8B 47 80                mov      eax, dword ptr [edi - 0x80]
00A5:    83 E0 01                and      eax, 1
00A8:    90                      nop
00A9:    90                      nop
00AA:    74 18                   je       0xc4    ; Conditional jump
00AC:    DC C2                   fadd     st(2), st(0)
00AE:    DD D8                   fstp     st(0)
00B0:    D8 D1                   fcom     st(1)
00B2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B4:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00B6:    90                      nop
00B7:    76 02                   jbe      0xbb    ; Conditional jump
00B9:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00BB:    DD 47 C8                fld      qword ptr [edi - 0x38]
00BE:    DC EA                   fsub     st(2), st(0)
00C0:    EB 16                   jmp      0xd8
00C2:    90                      nop
00C3:    90                      nop
00C4:    DC EA                   fsub     st(2), st(0)
00C6:    DD D8                   fstp     st(0)
00C8:    D8 D1                   fcom     st(1)
00CA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00CE:    90                      nop
00CF:    76 02                   jbe      0xd3    ; Conditional jump
00D1:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D3:    DD 47 C8                fld      qword ptr [edi - 0x38]
00D6:    DC C2                   fadd     st(2), st(0)
00D8:    DD D8                   fstp     st(0)
00DA:    90                      nop
00DB:    90                      nop
00DC:    DD 19                   fstp     qword ptr [ecx]
00DE:    DD 1B                   fstp     qword ptr [ebx]
00E0:    DD D8                   fstp     st(0)
00E2:    DD D8                   fstp     st(0)
00E4:    DD 01                   fld      qword ptr [ecx]
00E6:    DC 47 B8                fadd     qword ptr [edi - 0x48]
00E9:    DD 02                   fld      qword ptr [edx]
00EB:    DC 47 B0                fadd     qword ptr [edi - 0x50]
00EE:    DD 03                   fld      qword ptr [ebx]
00F0:    DC 47 A8                fadd     qword ptr [edi - 0x58]
00F3:    D9 C0                   fld      st(0)
00F5:    D8 4F 8C                fmul     dword ptr [edi - 0x74]
00F8:    D9 C2                   fld      st(2)
00FA:    D8 4F 88                fmul     dword ptr [edi - 0x78]
00FD:    DE C1                   faddp    st(1)
00FF:    D9 C3                   fld      st(3)
0101:    D8 4F 84                fmul     dword ptr [edi - 0x7c]
0104:    DE C1                   faddp    st(1)
0106:    D9 C1                   fld      st(1)
0108:    D8 4F A4                fmul     dword ptr [edi - 0x5c]
010B:    D9 C3                   fld      st(3)
010D:    D8 4F A0                fmul     dword ptr [edi - 0x60]
0110:    DE C1                   faddp    st(1)
0112:    D9 C4                   fld      st(4)
0114:    D8 4F 9C                fmul     dword ptr [edi - 0x64]
0117:    DE C1                   faddp    st(1)
0119:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
011B:    D8 4F 98                fmul     dword ptr [edi - 0x68]
011E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0120:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
0123:    DE C3                   faddp    st(3)
0125:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0127:    D8 4F 90                fmul     dword ptr [edi - 0x70]
012A:    DE C2                   faddp    st(2)
012C:    DD 47 C0                fld      qword ptr [edi - 0x40]
012F:    DD 41 08                fld      qword ptr [ecx + 8]
0132:    D8 C9                   fmul     st(1)
0134:    DD 59 08                fstp     qword ptr [ecx + 8]
0137:    DC CB                   fmul     st(3), st(0)
0139:    DC CA                   fmul     st(2), st(0)
013B:    DC C9                   fmul     st(1), st(0)
013D:    DD D8                   fstp     st(0)
013F:    90                      nop
0140:    DC 67 A8                fsub     qword ptr [edi - 0x58]
0143:    DD 1B                   fstp     qword ptr [ebx]
0145:    DC 67 B0                fsub     qword ptr [edi - 0x50]
0148:    DD 1A                   fstp     qword ptr [edx]
014A:    DC 67 B8                fsub     qword ptr [edi - 0x48]
014D:    DD 19                   fstp     qword ptr [ecx]
014F:    90                      nop
0150:    8B C3                   mov      eax, ebx
0152:    5F                      pop      edi
0153:    5E                      pop      esi
0154:    5B                      pop      ebx
0155:    89 EC                   mov      esp, ebp
0157:    5D                      pop      ebp
0158:    C2 08 00                ret      8
