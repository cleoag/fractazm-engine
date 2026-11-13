; Disassembly: ATetraVS2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 440 bytes, decoded: 440 bytes (100.0%)
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
0039:    DC 0E                   fmul     qword ptr [esi]
003B:    DD 02                   fld      qword ptr [edx]
003D:    D8 C1                   fadd     st(1)
003F:    DD 03                   fld      qword ptr [ebx]
0041:    D8 C2                   fadd     st(2)
0043:    D9 C0                   fld      st(0)
0045:    D8 C2                   fadd     st(2)
0047:    D9 EE                   fldz    ; Load 0.0
0049:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
004B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
004F:    77 14                   ja       0x65    ; Conditional jump
0051:    D9 E0                   fchs    ; -ST(0)
0053:    D8 E2                   fsub     st(2)
0055:    DD 1A                   fstp     qword ptr [edx]
0057:    D9 E0                   fchs    ; -ST(0)
0059:    D8 E1                   fsub     st(1)
005B:    DD 1B                   fstp     qword ptr [ebx]
005D:    DD D8                   fstp     st(0)
005F:    EB 34                   jmp      0x95
0061:    90                      nop
0062:    90                      nop
0063:    90                      nop
0064:    90                      nop
0065:    D8 E2                   fsub     st(2)
0067:    D8 E2                   fsub     st(2)
0069:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006B:    D8 E2                   fsub     st(2)
006D:    D8 E2                   fsub     st(2)
006F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0071:    D9 C0                   fld      st(0)
0073:    D8 C2                   fadd     st(2)
0075:    D9 EE                   fldz    ; Load 0.0
0077:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0079:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
007B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
007D:    72 10                   jb       0x8f    ; Conditional jump
007F:    D9 E0                   fchs    ; -ST(0)
0081:    D8 C2                   fadd     st(2)
0083:    DD 1A                   fstp     qword ptr [edx]
0085:    D9 E0                   fchs    ; -ST(0)
0087:    D8 C1                   fadd     st(1)
0089:    DD 1B                   fstp     qword ptr [ebx]
008B:    DD D8                   fstp     st(0)
008D:    EB 06                   jmp      0x95
008F:    DD D8                   fstp     st(0)
0091:    DD D8                   fstp     st(0)
0093:    DD D8                   fstp     st(0)
0095:    DD 46 D8                fld      qword ptr [esi - 0x28]
0098:    DC 0E                   fmul     qword ptr [esi]
009A:    DD 01                   fld      qword ptr [ecx]
009C:    D8 C1                   fadd     st(1)
009E:    DD 03                   fld      qword ptr [ebx]
00A0:    D8 C2                   fadd     st(2)
00A2:    D9 C0                   fld      st(0)
00A4:    D8 C2                   fadd     st(2)
00A6:    D9 EE                   fldz    ; Load 0.0
00A8:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00AA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00AE:    77 14                   ja       0xc4    ; Conditional jump
00B0:    D9 E0                   fchs    ; -ST(0)
00B2:    D8 E2                   fsub     st(2)
00B4:    DD 19                   fstp     qword ptr [ecx]
00B6:    D9 E0                   fchs    ; -ST(0)
00B8:    D8 E1                   fsub     st(1)
00BA:    DD 1B                   fstp     qword ptr [ebx]
00BC:    DD D8                   fstp     st(0)
00BE:    EB 34                   jmp      0xf4
00C0:    90                      nop
00C1:    90                      nop
00C2:    90                      nop
00C3:    90                      nop
00C4:    D8 E2                   fsub     st(2)
00C6:    D8 E2                   fsub     st(2)
00C8:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00CA:    D8 E2                   fsub     st(2)
00CC:    D8 E2                   fsub     st(2)
00CE:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D0:    D9 C0                   fld      st(0)
00D2:    D8 C2                   fadd     st(2)
00D4:    D9 EE                   fldz    ; Load 0.0
00D6:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00D8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DA:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00DC:    72 10                   jb       0xee    ; Conditional jump
00DE:    D9 E0                   fchs    ; -ST(0)
00E0:    D8 C2                   fadd     st(2)
00E2:    DD 19                   fstp     qword ptr [ecx]
00E4:    D9 E0                   fchs    ; -ST(0)
00E6:    D8 C1                   fadd     st(1)
00E8:    DD 1B                   fstp     qword ptr [ebx]
00EA:    DD D8                   fstp     st(0)
00EC:    EB 06                   jmp      0xf4
00EE:    DD D8                   fstp     st(0)
00F0:    DD D8                   fstp     st(0)
00F2:    DD D8                   fstp     st(0)
00F4:    DD 46 D8                fld      qword ptr [esi - 0x28]
00F7:    DC 0E                   fmul     qword ptr [esi]
00F9:    DD 01                   fld      qword ptr [ecx]
00FB:    D8 C1                   fadd     st(1)
00FD:    DD 02                   fld      qword ptr [edx]
00FF:    D8 C2                   fadd     st(2)
0101:    D9 C0                   fld      st(0)
0103:    D8 C2                   fadd     st(2)
0105:    D9 EE                   fldz    ; Load 0.0
0107:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0109:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
010B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
010D:    77 14                   ja       0x123    ; Conditional jump
010F:    D9 E0                   fchs    ; -ST(0)
0111:    D8 E2                   fsub     st(2)
0113:    DD 19                   fstp     qword ptr [ecx]
0115:    D9 E0                   fchs    ; -ST(0)
0117:    D8 E1                   fsub     st(1)
0119:    DD 1A                   fstp     qword ptr [edx]
011B:    DD D8                   fstp     st(0)
011D:    EB 34                   jmp      0x153
011F:    90                      nop
0120:    90                      nop
0121:    90                      nop
0122:    90                      nop
0123:    D8 E2                   fsub     st(2)
0125:    D8 E2                   fsub     st(2)
0127:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0129:    D8 E2                   fsub     st(2)
012B:    D8 E2                   fsub     st(2)
012D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
012F:    D9 C0                   fld      st(0)
0131:    D8 C2                   fadd     st(2)
0133:    D9 EE                   fldz    ; Load 0.0
0135:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0137:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0139:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
013B:    72 10                   jb       0x14d    ; Conditional jump
013D:    D9 E0                   fchs    ; -ST(0)
013F:    D8 C2                   fadd     st(2)
0141:    DD 19                   fstp     qword ptr [ecx]
0143:    D9 E0                   fchs    ; -ST(0)
0145:    D8 C1                   fadd     st(1)
0147:    DD 1A                   fstp     qword ptr [edx]
0149:    DD D8                   fstp     st(0)
014B:    EB 06                   jmp      0x153
014D:    DD D8                   fstp     st(0)
014F:    DD D8                   fstp     st(0)
0151:    DD D8                   fstp     st(0)
0153:    DD 01                   fld      qword ptr [ecx]
0155:    DD 02                   fld      qword ptr [edx]
0157:    DD 03                   fld      qword ptr [ebx]
0159:    D9 C0                   fld      st(0)
015B:    D8 C9                   fmul     st(1)
015D:    D9 C2                   fld      st(2)
015F:    D8 CB                   fmul     st(3)
0161:    DE C1                   faddp    st(1)
0163:    D9 C3                   fld      st(3)
0165:    D8 CC                   fmul     st(4)
0167:    DE C1                   faddp    st(1)
0169:    DC 4E C8                fmul     qword ptr [esi - 0x38]
016C:    DC 56 E0                fcom     qword ptr [esi - 0x20]
016F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0171:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0173:    73 07                   jae      0x17c    ; Conditional jump
0175:    DD D8                   fstp     st(0)
0177:    DD 46 E8                fld      qword ptr [esi - 0x18]
017A:    EB 16                   jmp      0x192
017C:    D9 E8                   fld1    ; Load 1.0
017E:    D8 D1                   fcom     st(1)
0180:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0182:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0184:    72 07                   jb       0x18d    ; Conditional jump
0186:    DD D8                   fstp     st(0)
0188:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
018B:    EB 05                   jmp      0x192
018D:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
018F:    DD 47 48                fld      qword ptr [edi + 0x48]
0192:    DD 41 08                fld      qword ptr [ecx + 8]
0195:    D8 C9                   fmul     st(1)
0197:    DD 59 08                fstp     qword ptr [ecx + 8]
019A:    DC CB                   fmul     st(3), st(0)
019C:    DC CA                   fmul     st(2), st(0)
019E:    DE C9                   fmulp    st(1)
01A0:    DC 47 A8                fadd     qword ptr [edi - 0x58]
01A3:    DD 19                   fstp     qword ptr [ecx]
01A5:    DC 47 A0                fadd     qword ptr [edi - 0x60]
01A8:    DD 1A                   fstp     qword ptr [edx]
01AA:    DC 47 98                fadd     qword ptr [edi - 0x68]
01AD:    DD 1B                   fstp     qword ptr [ebx]
01AF:    89 D8                   mov      eax, ebx
01B1:    5B                      pop      ebx
01B2:    5F                      pop      edi
01B3:    5E                      pop      esi
01B4:    5D                      pop      ebp
01B5:    C2 08 00                ret      8
