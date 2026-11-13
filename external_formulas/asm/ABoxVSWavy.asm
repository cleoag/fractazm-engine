; Disassembly: ABoxVSWavy.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 375 bytes, decoded: 375 bytes (100.0%)
; DEoption: 11

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 50 00 00 00       sub      esp, 0x50
0009:    56                      push     esi
000A:    57                      push     edi
000B:    53                      push     ebx
000C:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
000F:    89 C3                   mov      ebx, eax
0011:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
0014:    81 C7 80 00 00 00       add      edi, 0x80
001A:    83 7F 50 00             cmp      dword ptr [edi + 0x50], 0
001E:    7F 09                   jg       0x29    ; Conditional jump
0020:    FF 47 50                inc      dword ptr [edi + 0x50]
0023:    DD 46 F0                fld      qword ptr [esi - 0x10]
0026:    DD 5F 48                fstp     qword ptr [edi + 0x48]
0029:    DD 47 48                fld      qword ptr [edi + 0x48]
002C:    D9 E8                   fld1    ; Load 1.0
002E:    D9 C1                   fld      st(1)
0030:    D9 E1                   fabs    ; |ST(0)|
0032:    DE E1                   fsubrp   st(1)
0034:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0037:    DE C1                   faddp    st(1)
0039:    DD 5F 48                fstp     qword ptr [edi + 0x48]
003C:    DD 46 D8                fld      qword ptr [esi - 0x28]
003F:    D9 C0                   fld      st(0)
0041:    D9 C0                   fld      st(0)
0043:    DC C2                   fadd     st(2), st(0)
0045:    D9 E0                   fchs    ; -ST(0)
0047:    DD 03                   fld      qword ptr [ebx]
0049:    D8 D2                   fcom     st(2)
004B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
004F:    72 06                   jb       0x57    ; Conditional jump
0051:    D9 E0                   fchs    ; -ST(0)
0053:    D8 C3                   fadd     st(3)
0055:    EB 0C                   jmp      0x63
0057:    D8 D1                   fcom     st(1)
0059:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005D:    73 04                   jae      0x63    ; Conditional jump
005F:    D9 E0                   fchs    ; -ST(0)
0061:    D8 E3                   fsub     st(3)
0063:    DD 02                   fld      qword ptr [edx]
0065:    D8 D3                   fcom     st(3)
0067:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0069:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006B:    72 06                   jb       0x73    ; Conditional jump
006D:    D9 E0                   fchs    ; -ST(0)
006F:    D8 C4                   fadd     st(4)
0071:    EB 0C                   jmp      0x7f
0073:    D8 D2                   fcom     st(2)
0075:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0077:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0079:    73 04                   jae      0x7f    ; Conditional jump
007B:    D9 E0                   fchs    ; -ST(0)
007D:    D8 E4                   fsub     st(4)
007F:    DD 01                   fld      qword ptr [ecx]
0081:    D8 D4                   fcom     st(4)
0083:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0085:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0087:    72 06                   jb       0x8f    ; Conditional jump
0089:    D9 E0                   fchs    ; -ST(0)
008B:    D8 C5                   fadd     st(5)
008D:    EB 0C                   jmp      0x9b
008F:    D8 D3                   fcom     st(3)
0091:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0093:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0095:    73 04                   jae      0x9b    ; Conditional jump
0097:    D9 E0                   fchs    ; -ST(0)
0099:    D8 E5                   fsub     st(5)
009B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
009D:    DD D8                   fstp     st(0)
009F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A1:    DD D8                   fstp     st(0)
00A3:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A5:    DD D8                   fstp     st(0)
00A7:    D9 C2                   fld      st(2)
00A9:    DC 4E C0                fmul     qword ptr [esi - 0x40]
00AC:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00AE:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00B1:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00B4:    D9 C1                   fld      st(1)
00B6:    DC 4E C0                fmul     qword ptr [esi - 0x40]
00B9:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00BB:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00BE:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00C1:    D9 C0                   fld      st(0)
00C3:    DC 4E C0                fmul     qword ptr [esi - 0x40]
00C6:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00C8:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00CB:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
00CE:    90                      nop
00CF:    D9 C0                   fld      st(0)
00D1:    D8 C8                   fmul     st(0)
00D3:    D9 C2                   fld      st(2)
00D5:    D8 C8                   fmul     st(0)
00D7:    DE C1                   faddp    st(1)
00D9:    D9 C3                   fld      st(3)
00DB:    D8 C8                   fmul     st(0)
00DD:    DE C1                   faddp    st(1)
00DF:    D9 FA                   fsqrt    ; sqrt(ST(0))
00E1:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
00E4:    90                      nop
00E5:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00E8:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00EB:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00EE:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
00F1:    DE C1                   faddp    st(1)
00F3:    DD 45 D0                fld      qword ptr [ebp - 0x30]
00F6:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00F9:    DE C1                   faddp    st(1)
00FB:    DC 4E C8                fmul     qword ptr [esi - 0x38]
00FE:    90                      nop
00FF:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0102:    DC 45 E0                fadd     qword ptr [ebp - 0x20]
0105:    DC 45 D0                fadd     qword ptr [ebp - 0x30]
0108:    DC 4E B8                fmul     qword ptr [esi - 0x48]
010B:    83 7E AC 01             cmp      dword ptr [esi - 0x54], 1
010F:    74 0E                   je       0x11f    ; Conditional jump
0111:    DE C1                   faddp    st(1)
0113:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0116:    DC 4E B0                fmul     qword ptr [esi - 0x50]
0119:    DE C1                   faddp    st(1)
011B:    90                      nop
011C:    90                      nop
011D:    EB 08                   jmp      0x127
011F:    DC 46 B0                fadd     qword ptr [esi - 0x50]
0122:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0125:    DE C1                   faddp    st(1)
0127:    D8 C8                   fmul     st(0)
0129:    DC 56 E0                fcom     qword ptr [esi - 0x20]
012C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
012E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0130:    73 07                   jae      0x139    ; Conditional jump
0132:    DD D8                   fstp     st(0)
0134:    DD 46 E8                fld      qword ptr [esi - 0x18]
0137:    EB 16                   jmp      0x14f
0139:    D9 E8                   fld1    ; Load 1.0
013B:    D8 D1                   fcom     st(1)
013D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
013F:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0141:    72 07                   jb       0x14a    ; Conditional jump
0143:    DD D8                   fstp     st(0)
0145:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
0148:    EB 05                   jmp      0x14f
014A:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
014C:    DD 47 48                fld      qword ptr [edi + 0x48]
014F:    DD 41 08                fld      qword ptr [ecx + 8]
0152:    D8 C9                   fmul     st(1)
0154:    DD 59 08                fstp     qword ptr [ecx + 8]
0157:    DC CB                   fmul     st(3), st(0)
0159:    DC CA                   fmul     st(2), st(0)
015B:    DE C9                   fmulp    st(1)
015D:    DC 47 A8                fadd     qword ptr [edi - 0x58]
0160:    DD 19                   fstp     qword ptr [ecx]
0162:    DC 47 A0                fadd     qword ptr [edi - 0x60]
0165:    DD 1A                   fstp     qword ptr [edx]
0167:    DC 47 98                fadd     qword ptr [edi - 0x68]
016A:    DD 1B                   fstp     qword ptr [ebx]
016C:    89 D8                   mov      eax, ebx
016E:    5B                      pop      ebx
016F:    5F                      pop      edi
0170:    5E                      pop      esi
0171:    89 EC                   mov      esp, ebp
0173:    5D                      pop      ebp
0174:    C2 08 00                ret      8
