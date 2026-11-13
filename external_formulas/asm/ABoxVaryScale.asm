; Disassembly: ABoxVaryScale.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 290 bytes, decoded: 290 bytes (100.0%)
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
00A1:    D9 C0                   fld      st(0)
00A3:    D8 C9                   fmul     st(1)
00A5:    D9 C2                   fld      st(2)
00A7:    D8 CB                   fmul     st(3)
00A9:    DE C1                   faddp    st(1)
00AB:    D9 C3                   fld      st(3)
00AD:    D8 CC                   fmul     st(4)
00AF:    DE C1                   faddp    st(1)
00B1:    DD 46 C8                fld      qword ptr [esi - 0x38]
00B4:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00B6:    D9 ED                   fldln2    ; Load ln(2)
00B8:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00BA:    D9 F1                   fyl2x
00BC:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00BE:    DE C9                   fmulp    st(1)
00C0:    D9 EA                   fldl2e    ; Load log2(e)
00C2:    DE C9                   fmulp    st(1)
00C4:    D9 C0                   fld      st(0)
00C6:    D9 FC                   frndint    ; round(ST(0))
00C8:    DC E9                   fsub     st(1), st(0)
00CA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00CC:    D9 F0                   f2xm1
00CE:    D9 E8                   fld1    ; Load 1.0
00D0:    DE C1                   faddp    st(1)
00D2:    D9 FD                   fscale
00D4:    DD D9                   fstp     st(1)
00D6:    DC 56 E0                fcom     qword ptr [esi - 0x20]
00D9:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DB:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00DD:    73 07                   jae      0xe6    ; Conditional jump
00DF:    DD D8                   fstp     st(0)
00E1:    DD 46 E8                fld      qword ptr [esi - 0x18]
00E4:    EB 16                   jmp      0xfc
00E6:    D9 E8                   fld1    ; Load 1.0
00E8:    D8 D1                   fcom     st(1)
00EA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00EC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00EE:    72 07                   jb       0xf7    ; Conditional jump
00F0:    DD D8                   fstp     st(0)
00F2:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
00F5:    EB 05                   jmp      0xfc
00F7:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00F9:    DD 47 48                fld      qword ptr [edi + 0x48]
00FC:    DD 41 08                fld      qword ptr [ecx + 8]
00FF:    D8 C9                   fmul     st(1)
0101:    DD 59 08                fstp     qword ptr [ecx + 8]
0104:    DC CB                   fmul     st(3), st(0)
0106:    DC CA                   fmul     st(2), st(0)
0108:    DE C9                   fmulp    st(1)
010A:    DC 47 A8                fadd     qword ptr [edi - 0x58]
010D:    DD 19                   fstp     qword ptr [ecx]
010F:    DC 47 A0                fadd     qword ptr [edi - 0x60]
0112:    DD 1A                   fstp     qword ptr [edx]
0114:    DC 47 98                fadd     qword ptr [edi - 0x68]
0117:    DD 1B                   fstp     qword ptr [ebx]
0119:    89 D8                   mov      eax, ebx
011B:    5B                      pop      ebx
011C:    5F                      pop      edi
011D:    5E                      pop      esi
011E:    5D                      pop      ebp
011F:    C2 08 00                ret      8
