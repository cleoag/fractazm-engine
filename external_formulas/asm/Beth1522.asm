; Disassembly: Beth1522.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 298 bytes, decoded: 298 bytes (100.0%)

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
0045:    DD 03                   fld      qword ptr [ebx]
0047:    DC 08                   fmul     qword ptr [eax]
0049:    DD 43 08                fld      qword ptr [ebx + 8]
004C:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
004F:    DD 43 10                fld      qword ptr [ebx + 0x10]
0052:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0055:    DE C1                   faddp    st(1)
0057:    DD 43 20                fld      qword ptr [ebx + 0x20]
005A:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
005D:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0060:    DD 43 10                fld      qword ptr [ebx + 0x10]
0063:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0066:    DD 43 08                fld      qword ptr [ebx + 8]
0069:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
006C:    DE C1                   faddp    st(1)
006E:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0071:    DE E9                   fsubp    st(1)
0073:    DC 45 F8                fadd     qword ptr [ebp - 8]
0076:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
0079:    DE E9                   fsubp    st(1)
007B:    DE C9                   fmulp    st(1)
007D:    8B FE                   mov      edi, esi
007F:    83 C7 18                add      edi, 0x18
0082:    DC 2F                   fsubr    qword ptr [edi]
0084:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0087:    9B                      wait
0088:    DD 03                   fld      qword ptr [ebx]
008A:    DC 0A                   fmul     qword ptr [edx]
008C:    DD 43 08                fld      qword ptr [ebx + 8]
008F:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0092:    DD 43 10                fld      qword ptr [ebx + 0x10]
0095:    DC 4D F8                fmul     qword ptr [ebp - 8]
0098:    DE C1                   faddp    st(1)
009A:    DD 43 20                fld      qword ptr [ebx + 0x20]
009D:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00A0:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00A3:    DD 43 10                fld      qword ptr [ebx + 0x10]
00A6:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00A9:    DD 43 08                fld      qword ptr [ebx + 8]
00AC:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00AF:    DE C1                   faddp    st(1)
00B1:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00B4:    DE E9                   fsubp    st(1)
00B6:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
00B9:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
00BC:    DE E9                   fsubp    st(1)
00BE:    DE C9                   fmulp    st(1)
00C0:    8B FE                   mov      edi, esi
00C2:    83 C7 20                add      edi, 0x20
00C5:    DC 2F                   fsubr    qword ptr [edi]
00C7:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
00CA:    9B                      wait
00CB:    DD 03                   fld      qword ptr [ebx]
00CD:    DC 09                   fmul     qword ptr [ecx]
00CF:    DD 43 08                fld      qword ptr [ebx + 8]
00D2:    DC 4D F8                fmul     qword ptr [ebp - 8]
00D5:    DD 43 10                fld      qword ptr [ebx + 0x10]
00D8:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00DB:    DE C1                   faddp    st(1)
00DD:    DD 43 20                fld      qword ptr [ebx + 0x20]
00E0:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00E3:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00E6:    DD 43 10                fld      qword ptr [ebx + 0x10]
00E9:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00EC:    DD 43 08                fld      qword ptr [ebx + 8]
00EF:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00F2:    DE C1                   faddp    st(1)
00F4:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00F7:    DE E9                   fsubp    st(1)
00F9:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
00FC:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
00FF:    DE E9                   fsubp    st(1)
0101:    DE C9                   fmulp    st(1)
0103:    83 C6 28                add      esi, 0x28
0106:    DC 2E                   fsubr    qword ptr [esi]
0108:    DD 19                   fstp     qword ptr [ecx]
010A:    9B                      wait
010B:    8B 4D C8                mov      ecx, dword ptr [ebp - 0x38]
010E:    89 08                   mov      dword ptr [eax], ecx
0110:    8B 4D CC                mov      ecx, dword ptr [ebp - 0x34]
0113:    89 48 04                mov      dword ptr [eax + 4], ecx
0116:    8B 45 C0                mov      eax, dword ptr [ebp - 0x40]
0119:    89 02                   mov      dword ptr [edx], eax
011B:    8B 45 C4                mov      eax, dword ptr [ebp - 0x3c]
011E:    89 42 04                mov      dword ptr [edx + 4], eax
0121:    5F                      pop      edi
0122:    5E                      pop      esi
0123:    5B                      pop      ebx
0124:    8B E5                   mov      esp, ebp
0126:    5D                      pop      ebp
0127:    C2 08 00                ret      8
