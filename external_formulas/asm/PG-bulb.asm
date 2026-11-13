; Disassembly: PG-bulb.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 404 bytes, decoded: 404 bytes (100.0%)
; DEoption: 4

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
0016:    8B 47 EC                mov      eax, dword ptr [edi - 0x14]
0019:    83 F8 11                cmp      eax, 0x11
001C:    7C 02                   jl       0x20    ; Conditional jump
001E:    EB 05                   jmp      0x25
0020:    83 F8 01                cmp      eax, 1
0023:    7D 05                   jge      0x2a    ; Conditional jump
0025:    B8 02 00 00 00          mov      eax, 2
002A:    90                      nop
002B:    DD 03                   fld      qword ptr [ebx]
002D:    DD 5D F8                fstp     qword ptr [ebp - 8]
0030:    90                      nop
0031:    DD 02                   fld      qword ptr [edx]
0033:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0036:    90                      nop
0037:    DD 01                   fld      qword ptr [ecx]
0039:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
003C:    90                      nop
003D:    DD 41 08                fld      qword ptr [ecx + 8]
0040:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0043:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0046:    90                      nop
0047:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
004B:    75 28                   jne      0x75    ; Conditional jump
004D:    DD 03                   fld      qword ptr [ebx]
004F:    D8 C8                   fmul     st(0)
0051:    DD 02                   fld      qword ptr [edx]
0053:    D8 C8                   fmul     st(0)
0055:    DE C1                   faddp    st(1)
0057:    DD 03                   fld      qword ptr [ebx]
0059:    D8 C8                   fmul     st(0)
005B:    DD 02                   fld      qword ptr [edx]
005D:    D8 C8                   fmul     st(0)
005F:    DE E9                   fsubp    st(1)
0061:    D8 F1                   fdiv     st(1)
0063:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0066:    DD 03                   fld      qword ptr [ebx]
0068:    DC 0A                   fmul     qword ptr [edx]
006A:    D8 F1                   fdiv     st(1)
006C:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
006F:    DD D8                   fstp     st(0)
0071:    EB 2C                   jmp      0x9f
0073:    D9 D0                   fnop
0075:    DD 46 18                fld      qword ptr [esi + 0x18]
0078:    D8 C8                   fmul     st(0)
007A:    DD 46 20                fld      qword ptr [esi + 0x20]
007D:    D8 C8                   fmul     st(0)
007F:    DE C1                   faddp    st(1)
0081:    DD 46 18                fld      qword ptr [esi + 0x18]
0084:    D8 C8                   fmul     st(0)
0086:    DD 46 20                fld      qword ptr [esi + 0x20]
0089:    D8 C8                   fmul     st(0)
008B:    DE E9                   fsubp    st(1)
008D:    D8 F1                   fdiv     st(1)
008F:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0092:    DD 46 18                fld      qword ptr [esi + 0x18]
0095:    DC 4E 20                fmul     qword ptr [esi + 0x20]
0098:    D8 F1                   fdiv     st(1)
009A:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
009D:    DD D8                   fstp     st(0)
009F:    48                      dec      eax
00A0:    90                      nop
00A1:    48                      dec      eax
00A2:    DD 45 F8                fld      qword ptr [ebp - 8]
00A5:    DC 0B                   fmul     qword ptr [ebx]
00A7:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00AA:    DC 0A                   fmul     qword ptr [edx]
00AC:    DE E9                   fsubp    st(1)
00AE:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00B1:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00B4:    DC 09                   fmul     qword ptr [ecx]
00B6:    DE E9                   fsubp    st(1)
00B8:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00BB:    DC 49 08                fmul     qword ptr [ecx + 8]
00BE:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00C1:    DC 09                   fmul     qword ptr [ecx]
00C3:    DE C1                   faddp    st(1)
00C5:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00C8:    DE C1                   faddp    st(1)
00CA:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00CD:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00D0:    DC 49 08                fmul     qword ptr [ecx + 8]
00D3:    DE C1                   faddp    st(1)
00D5:    D9 D0                   fnop
00D7:    DD 45 F8                fld      qword ptr [ebp - 8]
00DA:    DC 0A                   fmul     qword ptr [edx]
00DC:    90                      nop
00DD:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00E0:    DC 0B                   fmul     qword ptr [ebx]
00E2:    DE C1                   faddp    st(1)
00E4:    DD 45 D0                fld      qword ptr [ebp - 0x30]
00E7:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00EA:    DC 09                   fmul     qword ptr [ecx]
00EC:    DE E9                   fsubp    st(1)
00EE:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00F1:    DC 49 08                fmul     qword ptr [ecx + 8]
00F4:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00F7:    DC 09                   fmul     qword ptr [ecx]
00F9:    DE C1                   faddp    st(1)
00FB:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00FE:    DE E9                   fsubp    st(1)
0100:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0103:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0106:    DC 49 08                fmul     qword ptr [ecx + 8]
0109:    DE C1                   faddp    st(1)
010B:    D9 D0                   fnop
010D:    DD 45 F8                fld      qword ptr [ebp - 8]
0110:    DC 09                   fmul     qword ptr [ecx]
0112:    90                      nop
0113:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0116:    DC 0B                   fmul     qword ptr [ebx]
0118:    DE C1                   faddp    st(1)
011A:    DD 45 F0                fld      qword ptr [ebp - 0x10]
011D:    DC 49 08                fmul     qword ptr [ecx + 8]
0120:    DE E9                   fsubp    st(1)
0122:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0125:    DC 0A                   fmul     qword ptr [edx]
0127:    90                      nop
0128:    DE E9                   fsubp    st(1)
012A:    D9 D0                   fnop
012C:    DD 45 F8                fld      qword ptr [ebp - 8]
012F:    DC 49 08                fmul     qword ptr [ecx + 8]
0132:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0135:    DC 0B                   fmul     qword ptr [ebx]
0137:    DE C1                   faddp    st(1)
0139:    DD 45 F0                fld      qword ptr [ebp - 0x10]
013C:    DC 09                   fmul     qword ptr [ecx]
013E:    90                      nop
013F:    DE C1                   faddp    st(1)
0141:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0144:    DC 0A                   fmul     qword ptr [edx]
0146:    90                      nop
0147:    DE C1                   faddp    st(1)
0149:    D9 D0                   fnop
014B:    83 F8 00                cmp      eax, 0
014E:    0F 8E 11 00 00 00       jle      0x165    ; Conditional jump
0154:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0157:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
015A:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
015D:    DD 5D F8                fstp     qword ptr [ebp - 8]
0160:    E9 3C FF FF FF          jmp      0xa1
0165:    D9 D0                   fnop
0167:    DC 46 C8                fadd     qword ptr [esi - 0x38]
016A:    DD 59 08                fstp     qword ptr [ecx + 8]
016D:    DC 46 28                fadd     qword ptr [esi + 0x28]
0170:    DD 19                   fstp     qword ptr [ecx]
0172:    DC 46 20                fadd     qword ptr [esi + 0x20]
0175:    DD 1A                   fstp     qword ptr [edx]
0177:    DC 46 18                fadd     qword ptr [esi + 0x18]
017A:    DD 1B                   fstp     qword ptr [ebx]
017C:    90                      nop
017D:    D9 D0                   fnop
017F:    8B C3                   mov      eax, ebx
0181:    5F                      pop      edi
0182:    5E                      pop      esi
0183:    5B                      pop      ebx
0184:    89 EC                   mov      esp, ebp
0186:    5D                      pop      ebp
0187:    C2 08 00                ret      8
018A:    B8 08 00 00 00          mov      eax, 8
018F:    E9 97 FE FF FF          jmp      0x2b
