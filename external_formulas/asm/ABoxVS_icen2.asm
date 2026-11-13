; Disassembly: ABoxVS_icen2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 351 bytes, decoded: 351 bytes (100.0%)
; DEoption: 11

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
000E:    81 C7 80 00 00 00       add      edi, 0x80
0014:    83 7F 50 00             cmp      dword ptr [edi + 0x50], 0
0018:    7F 09                   jg       0x23    ; Conditional jump
001A:    FF 47 50                inc      dword ptr [edi + 0x50]
001D:    DD 46 F0                fld      qword ptr [esi - 0x10]
0020:    DD 5F 48                fstp     qword ptr [edi + 0x48]
0023:    DD 47 48                fld      qword ptr [edi + 0x48]
0026:    D9 E8                   fld1    ; Load 1.0
0028:    D9 C1                   fld      st(1)
002A:    D9 E1                   fabs    ; |ST(0)|
002C:    DE E1                   fsubrp   st(1)
002E:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0031:    DE C1                   faddp    st(1)
0033:    DD 5F 48                fstp     qword ptr [edi + 0x48]
0036:    DD 46 D8                fld      qword ptr [esi - 0x28]
0039:    D9 C0                   fld      st(0)
003B:    D9 C0                   fld      st(0)
003D:    DC C2                   fadd     st(2), st(0)
003F:    D9 E0                   fchs    ; -ST(0)
0041:    DD 03                   fld      qword ptr [ebx]
0043:    D8 D2                   fcom     st(2)
0045:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0047:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0049:    72 06                   jb       0x51    ; Conditional jump
004B:    D9 E0                   fchs    ; -ST(0)
004D:    D8 C3                   fadd     st(3)
004F:    EB 0C                   jmp      0x5d
0051:    D8 D1                   fcom     st(1)
0053:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0055:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0057:    73 04                   jae      0x5d    ; Conditional jump
0059:    D9 E0                   fchs    ; -ST(0)
005B:    D8 E3                   fsub     st(3)
005D:    DD 02                   fld      qword ptr [edx]
005F:    D8 D3                   fcom     st(3)
0061:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0063:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0065:    72 06                   jb       0x6d    ; Conditional jump
0067:    D9 E0                   fchs    ; -ST(0)
0069:    D8 C4                   fadd     st(4)
006B:    EB 0C                   jmp      0x79
006D:    D8 D2                   fcom     st(2)
006F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0071:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0073:    73 04                   jae      0x79    ; Conditional jump
0075:    D9 E0                   fchs    ; -ST(0)
0077:    D8 E4                   fsub     st(4)
0079:    DD 01                   fld      qword ptr [ecx]
007B:    D8 D4                   fcom     st(4)
007D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
007F:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0081:    72 06                   jb       0x89    ; Conditional jump
0083:    D9 E0                   fchs    ; -ST(0)
0085:    D8 C5                   fadd     st(5)
0087:    EB 0C                   jmp      0x95
0089:    D8 D3                   fcom     st(3)
008B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
008D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
008F:    73 04                   jae      0x95    ; Conditional jump
0091:    D9 E0                   fchs    ; -ST(0)
0093:    D8 E5                   fsub     st(5)
0095:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0097:    DD D8                   fstp     st(0)
0099:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
009B:    DD D8                   fstp     st(0)
009D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
009F:    DD D8                   fstp     st(0)
00A1:    83 7E AC 00             cmp      dword ptr [esi - 0x54], 0
00A5:    75 17                   jne      0xbe    ; Conditional jump
00A7:    D9 C0                   fld      st(0)
00A9:    DC 46 C0                fadd     qword ptr [esi - 0x40]
00AC:    D8 C9                   fmul     st(1)
00AE:    D9 C2                   fld      st(2)
00B0:    DC 46 B8                fadd     qword ptr [esi - 0x48]
00B3:    D8 CB                   fmul     st(3)
00B5:    DE C1                   faddp    st(1)
00B7:    D9 C3                   fld      st(3)
00B9:    DC 46 B0                fadd     qword ptr [esi - 0x50]
00BC:    EB 15                   jmp      0xd3
00BE:    D9 C0                   fld      st(0)
00C0:    DC 47 98                fadd     qword ptr [edi - 0x68]
00C3:    D8 C9                   fmul     st(1)
00C5:    D9 C2                   fld      st(2)
00C7:    DC 47 A0                fadd     qword ptr [edi - 0x60]
00CA:    D8 CB                   fmul     st(3)
00CC:    DE C1                   faddp    st(1)
00CE:    D9 C3                   fld      st(3)
00D0:    DC 46 A8                fadd     qword ptr [esi - 0x58]
00D3:    D8 CC                   fmul     st(4)
00D5:    DE C1                   faddp    st(1)
00D7:    DD 46 C8                fld      qword ptr [esi - 0x38]
00DA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00DC:    D9 ED                   fldln2    ; Load ln(2)
00DE:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E0:    D9 F1                   fyl2x
00E2:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E4:    DE C9                   fmulp    st(1)
00E6:    D9 EA                   fldl2e    ; Load log2(e)
00E8:    DE C9                   fmulp    st(1)
00EA:    D9 C0                   fld      st(0)
00EC:    D9 FC                   frndint    ; round(ST(0))
00EE:    DC E9                   fsub     st(1), st(0)
00F0:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00F2:    D9 F0                   f2xm1
00F4:    D9 E8                   fld1    ; Load 1.0
00F6:    DE C1                   faddp    st(1)
00F8:    D9 FD                   fscale
00FA:    DD D9                   fstp     st(1)
00FC:    DC 56 E0                fcom     qword ptr [esi - 0x20]
00FF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0101:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0103:    73 07                   jae      0x10c    ; Conditional jump
0105:    DD D8                   fstp     st(0)
0107:    DD 46 E8                fld      qword ptr [esi - 0x18]
010A:    EB 16                   jmp      0x122
010C:    D9 E8                   fld1    ; Load 1.0
010E:    D8 D1                   fcom     st(1)
0110:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0112:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0114:    72 07                   jb       0x11d    ; Conditional jump
0116:    DD D8                   fstp     st(0)
0118:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
011B:    EB 05                   jmp      0x122
011D:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
011F:    DD 47 48                fld      qword ptr [edi + 0x48]
0122:    DD 41 08                fld      qword ptr [ecx + 8]
0125:    D8 C9                   fmul     st(1)
0127:    DD 59 08                fstp     qword ptr [ecx + 8]
012A:    DC CB                   fmul     st(3), st(0)
012C:    DC CA                   fmul     st(2), st(0)
012E:    DE C9                   fmulp    st(1)
0130:    83 7E AC 00             cmp      dword ptr [esi - 0x54], 0
0134:    75 11                   jne      0x147    ; Conditional jump
0136:    DC 47 A8                fadd     qword ptr [edi - 0x58]
0139:    DD 19                   fstp     qword ptr [ecx]
013B:    DC 47 A0                fadd     qword ptr [edi - 0x60]
013E:    DD 1A                   fstp     qword ptr [edx]
0140:    DC 47 98                fadd     qword ptr [edi - 0x68]
0143:    DD 1B                   fstp     qword ptr [ebx]
0145:    EB 0F                   jmp      0x156
0147:    DC 46 B0                fadd     qword ptr [esi - 0x50]
014A:    DD 19                   fstp     qword ptr [ecx]
014C:    DC 46 B8                fadd     qword ptr [esi - 0x48]
014F:    DD 1A                   fstp     qword ptr [edx]
0151:    DC 46 C0                fadd     qword ptr [esi - 0x40]
0154:    DD 1B                   fstp     qword ptr [ebx]
0156:    89 D8                   mov      eax, ebx
0158:    5B                      pop      ebx
0159:    5F                      pop      edi
015A:    5E                      pop      esi
015B:    5D                      pop      ebp
015C:    C2 08 00                ret      8
