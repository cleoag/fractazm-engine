; Disassembly: Makin4D p8a.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 441 bytes, decoded: 441 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    57                      push     edi
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    8B D8                   mov      ebx, eax
000E:    DD 03                   fld      qword ptr [ebx]
0010:    DC 09                   fmul     qword ptr [ecx]
0012:    DD 02                   fld      qword ptr [edx]
0014:    DC 49 08                fmul     qword ptr [ecx + 8]
0017:    DE E9                   fsubp    st(1)
0019:    D8 C0                   fadd     st(0)
001B:    90                      nop
001C:    DD 03                   fld      qword ptr [ebx]
001E:    DC 49 08                fmul     qword ptr [ecx + 8]
0021:    DD 02                   fld      qword ptr [edx]
0023:    DC 09                   fmul     qword ptr [ecx]
0025:    DE C1                   faddp    st(1)
0027:    D8 C0                   fadd     st(0)
0029:    90                      nop
002A:    DD 03                   fld      qword ptr [ebx]
002C:    D8 C8                   fmul     st(0)
002E:    DD 02                   fld      qword ptr [edx]
0030:    D8 C8                   fmul     st(0)
0032:    DE C1                   faddp    st(1)
0034:    DD 01                   fld      qword ptr [ecx]
0036:    D8 C8                   fmul     st(0)
0038:    DD 41 08                fld      qword ptr [ecx + 8]
003B:    D8 C8                   fmul     st(0)
003D:    DE C1                   faddp    st(1)
003F:    D8 D1                   fcom     st(1)
0041:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0043:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0045:    73 25                   jae      0x6c    ; Conditional jump
0047:    DE F1                   fdivrp   st(1)
0049:    D9 E0                   fchs    ; -ST(0)
004B:    D9 E8                   fld1    ; Load 1.0
004D:    DE C1                   faddp    st(1)
004F:    DD 03                   fld      qword ptr [ebx]
0051:    D8 C8                   fmul     st(0)
0053:    DD 02                   fld      qword ptr [edx]
0055:    D8 C8                   fmul     st(0)
0057:    DE E9                   fsubp    st(1)
0059:    D8 C9                   fmul     st(1)
005B:    90                      nop
005C:    DD 03                   fld      qword ptr [ebx]
005E:    DC 0A                   fmul     qword ptr [edx]
0060:    D8 C0                   fadd     st(0)
0062:    D8 CA                   fmul     st(2)
0064:    DD 1A                   fstp     qword ptr [edx]
0066:    DD 1B                   fstp     qword ptr [ebx]
0068:    DD D8                   fstp     st(0)
006A:    EB 1C                   jmp      0x88
006C:    DE F9                   fdivp    st(1)
006E:    D9 E8                   fld1    ; Load 1.0
0070:    D9 E0                   fchs    ; -ST(0)
0072:    DE C1                   faddp    st(1)
0074:    DD 01                   fld      qword ptr [ecx]
0076:    D8 C8                   fmul     st(0)
0078:    DD 41 08                fld      qword ptr [ecx + 8]
007B:    D8 C8                   fmul     st(0)
007D:    DE E9                   fsubp    st(1)
007F:    D8 C9                   fmul     st(1)
0081:    DD 01                   fld      qword ptr [ecx]
0083:    DC 49 08                fmul     qword ptr [ecx + 8]
0086:    EB D8                   jmp      0x60
0088:    DD 59 08                fstp     qword ptr [ecx + 8]
008B:    DD 19                   fstp     qword ptr [ecx]
008D:    DD 03                   fld      qword ptr [ebx]
008F:    DC 09                   fmul     qword ptr [ecx]
0091:    DD 02                   fld      qword ptr [edx]
0093:    DC 49 08                fmul     qword ptr [ecx + 8]
0096:    DE E9                   fsubp    st(1)
0098:    D8 C0                   fadd     st(0)
009A:    90                      nop
009B:    DD 03                   fld      qword ptr [ebx]
009D:    DC 49 08                fmul     qword ptr [ecx + 8]
00A0:    DD 02                   fld      qword ptr [edx]
00A2:    DC 09                   fmul     qword ptr [ecx]
00A4:    DE C1                   faddp    st(1)
00A6:    D8 C0                   fadd     st(0)
00A8:    90                      nop
00A9:    DD 03                   fld      qword ptr [ebx]
00AB:    D8 C8                   fmul     st(0)
00AD:    DD 02                   fld      qword ptr [edx]
00AF:    D8 C8                   fmul     st(0)
00B1:    DE C1                   faddp    st(1)
00B3:    DD 01                   fld      qword ptr [ecx]
00B5:    D8 C8                   fmul     st(0)
00B7:    DD 41 08                fld      qword ptr [ecx + 8]
00BA:    D8 C8                   fmul     st(0)
00BC:    DE C1                   faddp    st(1)
00BE:    D8 D1                   fcom     st(1)
00C0:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C2:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00C4:    73 25                   jae      0xeb    ; Conditional jump
00C6:    DE F1                   fdivrp   st(1)
00C8:    D9 E0                   fchs    ; -ST(0)
00CA:    D9 E8                   fld1    ; Load 1.0
00CC:    DE C1                   faddp    st(1)
00CE:    DD 03                   fld      qword ptr [ebx]
00D0:    D8 C8                   fmul     st(0)
00D2:    DD 02                   fld      qword ptr [edx]
00D4:    D8 C8                   fmul     st(0)
00D6:    DE E9                   fsubp    st(1)
00D8:    D8 C9                   fmul     st(1)
00DA:    90                      nop
00DB:    DD 03                   fld      qword ptr [ebx]
00DD:    DC 0A                   fmul     qword ptr [edx]
00DF:    D8 C0                   fadd     st(0)
00E1:    D8 CA                   fmul     st(2)
00E3:    DD 1A                   fstp     qword ptr [edx]
00E5:    DD 1B                   fstp     qword ptr [ebx]
00E7:    DD D8                   fstp     st(0)
00E9:    EB 1C                   jmp      0x107
00EB:    DE F9                   fdivp    st(1)
00ED:    D9 E8                   fld1    ; Load 1.0
00EF:    D9 E0                   fchs    ; -ST(0)
00F1:    DE C1                   faddp    st(1)
00F3:    DD 01                   fld      qword ptr [ecx]
00F5:    D8 C8                   fmul     st(0)
00F7:    DD 41 08                fld      qword ptr [ecx + 8]
00FA:    D8 C8                   fmul     st(0)
00FC:    DE E9                   fsubp    st(1)
00FE:    D8 C9                   fmul     st(1)
0100:    DD 01                   fld      qword ptr [ecx]
0102:    DC 49 08                fmul     qword ptr [ecx + 8]
0105:    EB D8                   jmp      0xdf
0107:    DD 59 08                fstp     qword ptr [ecx + 8]
010A:    DD 19                   fstp     qword ptr [ecx]
010C:    DD 03                   fld      qword ptr [ebx]
010E:    DC 09                   fmul     qword ptr [ecx]
0110:    DD 02                   fld      qword ptr [edx]
0112:    DC 49 08                fmul     qword ptr [ecx + 8]
0115:    DE E9                   fsubp    st(1)
0117:    D8 C0                   fadd     st(0)
0119:    90                      nop
011A:    DD 03                   fld      qword ptr [ebx]
011C:    DC 49 08                fmul     qword ptr [ecx + 8]
011F:    DD 02                   fld      qword ptr [edx]
0121:    DC 09                   fmul     qword ptr [ecx]
0123:    DE C1                   faddp    st(1)
0125:    D8 C0                   fadd     st(0)
0127:    90                      nop
0128:    DD 03                   fld      qword ptr [ebx]
012A:    D8 C8                   fmul     st(0)
012C:    DD 02                   fld      qword ptr [edx]
012E:    D8 C8                   fmul     st(0)
0130:    DE C1                   faddp    st(1)
0132:    DD 01                   fld      qword ptr [ecx]
0134:    D8 C8                   fmul     st(0)
0136:    DD 41 08                fld      qword ptr [ecx + 8]
0139:    D8 C8                   fmul     st(0)
013B:    DE C1                   faddp    st(1)
013D:    D8 D1                   fcom     st(1)
013F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0141:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0143:    73 25                   jae      0x16a    ; Conditional jump
0145:    DE F1                   fdivrp   st(1)
0147:    D9 E0                   fchs    ; -ST(0)
0149:    D9 E8                   fld1    ; Load 1.0
014B:    DE C1                   faddp    st(1)
014D:    DD 03                   fld      qword ptr [ebx]
014F:    D8 C8                   fmul     st(0)
0151:    DD 02                   fld      qword ptr [edx]
0153:    D8 C8                   fmul     st(0)
0155:    DE E9                   fsubp    st(1)
0157:    D8 C9                   fmul     st(1)
0159:    90                      nop
015A:    DD 03                   fld      qword ptr [ebx]
015C:    DC 0A                   fmul     qword ptr [edx]
015E:    D8 C0                   fadd     st(0)
0160:    D8 CA                   fmul     st(2)
0162:    DD 1A                   fstp     qword ptr [edx]
0164:    DD 1B                   fstp     qword ptr [ebx]
0166:    DD D8                   fstp     st(0)
0168:    EB 1C                   jmp      0x186
016A:    DE F9                   fdivp    st(1)
016C:    D9 E8                   fld1    ; Load 1.0
016E:    D9 E0                   fchs    ; -ST(0)
0170:    DE C1                   faddp    st(1)
0172:    DD 01                   fld      qword ptr [ecx]
0174:    D8 C8                   fmul     st(0)
0176:    DD 41 08                fld      qword ptr [ecx + 8]
0179:    D8 C8                   fmul     st(0)
017B:    DE E9                   fsubp    st(1)
017D:    D8 C9                   fmul     st(1)
017F:    DD 01                   fld      qword ptr [ecx]
0181:    DC 49 08                fmul     qword ptr [ecx + 8]
0184:    EB D8                   jmp      0x15e
0186:    DD 59 08                fstp     qword ptr [ecx + 8]
0189:    DD 19                   fstp     qword ptr [ecx]
018B:    DD 46 18                fld      qword ptr [esi + 0x18]
018E:    DC 03                   fadd     qword ptr [ebx]
0190:    DD 1B                   fstp     qword ptr [ebx]
0192:    DD 46 20                fld      qword ptr [esi + 0x20]
0195:    DC 02                   fadd     qword ptr [edx]
0197:    DD 1A                   fstp     qword ptr [edx]
0199:    DD 46 28                fld      qword ptr [esi + 0x28]
019C:    DC 01                   fadd     qword ptr [ecx]
019E:    DD 19                   fstp     qword ptr [ecx]
01A0:    DD 46 C8                fld      qword ptr [esi - 0x38]
01A3:    DC 47 F0                fadd     qword ptr [edi - 0x10]
01A6:    DC 41 08                fadd     qword ptr [ecx + 8]
01A9:    DD 59 08                fstp     qword ptr [ecx + 8]
01AC:    D9 D0                   fnop
01AE:    8B C3                   mov      eax, ebx
01B0:    5F                      pop      edi
01B1:    5E                      pop      esi
01B2:    5B                      pop      ebx
01B3:    89 EC                   mov      esp, ebp
01B5:    5D                      pop      ebp
01B6:    C2 08 00                ret      8
