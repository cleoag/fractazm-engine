; Disassembly: Beth1241.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 446 bytes, decoded: 446 bytes (100.0%)
; DEoption: 4

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
0013:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0016:    9B                      wait
0017:    DD 45 D8                fld      qword ptr [ebp - 0x28]
001A:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
001D:    DD 5D F8                fstp     qword ptr [ebp - 8]
0020:    9B                      wait
0021:    DD 02                   fld      qword ptr [edx]
0023:    DC 0A                   fmul     qword ptr [edx]
0025:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0028:    9B                      wait
0029:    DD 45 D0                fld      qword ptr [ebp - 0x30]
002C:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
002F:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0032:    9B                      wait
0033:    DD 01                   fld      qword ptr [ecx]
0035:    DC 09                   fmul     qword ptr [ecx]
0037:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
003A:    9B                      wait
003B:    DD 45 C8                fld      qword ptr [ebp - 0x38]
003E:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0041:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0044:    9B                      wait
0045:    8B 7D 0C                mov      edi, dword ptr [ebp + 0xc]
0048:    DD 07                   fld      qword ptr [edi]
004A:    8B 7D 0C                mov      edi, dword ptr [ebp + 0xc]
004D:    DC 0F                   fmul     qword ptr [edi]
004F:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0052:    9B                      wait
0053:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0056:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0059:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
005C:    9B                      wait
005D:    DD 03                   fld      qword ptr [ebx]
005F:    DC 08                   fmul     qword ptr [eax]
0061:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0064:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0067:    DC 45 E0                fadd     qword ptr [ebp - 0x20]
006A:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
006D:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0070:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0073:    DD 45 C8                fld      qword ptr [ebp - 0x38]
0076:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0079:    DE C1                   faddp    st(1)
007B:    DD 45 C0                fld      qword ptr [ebp - 0x40]
007E:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0081:    DE C1                   faddp    st(1)
0083:    DC 4B 20                fmul     qword ptr [ebx + 0x20]
0086:    DE E9                   fsubp    st(1)
0088:    DD 43 10                fld      qword ptr [ebx + 0x10]
008B:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
008E:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0091:    DC 45 C8                fadd     qword ptr [ebp - 0x38]
0094:    DC 45 C0                fadd     qword ptr [ebp - 0x40]
0097:    DE C9                   fmulp    st(1)
0099:    DE C1                   faddp    st(1)
009B:    DD 43 08                fld      qword ptr [ebx + 8]
009E:    DC 4D F8                fmul     qword ptr [ebp - 8]
00A1:    DE C1                   faddp    st(1)
00A3:    DE C9                   fmulp    st(1)
00A5:    8B FE                   mov      edi, esi
00A7:    83 C7 18                add      edi, 0x18
00AA:    DC 07                   fadd     qword ptr [edi]
00AC:    DD 18                   fstp     qword ptr [eax]
00AE:    9B                      wait
00AF:    DD 03                   fld      qword ptr [ebx]
00B1:    DC 0A                   fmul     qword ptr [edx]
00B3:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00B6:    DC 45 E0                fadd     qword ptr [ebp - 0x20]
00B9:    DC 45 F8                fadd     qword ptr [ebp - 8]
00BC:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
00BF:    DD 45 C8                fld      qword ptr [ebp - 0x38]
00C2:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
00C5:    DD 45 C0                fld      qword ptr [ebp - 0x40]
00C8:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00CB:    DE C1                   faddp    st(1)
00CD:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00D0:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
00D3:    DE C1                   faddp    st(1)
00D5:    DC 4B 20                fmul     qword ptr [ebx + 0x20]
00D8:    DE E9                   fsubp    st(1)
00DA:    DD 43 10                fld      qword ptr [ebx + 0x10]
00DD:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00E0:    DD 45 C8                fld      qword ptr [ebp - 0x38]
00E3:    DC 45 C0                fadd     qword ptr [ebp - 0x40]
00E6:    DC 45 D8                fadd     qword ptr [ebp - 0x28]
00E9:    DE C9                   fmulp    st(1)
00EB:    DE C1                   faddp    st(1)
00ED:    DD 43 08                fld      qword ptr [ebx + 8]
00F0:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00F3:    DE C1                   faddp    st(1)
00F5:    DE C9                   fmulp    st(1)
00F7:    8B C6                   mov      eax, esi
00F9:    8B F8                   mov      edi, eax
00FB:    83 C7 20                add      edi, 0x20
00FE:    DC 07                   fadd     qword ptr [edi]
0100:    DD 1A                   fstp     qword ptr [edx]
0102:    9B                      wait
0103:    DD 03                   fld      qword ptr [ebx]
0105:    DC 09                   fmul     qword ptr [ecx]
0107:    DD 45 E0                fld      qword ptr [ebp - 0x20]
010A:    DC 45 F8                fadd     qword ptr [ebp - 8]
010D:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0110:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
0113:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0116:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0119:    DD 45 D8                fld      qword ptr [ebp - 0x28]
011C:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
011F:    DE C1                   faddp    st(1)
0121:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0124:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0127:    DE C1                   faddp    st(1)
0129:    DC 4B 20                fmul     qword ptr [ebx + 0x20]
012C:    DE E9                   fsubp    st(1)
012E:    DD 43 10                fld      qword ptr [ebx + 0x10]
0131:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0134:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0137:    DC 45 D8                fadd     qword ptr [ebp - 0x28]
013A:    DC 45 D0                fadd     qword ptr [ebp - 0x30]
013D:    DE C9                   fmulp    st(1)
013F:    DE C1                   faddp    st(1)
0141:    DD 43 08                fld      qword ptr [ebx + 8]
0144:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0147:    DE C1                   faddp    st(1)
0149:    DE C9                   fmulp    st(1)
014B:    8B D0                   mov      edx, eax
014D:    83 C2 28                add      edx, 0x28
0150:    DC 02                   fadd     qword ptr [edx]
0152:    DD 19                   fstp     qword ptr [ecx]
0154:    9B                      wait
0155:    83 E8 38                sub      eax, 0x38
0158:    DD 00                   fld      qword ptr [eax]
015A:    8B 46 30                mov      eax, dword ptr [esi + 0x30]
015D:    83 E8 10                sub      eax, 0x10
0160:    DC 00                   fadd     qword ptr [eax]
0162:    DD 03                   fld      qword ptr [ebx]
0164:    8B 45 0C                mov      eax, dword ptr [ebp + 0xc]
0167:    DC 08                   fmul     qword ptr [eax]
0169:    DD 45 F8                fld      qword ptr [ebp - 8]
016C:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
016F:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0172:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
0175:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0178:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
017B:    DD 45 D0                fld      qword ptr [ebp - 0x30]
017E:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0181:    DE C1                   faddp    st(1)
0183:    DD 45 C8                fld      qword ptr [ebp - 0x38]
0186:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0189:    DE C1                   faddp    st(1)
018B:    DC 4B 20                fmul     qword ptr [ebx + 0x20]
018E:    DE E9                   fsubp    st(1)
0190:    DD 43 10                fld      qword ptr [ebx + 0x10]
0193:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0196:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0199:    DC 45 D0                fadd     qword ptr [ebp - 0x30]
019C:    DC 45 C8                fadd     qword ptr [ebp - 0x38]
019F:    DE C9                   fmulp    st(1)
01A1:    DE C1                   faddp    st(1)
01A3:    DD 43 08                fld      qword ptr [ebx + 8]
01A6:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
01A9:    DE C1                   faddp    st(1)
01AB:    DE C9                   fmulp    st(1)
01AD:    DE C1                   faddp    st(1)
01AF:    8B 45 0C                mov      eax, dword ptr [ebp + 0xc]
01B2:    DD 18                   fstp     qword ptr [eax]
01B4:    9B                      wait
01B5:    5F                      pop      edi
01B6:    5E                      pop      esi
01B7:    5B                      pop      ebx
01B8:    8B E5                   mov      esp, ebp
01BA:    5D                      pop      ebp
01BB:    C2 08 00                ret      8
