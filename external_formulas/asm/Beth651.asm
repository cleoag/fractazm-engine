; Disassembly: Beth651.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 319 bytes, decoded: 319 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    83 C4 C0                add      esp, -0x40
0006:    53                      push     ebx
0007:    56                      push     esi
0008:    57                      push     edi
0009:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000C:    8B 5E 30                mov      ebx, dword ptr [esi + 0x30]
000F:    DD 00                   fld      qword ptr [eax]
0011:    DC 08                   fmul     qword ptr [eax]
0013:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0016:    9B                      wait
0017:    DD 45 E0                fld      qword ptr [ebp - 0x20]
001A:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
001D:    DD 5D F8                fstp     qword ptr [ebp - 8]
0020:    9B                      wait
0021:    DD 02                   fld      qword ptr [edx]
0023:    DC 0A                   fmul     qword ptr [edx]
0025:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0028:    9B                      wait
0029:    DD 45 D8                fld      qword ptr [ebp - 0x28]
002C:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
002F:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0032:    9B                      wait
0033:    DD 01                   fld      qword ptr [ecx]
0035:    DC 09                   fmul     qword ptr [ecx]
0037:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
003A:    9B                      wait
003B:    DD 45 D0                fld      qword ptr [ebp - 0x30]
003E:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0041:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0044:    9B                      wait
0045:    DD 43 20                fld      qword ptr [ebx + 0x20]
0048:    DC 0A                   fmul     qword ptr [edx]
004A:    DC 09                   fmul     qword ptr [ecx]
004C:    DD 43 08                fld      qword ptr [ebx + 8]
004F:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0052:    DD 43 28                fld      qword ptr [ebx + 0x28]
0055:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0058:    DE C1                   faddp    st(1)
005A:    DD 43 30                fld      qword ptr [ebx + 0x30]
005D:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0060:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0063:    DD 43 38                fld      qword ptr [ebx + 0x38]
0066:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0069:    DD 03                   fld      qword ptr [ebx]
006B:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
006E:    DE C1                   faddp    st(1)
0070:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0073:    DE E9                   fsubp    st(1)
0075:    DD 43 18                fld      qword ptr [ebx + 0x18]
0078:    DC 4D F8                fmul     qword ptr [ebp - 8]
007B:    DE C1                   faddp    st(1)
007D:    DC 4B 10                fmul     qword ptr [ebx + 0x10]
0080:    DE E9                   fsubp    st(1)
0082:    DE C9                   fmulp    st(1)
0084:    8B FE                   mov      edi, esi
0086:    83 C7 18                add      edi, 0x18
0089:    DC 07                   fadd     qword ptr [edi]
008B:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
008E:    9B                      wait
008F:    DD 43 20                fld      qword ptr [ebx + 0x20]
0092:    DC 09                   fmul     qword ptr [ecx]
0094:    DC 08                   fmul     qword ptr [eax]
0096:    DD 43 08                fld      qword ptr [ebx + 8]
0099:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
009C:    DD 43 28                fld      qword ptr [ebx + 0x28]
009F:    DC 4D F8                fmul     qword ptr [ebp - 8]
00A2:    DE C1                   faddp    st(1)
00A4:    DD 43 30                fld      qword ptr [ebx + 0x30]
00A7:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00AA:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00AD:    DD 43 38                fld      qword ptr [ebx + 0x38]
00B0:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00B3:    DD 03                   fld      qword ptr [ebx]
00B5:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00B8:    DE C1                   faddp    st(1)
00BA:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00BD:    DE E9                   fsubp    st(1)
00BF:    DD 43 18                fld      qword ptr [ebx + 0x18]
00C2:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00C5:    DE C1                   faddp    st(1)
00C7:    DC 4B 10                fmul     qword ptr [ebx + 0x10]
00CA:    DE E9                   fsubp    st(1)
00CC:    DE C9                   fmulp    st(1)
00CE:    8B FE                   mov      edi, esi
00D0:    83 C7 20                add      edi, 0x20
00D3:    DC 07                   fadd     qword ptr [edi]
00D5:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
00D8:    9B                      wait
00D9:    DD 43 20                fld      qword ptr [ebx + 0x20]
00DC:    DC 08                   fmul     qword ptr [eax]
00DE:    DC 0A                   fmul     qword ptr [edx]
00E0:    DD 43 08                fld      qword ptr [ebx + 8]
00E3:    DC 4D F8                fmul     qword ptr [ebp - 8]
00E6:    DD 43 28                fld      qword ptr [ebx + 0x28]
00E9:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00EC:    DE C1                   faddp    st(1)
00EE:    DD 43 30                fld      qword ptr [ebx + 0x30]
00F1:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00F4:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00F7:    DD 43 38                fld      qword ptr [ebx + 0x38]
00FA:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00FD:    DD 03                   fld      qword ptr [ebx]
00FF:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0102:    DE C1                   faddp    st(1)
0104:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0107:    DE E9                   fsubp    st(1)
0109:    DD 43 18                fld      qword ptr [ebx + 0x18]
010C:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
010F:    DE C1                   faddp    st(1)
0111:    DC 4B 10                fmul     qword ptr [ebx + 0x10]
0114:    DE E9                   fsubp    st(1)
0116:    DE C9                   fmulp    st(1)
0118:    83 C6 28                add      esi, 0x28
011B:    DC 06                   fadd     qword ptr [esi]
011D:    DD 19                   fstp     qword ptr [ecx]
011F:    9B                      wait
0120:    8B 4D C8                mov      ecx, dword ptr [ebp - 0x38]
0123:    89 08                   mov      dword ptr [eax], ecx
0125:    8B 4D CC                mov      ecx, dword ptr [ebp - 0x34]
0128:    89 48 04                mov      dword ptr [eax + 4], ecx
012B:    8B 45 C0                mov      eax, dword ptr [ebp - 0x40]
012E:    89 02                   mov      dword ptr [edx], eax
0130:    8B 45 C4                mov      eax, dword ptr [ebp - 0x3c]
0133:    89 42 04                mov      dword ptr [edx + 4], eax
0136:    5F                      pop      edi
0137:    5E                      pop      esi
0138:    5B                      pop      ebx
0139:    8B E5                   mov      esp, ebp
013B:    5D                      pop      ebp
013C:    C2 08 00                ret      8
