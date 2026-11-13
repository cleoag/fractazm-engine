; Disassembly: Beth1032.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 484 bytes, decoded: 484 bytes (100.0%)

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
004F:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0052:    DD 43 10                fld      qword ptr [ebx + 0x10]
0055:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0058:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
005B:    DE C1                   faddp    st(1)
005D:    DD 43 18                fld      qword ptr [ebx + 0x18]
0060:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0063:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0066:    DE E9                   fsubp    st(1)
0068:    DD 43 20                fld      qword ptr [ebx + 0x20]
006B:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
006E:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0071:    DE E9                   fsubp    st(1)
0073:    DD 43 58                fld      qword ptr [ebx + 0x58]
0076:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0079:    DD 43 38                fld      qword ptr [ebx + 0x38]
007C:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
007F:    DD 43 40                fld      qword ptr [ebx + 0x40]
0082:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0085:    DE C1                   faddp    st(1)
0087:    DD 43 60                fld      qword ptr [ebx + 0x60]
008A:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
008D:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0090:    DE E9                   fsubp    st(1)
0092:    DE C9                   fmulp    st(1)
0094:    DE E9                   fsubp    st(1)
0096:    DD 43 50                fld      qword ptr [ebx + 0x50]
0099:    DC 4D F8                fmul     qword ptr [ebp - 8]
009C:    DD 43 30                fld      qword ptr [ebx + 0x30]
009F:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00A2:    DD 43 28                fld      qword ptr [ebx + 0x28]
00A5:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00A8:    DE C1                   faddp    st(1)
00AA:    DE C9                   fmulp    st(1)
00AC:    DE E9                   fsubp    st(1)
00AE:    DD 43 48                fld      qword ptr [ebx + 0x48]
00B1:    DC 4D F8                fmul     qword ptr [ebp - 8]
00B4:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00B7:    DE C1                   faddp    st(1)
00B9:    DE C9                   fmulp    st(1)
00BB:    8B FE                   mov      edi, esi
00BD:    83 C7 18                add      edi, 0x18
00C0:    DC 07                   fadd     qword ptr [edi]
00C2:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
00C5:    9B                      wait
00C6:    DD 03                   fld      qword ptr [ebx]
00C8:    DC 0A                   fmul     qword ptr [edx]
00CA:    DD 43 08                fld      qword ptr [ebx + 8]
00CD:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00D0:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00D3:    DD 43 10                fld      qword ptr [ebx + 0x10]
00D6:    DC 4D F8                fmul     qword ptr [ebp - 8]
00D9:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00DC:    DE C1                   faddp    st(1)
00DE:    DD 43 18                fld      qword ptr [ebx + 0x18]
00E1:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00E4:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00E7:    DE E9                   fsubp    st(1)
00E9:    DD 43 20                fld      qword ptr [ebx + 0x20]
00EC:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00EF:    DC 4D F8                fmul     qword ptr [ebp - 8]
00F2:    DE E9                   fsubp    st(1)
00F4:    DD 43 58                fld      qword ptr [ebx + 0x58]
00F7:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00FA:    DD 43 38                fld      qword ptr [ebx + 0x38]
00FD:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0100:    DD 43 40                fld      qword ptr [ebx + 0x40]
0103:    DC 4D F8                fmul     qword ptr [ebp - 8]
0106:    DE C1                   faddp    st(1)
0108:    DD 43 60                fld      qword ptr [ebx + 0x60]
010B:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
010E:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0111:    DE E9                   fsubp    st(1)
0113:    DE C9                   fmulp    st(1)
0115:    DE E9                   fsubp    st(1)
0117:    DD 43 50                fld      qword ptr [ebx + 0x50]
011A:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
011D:    DD 43 30                fld      qword ptr [ebx + 0x30]
0120:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0123:    DD 43 28                fld      qword ptr [ebx + 0x28]
0126:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0129:    DE C1                   faddp    st(1)
012B:    DE C9                   fmulp    st(1)
012D:    DE E9                   fsubp    st(1)
012F:    DD 43 48                fld      qword ptr [ebx + 0x48]
0132:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0135:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0138:    DE C1                   faddp    st(1)
013A:    DE C9                   fmulp    st(1)
013C:    8B FE                   mov      edi, esi
013E:    83 C7 20                add      edi, 0x20
0141:    DC 07                   fadd     qword ptr [edi]
0143:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0146:    9B                      wait
0147:    DD 03                   fld      qword ptr [ebx]
0149:    DC 09                   fmul     qword ptr [ecx]
014B:    DD 43 08                fld      qword ptr [ebx + 8]
014E:    DC 4D F8                fmul     qword ptr [ebp - 8]
0151:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0154:    DD 43 10                fld      qword ptr [ebx + 0x10]
0157:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
015A:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
015D:    DE C1                   faddp    st(1)
015F:    DD 43 18                fld      qword ptr [ebx + 0x18]
0162:    DC 4D F8                fmul     qword ptr [ebp - 8]
0165:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0168:    DE E9                   fsubp    st(1)
016A:    DD 43 20                fld      qword ptr [ebx + 0x20]
016D:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0170:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0173:    DE E9                   fsubp    st(1)
0175:    DD 43 58                fld      qword ptr [ebx + 0x58]
0178:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
017B:    DD 43 38                fld      qword ptr [ebx + 0x38]
017E:    DC 4D F8                fmul     qword ptr [ebp - 8]
0181:    DD 43 40                fld      qword ptr [ebx + 0x40]
0184:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0187:    DE C1                   faddp    st(1)
0189:    DD 43 60                fld      qword ptr [ebx + 0x60]
018C:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
018F:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0192:    DE E9                   fsubp    st(1)
0194:    DE C9                   fmulp    st(1)
0196:    DE E9                   fsubp    st(1)
0198:    DD 43 50                fld      qword ptr [ebx + 0x50]
019B:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
019E:    DD 43 30                fld      qword ptr [ebx + 0x30]
01A1:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
01A4:    DD 43 28                fld      qword ptr [ebx + 0x28]
01A7:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
01AA:    DE C1                   faddp    st(1)
01AC:    DE C9                   fmulp    st(1)
01AE:    DE E9                   fsubp    st(1)
01B0:    DD 43 48                fld      qword ptr [ebx + 0x48]
01B3:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
01B6:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
01B9:    DE C1                   faddp    st(1)
01BB:    DE C9                   fmulp    st(1)
01BD:    83 C6 28                add      esi, 0x28
01C0:    DC 06                   fadd     qword ptr [esi]
01C2:    DD 19                   fstp     qword ptr [ecx]
01C4:    9B                      wait
01C5:    8B 4D C8                mov      ecx, dword ptr [ebp - 0x38]
01C8:    89 08                   mov      dword ptr [eax], ecx
01CA:    8B 4D CC                mov      ecx, dword ptr [ebp - 0x34]
01CD:    89 48 04                mov      dword ptr [eax + 4], ecx
01D0:    8B 45 C0                mov      eax, dword ptr [ebp - 0x40]
01D3:    89 02                   mov      dword ptr [edx], eax
01D5:    8B 45 C4                mov      eax, dword ptr [ebp - 0x3c]
01D8:    89 42 04                mov      dword ptr [edx + 4], eax
01DB:    5F                      pop      edi
01DC:    5E                      pop      esi
01DD:    5B                      pop      ebx
01DE:    8B E5                   mov      esp, ebp
01E0:    5D                      pop      ebp
01E1:    C2 08 00                ret      8
