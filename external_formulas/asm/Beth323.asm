; Disassembly: Beth323.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 262 bytes, decoded: 262 bytes (100.0%)

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
0018:    DD 55 F8                fst      qword ptr [ebp - 8]
001B:    D8 C8                   fmul     st(0)
001D:    DD 1B                   fstp     qword ptr [ebx]
001F:    DD 02                   fld      qword ptr [edx]
0021:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0024:    D8 C8                   fmul     st(0)
0026:    DD 1A                   fstp     qword ptr [edx]
0028:    DD 01                   fld      qword ptr [ecx]
002A:    DD 55 E8                fst      qword ptr [ebp - 0x18]
002D:    D8 C8                   fmul     st(0)
002F:    DD 19                   fstp     qword ptr [ecx]
0031:    D9 D0                   fnop
0033:    DD 02                   fld      qword ptr [edx]
0035:    DC 21                   fsub     qword ptr [ecx]
0037:    DC 0B                   fmul     qword ptr [ebx]
0039:    DC 0B                   fmul     qword ptr [ebx]
003B:    DC 0F                   fmul     qword ptr [edi]
003D:    DD 02                   fld      qword ptr [edx]
003F:    D8 C8                   fmul     st(0)
0041:    DD 55 E0                fst      qword ptr [ebp - 0x20]
0044:    DD 01                   fld      qword ptr [ecx]
0046:    D8 C8                   fmul     st(0)
0048:    DD 55 D8                fst      qword ptr [ebp - 0x28]
004B:    DE E9                   fsubp    st(1)
004D:    DC 0B                   fmul     qword ptr [ebx]
004F:    DC 4F 08                fmul     qword ptr [edi + 8]
0052:    DE C1                   faddp    st(1)
0054:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0057:    DC 0A                   fmul     qword ptr [edx]
0059:    DD 45 D8                fld      qword ptr [ebp - 0x28]
005C:    DC 09                   fmul     qword ptr [ecx]
005E:    DE E9                   fsubp    st(1)
0060:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0063:    DE C1                   faddp    st(1)
0065:    DC 4D F8                fmul     qword ptr [ebp - 8]
0068:    DC 46 18                fadd     qword ptr [esi + 0x18]
006B:    DD 5D F8                fstp     qword ptr [ebp - 8]
006E:    D9 D0                   fnop
0070:    DD 01                   fld      qword ptr [ecx]
0072:    DC 23                   fsub     qword ptr [ebx]
0074:    DC 0A                   fmul     qword ptr [edx]
0076:    DC 0A                   fmul     qword ptr [edx]
0078:    DC 0F                   fmul     qword ptr [edi]
007A:    DD 01                   fld      qword ptr [ecx]
007C:    D8 C8                   fmul     st(0)
007E:    DD 55 E0                fst      qword ptr [ebp - 0x20]
0081:    DD 03                   fld      qword ptr [ebx]
0083:    D8 C8                   fmul     st(0)
0085:    DD 55 D8                fst      qword ptr [ebp - 0x28]
0088:    DE E9                   fsubp    st(1)
008A:    DC 0A                   fmul     qword ptr [edx]
008C:    DC 4F 08                fmul     qword ptr [edi + 8]
008F:    DE C1                   faddp    st(1)
0091:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0094:    DC 09                   fmul     qword ptr [ecx]
0096:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0099:    DC 0B                   fmul     qword ptr [ebx]
009B:    DE E9                   fsubp    st(1)
009D:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00A0:    DE C1                   faddp    st(1)
00A2:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00A5:    DC 46 20                fadd     qword ptr [esi + 0x20]
00A8:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00AB:    D9 D0                   fnop
00AD:    DD 03                   fld      qword ptr [ebx]
00AF:    DC 22                   fsub     qword ptr [edx]
00B1:    DC 09                   fmul     qword ptr [ecx]
00B3:    DC 09                   fmul     qword ptr [ecx]
00B5:    DC 0F                   fmul     qword ptr [edi]
00B7:    DD 03                   fld      qword ptr [ebx]
00B9:    D8 C8                   fmul     st(0)
00BB:    DD 55 E0                fst      qword ptr [ebp - 0x20]
00BE:    DD 02                   fld      qword ptr [edx]
00C0:    D8 C8                   fmul     st(0)
00C2:    DD 55 D8                fst      qword ptr [ebp - 0x28]
00C5:    DE E9                   fsubp    st(1)
00C7:    DC 09                   fmul     qword ptr [ecx]
00C9:    DC 4F 08                fmul     qword ptr [edi + 8]
00CC:    DE C1                   faddp    st(1)
00CE:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00D1:    DC 0B                   fmul     qword ptr [ebx]
00D3:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00D6:    DC 0A                   fmul     qword ptr [edx]
00D8:    DE E9                   fsubp    st(1)
00DA:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00DD:    DE C1                   faddp    st(1)
00DF:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00E2:    DC 46 28                fadd     qword ptr [esi + 0x28]
00E5:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00E8:    D9 D0                   fnop
00EA:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00ED:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00F0:    DD 45 F8                fld      qword ptr [ebp - 8]
00F3:    DD 1B                   fstp     qword ptr [ebx]
00F5:    DD 1A                   fstp     qword ptr [edx]
00F7:    DD 19                   fstp     qword ptr [ecx]
00F9:    D9 D0                   fnop
00FB:    8B C3                   mov      eax, ebx
00FD:    5F                      pop      edi
00FE:    5E                      pop      esi
00FF:    5B                      pop      ebx
0100:    89 EC                   mov      esp, ebp
0102:    5D                      pop      ebp
0103:    C2 08 00                ret      8
