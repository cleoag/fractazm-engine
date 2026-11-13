; Disassembly: Mandalex.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 536 bytes, decoded: 536 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 3C 00 00 00       sub      esp, 0x3c
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
000F:    89 C3                   mov      ebx, eax
0011:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
0014:    81 C7 80 00 00 00       add      edi, 0x80
001A:    DD 46 AC                fld      qword ptr [esi - 0x54]
001D:    E8 08 00 00 00          call     0x2a
0022:    E9 9E 00 00 00          jmp      0xc5
0027:    90                      nop
0028:    90                      nop
0029:    90                      nop
002A:    B8 00 00 00 00          mov      eax, 0
002F:    89 45 F8                mov      dword ptr [ebp - 8], eax
0032:    DD 03                   fld      qword ptr [ebx]
0034:    D9 E1                   fabs    ; |ST(0)|
0036:    D8 D9                   fcomp    st(1)
0038:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
003A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
003C:    73 03                   jae      0x41    ; Conditional jump
003E:    FF 45 F8                inc      dword ptr [ebp - 8]
0041:    DD 02                   fld      qword ptr [edx]
0043:    D9 E1                   fabs    ; |ST(0)|
0045:    D8 D9                   fcomp    st(1)
0047:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0049:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
004B:    73 03                   jae      0x50    ; Conditional jump
004D:    FF 45 F8                inc      dword ptr [ebp - 8]
0050:    DD 01                   fld      qword ptr [ecx]
0052:    D9 E1                   fabs    ; |ST(0)|
0054:    D8 D9                   fcomp    st(1)
0056:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0058:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005A:    73 03                   jae      0x5f    ; Conditional jump
005C:    FF 45 F8                inc      dword ptr [ebp - 8]
005F:    8B 46 98                mov      eax, dword ptr [esi - 0x68]
0062:    83 E0 03                and      eax, 3
0065:    33 45 F8                xor      eax, dword ptr [ebp - 8]
0068:    0F 85 51 00 00 00       jne      0xbf    ; Conditional jump
006E:    DC 4E 9C                fmul     qword ptr [esi - 0x64]
0071:    DD 03                   fld      qword ptr [ebx]
0073:    D9 EE                   fldz    ; Load 0.0
0075:    D8 D9                   fcomp    st(1)
0077:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0079:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
007B:    73 04                   jae      0x81    ; Conditional jump
007D:    D9 E8                   fld1    ; Load 1.0
007F:    EB 04                   jmp      0x85
0081:    D9 E8                   fld1    ; Load 1.0
0083:    D9 E0                   fchs    ; -ST(0)
0085:    D8 CA                   fmul     st(2)
0087:    DE E9                   fsubp    st(1)
0089:    DD 1B                   fstp     qword ptr [ebx]
008B:    DD 02                   fld      qword ptr [edx]
008D:    D9 EE                   fldz    ; Load 0.0
008F:    D8 D9                   fcomp    st(1)
0091:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0093:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0095:    73 04                   jae      0x9b    ; Conditional jump
0097:    D9 E8                   fld1    ; Load 1.0
0099:    EB 04                   jmp      0x9f
009B:    D9 E8                   fld1    ; Load 1.0
009D:    D9 E0                   fchs    ; -ST(0)
009F:    D8 CA                   fmul     st(2)
00A1:    DE E9                   fsubp    st(1)
00A3:    DD 1A                   fstp     qword ptr [edx]
00A5:    DD 01                   fld      qword ptr [ecx]
00A7:    D9 EE                   fldz    ; Load 0.0
00A9:    D8 D9                   fcomp    st(1)
00AB:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AD:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00AF:    73 04                   jae      0xb5    ; Conditional jump
00B1:    D9 E8                   fld1    ; Load 1.0
00B3:    EB 04                   jmp      0xb9
00B5:    D9 E8                   fld1    ; Load 1.0
00B7:    D9 E0                   fchs    ; -ST(0)
00B9:    D8 CA                   fmul     st(2)
00BB:    DE E9                   fsubp    st(1)
00BD:    DD 19                   fstp     qword ptr [ecx]
00BF:    DD D8                   fstp     st(0)
00C1:    C3                      ret
00C2:    90                      nop
00C3:    90                      nop
00C4:    90                      nop
00C5:    DD 03                   fld      qword ptr [ebx]
00C7:    DD 02                   fld      qword ptr [edx]
00C9:    DD 01                   fld      qword ptr [ecx]
00CB:    90                      nop
00CC:    D9 C0                   fld      st(0)
00CE:    D8 4E BC                fmul     dword ptr [esi - 0x44]
00D1:    D9 C2                   fld      st(2)
00D3:    D8 4E B8                fmul     dword ptr [esi - 0x48]
00D6:    DE C1                   faddp    st(1)
00D8:    D9 C3                   fld      st(3)
00DA:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
00DD:    DE C1                   faddp    st(1)
00DF:    D9 C1                   fld      st(1)
00E1:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
00E4:    D9 C3                   fld      st(3)
00E6:    D8 4E D0                fmul     dword ptr [esi - 0x30]
00E9:    DE C1                   faddp    st(1)
00EB:    D9 C4                   fld      st(4)
00ED:    D8 4E CC                fmul     dword ptr [esi - 0x34]
00F0:    DE C1                   faddp    st(1)
00F2:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00F4:    D8 4E C8                fmul     dword ptr [esi - 0x38]
00F7:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00F9:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
00FC:    DE C3                   faddp    st(3)
00FE:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0100:    D8 4E C0                fmul     dword ptr [esi - 0x40]
0103:    DE C2                   faddp    st(2)
0105:    DD 19                   fstp     qword ptr [ecx]
0107:    DD 1A                   fstp     qword ptr [edx]
0109:    DD 1B                   fstp     qword ptr [ebx]
010B:    90                      nop
010C:    DD 46 D8                fld      qword ptr [esi - 0x28]
010F:    D9 C0                   fld      st(0)
0111:    D9 C0                   fld      st(0)
0113:    DC C2                   fadd     st(2), st(0)
0115:    D9 E0                   fchs    ; -ST(0)
0117:    DD 03                   fld      qword ptr [ebx]
0119:    D8 D2                   fcom     st(2)
011B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
011D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
011F:    72 06                   jb       0x127    ; Conditional jump
0121:    D9 E0                   fchs    ; -ST(0)
0123:    D8 C3                   fadd     st(3)
0125:    EB 0C                   jmp      0x133
0127:    D8 D1                   fcom     st(1)
0129:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
012B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
012D:    73 04                   jae      0x133    ; Conditional jump
012F:    D9 E0                   fchs    ; -ST(0)
0131:    D8 E3                   fsub     st(3)
0133:    DD 02                   fld      qword ptr [edx]
0135:    D8 D3                   fcom     st(3)
0137:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0139:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
013B:    72 06                   jb       0x143    ; Conditional jump
013D:    D9 E0                   fchs    ; -ST(0)
013F:    D8 C4                   fadd     st(4)
0141:    EB 0C                   jmp      0x14f
0143:    D8 D2                   fcom     st(2)
0145:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0147:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0149:    73 04                   jae      0x14f    ; Conditional jump
014B:    D9 E0                   fchs    ; -ST(0)
014D:    D8 E4                   fsub     st(4)
014F:    DD 01                   fld      qword ptr [ecx]
0151:    D8 D4                   fcom     st(4)
0153:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0155:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0157:    72 06                   jb       0x15f    ; Conditional jump
0159:    D9 E0                   fchs    ; -ST(0)
015B:    D8 C5                   fadd     st(5)
015D:    EB 0C                   jmp      0x16b
015F:    D8 D3                   fcom     st(3)
0161:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0163:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0165:    73 04                   jae      0x16b    ; Conditional jump
0167:    D9 E0                   fchs    ; -ST(0)
0169:    D8 E5                   fsub     st(5)
016B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
016D:    DD D8                   fstp     st(0)
016F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0171:    DD D8                   fstp     st(0)
0173:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0175:    DD D8                   fstp     st(0)
0177:    D9 C0                   fld      st(0)
0179:    D8 4E BC                fmul     dword ptr [esi - 0x44]
017C:    D9 C2                   fld      st(2)
017E:    D8 4E B8                fmul     dword ptr [esi - 0x48]
0181:    DE C1                   faddp    st(1)
0183:    D9 C3                   fld      st(3)
0185:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
0188:    DE C1                   faddp    st(1)
018A:    D9 C1                   fld      st(1)
018C:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
018F:    D9 C3                   fld      st(3)
0191:    D8 4E D0                fmul     dword ptr [esi - 0x30]
0194:    DE C1                   faddp    st(1)
0196:    D9 C4                   fld      st(4)
0198:    D8 4E CC                fmul     dword ptr [esi - 0x34]
019B:    DE C1                   faddp    st(1)
019D:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
019F:    D8 4E C8                fmul     dword ptr [esi - 0x38]
01A2:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
01A4:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
01A7:    DE C3                   faddp    st(3)
01A9:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
01AB:    D8 4E C0                fmul     dword ptr [esi - 0x40]
01AE:    DE C2                   faddp    st(2)
01B0:    90                      nop
01B1:    D9 C0                   fld      st(0)
01B3:    D8 C9                   fmul     st(1)
01B5:    D9 C2                   fld      st(2)
01B7:    D8 CB                   fmul     st(3)
01B9:    DE C1                   faddp    st(1)
01BB:    D9 C3                   fld      st(3)
01BD:    D8 CC                   fmul     st(4)
01BF:    DE C1                   faddp    st(1)
01C1:    DC 56 E0                fcom     qword ptr [esi - 0x20]
01C4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01C6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01C8:    73 07                   jae      0x1d1    ; Conditional jump
01CA:    DD D8                   fstp     st(0)
01CC:    DD 46 E8                fld      qword ptr [esi - 0x18]
01CF:    EB 16                   jmp      0x1e7
01D1:    D9 E8                   fld1    ; Load 1.0
01D3:    D8 D1                   fcom     st(1)
01D5:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01D7:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01D9:    72 07                   jb       0x1e2    ; Conditional jump
01DB:    DD D8                   fstp     st(0)
01DD:    DC 7E F0                fdivr    qword ptr [esi - 0x10]
01E0:    EB 05                   jmp      0x1e7
01E2:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
01E4:    DD 46 F0                fld      qword ptr [esi - 0x10]
01E7:    DD 41 08                fld      qword ptr [ecx + 8]
01EA:    D8 C9                   fmul     st(1)
01EC:    DD 59 08                fstp     qword ptr [ecx + 8]
01EF:    DC CB                   fmul     st(3), st(0)
01F1:    DC CA                   fmul     st(2), st(0)
01F3:    DE C9                   fmulp    st(1)
01F5:    DC 47 A8                fadd     qword ptr [edi - 0x58]
01F8:    DD 19                   fstp     qword ptr [ecx]
01FA:    DC 47 A0                fadd     qword ptr [edi - 0x60]
01FD:    DD 1A                   fstp     qword ptr [edx]
01FF:    DC 47 98                fadd     qword ptr [edi - 0x68]
0202:    DD 1B                   fstp     qword ptr [ebx]
0204:    90                      nop
0205:    DD 46 A4                fld      qword ptr [esi - 0x5c]
0208:    E8 1D FE FF FF          call     0x2a
020D:    89 D8                   mov      eax, ebx
020F:    5F                      pop      edi
0210:    5E                      pop      esi
0211:    5B                      pop      ebx
0212:    8B E5                   mov      esp, ebp
0214:    5D                      pop      ebp
0215:    C2 08 00                ret      8
