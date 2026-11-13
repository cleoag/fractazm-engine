; Disassembly: ABoxVaryScale2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 317 bytes, decoded: 317 bytes (100.0%)
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
00A3:    DD 46 C0                fld      qword ptr [esi - 0x40]
00A6:    E8 6C 00 00 00          call     0x117
00AB:    D9 C2                   fld      st(2)
00AD:    DD 46 C0                fld      qword ptr [esi - 0x40]
00B0:    E8 62 00 00 00          call     0x117
00B5:    DE C1                   faddp    st(1)
00B7:    D9 C3                   fld      st(3)
00B9:    DD 46 C0                fld      qword ptr [esi - 0x40]
00BC:    E8 56 00 00 00          call     0x117
00C1:    DE C1                   faddp    st(1)
00C3:    DD 46 C8                fld      qword ptr [esi - 0x38]
00C6:    E8 4C 00 00 00          call     0x117
00CB:    DC 56 E0                fcom     qword ptr [esi - 0x20]
00CE:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00D0:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D2:    73 07                   jae      0xdb    ; Conditional jump
00D4:    DD D8                   fstp     st(0)
00D6:    DD 46 E8                fld      qword ptr [esi - 0x18]
00D9:    EB 16                   jmp      0xf1
00DB:    D9 E8                   fld1    ; Load 1.0
00DD:    D8 D1                   fcom     st(1)
00DF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E1:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00E3:    72 07                   jb       0xec    ; Conditional jump
00E5:    DD D8                   fstp     st(0)
00E7:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
00EA:    EB 05                   jmp      0xf1
00EC:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00EE:    DD 47 48                fld      qword ptr [edi + 0x48]
00F1:    DD 41 08                fld      qword ptr [ecx + 8]
00F4:    D8 C9                   fmul     st(1)
00F6:    DD 59 08                fstp     qword ptr [ecx + 8]
00F9:    DC CB                   fmul     st(3), st(0)
00FB:    DC CA                   fmul     st(2), st(0)
00FD:    DE C9                   fmulp    st(1)
00FF:    DC 47 A8                fadd     qword ptr [edi - 0x58]
0102:    DD 19                   fstp     qword ptr [ecx]
0104:    DC 47 A0                fadd     qword ptr [edi - 0x60]
0107:    DD 1A                   fstp     qword ptr [edx]
0109:    DC 47 98                fadd     qword ptr [edi - 0x68]
010C:    DD 1B                   fstp     qword ptr [ebx]
010E:    89 D8                   mov      eax, ebx
0110:    5B                      pop      ebx
0111:    5F                      pop      edi
0112:    5E                      pop      esi
0113:    5D                      pop      ebp
0114:    C2 08 00                ret      8
0117:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0119:    D9 E1                   fabs    ; |ST(0)|
011B:    D9 ED                   fldln2    ; Load ln(2)
011D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
011F:    D9 F1                   fyl2x
0121:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0123:    DE C9                   fmulp    st(1)
0125:    D9 EA                   fldl2e    ; Load log2(e)
0127:    DE C9                   fmulp    st(1)
0129:    D9 C0                   fld      st(0)
012B:    D9 FC                   frndint    ; round(ST(0))
012D:    DC E9                   fsub     st(1), st(0)
012F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0131:    D9 F0                   f2xm1
0133:    D9 E8                   fld1    ; Load 1.0
0135:    DE C1                   faddp    st(1)
0137:    D9 FD                   fscale
0139:    DD D9                   fstp     st(1)
013B:    C3                      ret
013C:    90                      nop
