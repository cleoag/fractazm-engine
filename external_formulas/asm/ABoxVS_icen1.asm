; Disassembly: ABoxVS_icen1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 371 bytes, decoded: 371 bytes (100.0%)
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
002E:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0031:    DE C1                   faddp    st(1)
0033:    DD 5F 48                fstp     qword ptr [edi + 0x48]
0036:    DD 46 E0                fld      qword ptr [esi - 0x20]
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
00A1:    83 7E B4 00             cmp      dword ptr [esi - 0x4c], 0
00A5:    75 1A                   jne      0xc1    ; Conditional jump
00A7:    DD 46 C8                fld      qword ptr [esi - 0x38]
00AA:    D8 C1                   fadd     st(1)
00AC:    DD D8                   fstp     st(0)
00AE:    90                      nop
00AF:    DD 46 C0                fld      qword ptr [esi - 0x40]
00B2:    D8 C2                   fadd     st(2)
00B4:    DD D8                   fstp     st(0)
00B6:    90                      nop
00B7:    DD 46 B8                fld      qword ptr [esi - 0x48]
00BA:    D8 C3                   fadd     st(3)
00BC:    DD D8                   fstp     st(0)
00BE:    90                      nop
00BF:    EB 18                   jmp      0xd9
00C1:    DD 47 98                fld      qword ptr [edi - 0x68]
00C4:    D8 C1                   fadd     st(1)
00C6:    DD D8                   fstp     st(0)
00C8:    90                      nop
00C9:    DD 47 A0                fld      qword ptr [edi - 0x60]
00CC:    D8 C2                   fadd     st(2)
00CE:    DD D8                   fstp     st(0)
00D0:    90                      nop
00D1:    DD 47 A8                fld      qword ptr [edi - 0x58]
00D4:    D8 C3                   fadd     st(3)
00D6:    DD D8                   fstp     st(0)
00D8:    90                      nop
00D9:    D9 C0                   fld      st(0)
00DB:    D8 C9                   fmul     st(1)
00DD:    D9 C2                   fld      st(2)
00DF:    D8 CB                   fmul     st(3)
00E1:    DE C1                   faddp    st(1)
00E3:    D9 C3                   fld      st(3)
00E5:    D8 CC                   fmul     st(4)
00E7:    DE C1                   faddp    st(1)
00E9:    DD 46 E8                fld      qword ptr [esi - 0x18]
00EC:    D8 C8                   fmul     st(0)
00EE:    D8 D9                   fcomp    st(1)
00F0:    90                      nop
00F1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00F5:    72 13                   jb       0x10a    ; Conditional jump
00F7:    DD D8                   fstp     st(0)
00F9:    DD 47 48                fld      qword ptr [edi + 0x48]
00FC:    DC 4E D0                fmul     qword ptr [esi - 0x30]
00FF:    DD 46 E8                fld      qword ptr [esi - 0x18]
0102:    D8 C8                   fmul     st(0)
0104:    DE F9                   fdivp    st(1)
0106:    90                      nop
0107:    90                      nop
0108:    EB 1A                   jmp      0x124
010A:    DD 46 D0                fld      qword ptr [esi - 0x30]
010D:    D8 D1                   fcom     st(1)
010F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0111:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0113:    72 0A                   jb       0x11f    ; Conditional jump
0115:    DD D8                   fstp     st(0)
0117:    DC 76 D0                fdiv     qword ptr [esi - 0x30]
011A:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
011D:    EB 05                   jmp      0x124
011F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0121:    DD 47 48                fld      qword ptr [edi + 0x48]
0124:    DD 41 08                fld      qword ptr [ecx + 8]
0127:    D8 C9                   fmul     st(1)
0129:    DD 59 08                fstp     qword ptr [ecx + 8]
012C:    DC CB                   fmul     st(3), st(0)
012E:    DC CA                   fmul     st(2), st(0)
0130:    DE C9                   fmulp    st(1)
0132:    83 7E B4 00             cmp      dword ptr [esi - 0x4c], 0
0136:    75 1A                   jne      0x152    ; Conditional jump
0138:    DC 47 A8                fadd     qword ptr [edi - 0x58]
013B:    DC 66 B8                fsub     qword ptr [esi - 0x48]
013E:    DD 19                   fstp     qword ptr [ecx]
0140:    DC 47 A0                fadd     qword ptr [edi - 0x60]
0143:    DC 66 C0                fsub     qword ptr [esi - 0x40]
0146:    DD 1A                   fstp     qword ptr [edx]
0148:    DC 47 98                fadd     qword ptr [edi - 0x68]
014B:    DC 66 C8                fsub     qword ptr [esi - 0x38]
014E:    DD 1B                   fstp     qword ptr [ebx]
0150:    EB 18                   jmp      0x16a
0152:    DC 67 A8                fsub     qword ptr [edi - 0x58]
0155:    DC 46 B8                fadd     qword ptr [esi - 0x48]
0158:    DD 19                   fstp     qword ptr [ecx]
015A:    DC 67 A0                fsub     qword ptr [edi - 0x60]
015D:    DC 46 C0                fadd     qword ptr [esi - 0x40]
0160:    DD 1A                   fstp     qword ptr [edx]
0162:    DC 67 98                fsub     qword ptr [edi - 0x68]
0165:    DC 46 C8                fadd     qword ptr [esi - 0x38]
0168:    DD 1B                   fstp     qword ptr [ebx]
016A:    89 D8                   mov      eax, ebx
016C:    5B                      pop      ebx
016D:    5F                      pop      edi
016E:    5E                      pop      esi
016F:    5D                      pop      ebp
0170:    C2 08 00                ret      8
