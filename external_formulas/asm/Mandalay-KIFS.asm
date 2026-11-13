; Disassembly: Mandalay-KIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 415 bytes, decoded: 415 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    83 EC 50                sub      esp, 0x50
0006:    90                      nop
0007:    90                      nop
0008:    90                      nop
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
000F:    89 C3                   mov      ebx, eax
0011:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
0014:    81 C7 80 00 00 00       add      edi, 0x80
001A:    90                      nop
001B:    90                      nop
001C:    90                      nop
001D:    90                      nop
001E:    DD 01                   fld      qword ptr [ecx]
0020:    D9 E1                   fabs    ; |ST(0)|
0022:    DD 02                   fld      qword ptr [edx]
0024:    D9 E1                   fabs    ; |ST(0)|
0026:    DD 03                   fld      qword ptr [ebx]
0028:    D9 E1                   fabs    ; |ST(0)|
002A:    D8 D1                   fcom     st(1)
002C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
002E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0030:    73 02                   jae      0x34    ; Conditional jump
0032:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0034:    D8 D2                   fcom     st(2)
0036:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0038:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
003A:    73 02                   jae      0x3e    ; Conditional jump
003C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0040:    D8 D2                   fcom     st(2)
0042:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0044:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0046:    73 02                   jae      0x4a    ; Conditional jump
0048:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
004A:    DD 1A                   fstp     qword ptr [edx]
004C:    DD 1B                   fstp     qword ptr [ebx]
004E:    DD 19                   fstp     qword ptr [ecx]
0050:    90                      nop
0051:    DD 46 D8                fld      qword ptr [esi - 0x28]
0054:    D8 C0                   fadd     st(0)
0056:    DD 03                   fld      qword ptr [ebx]
0058:    D8 E1                   fsub     st(1)
005A:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
005D:    DD D8                   fstp     st(0)
005F:    90                      nop
0060:    DD 46 D8                fld      qword ptr [esi - 0x28]
0063:    DD 03                   fld      qword ptr [ebx]
0065:    D8 E1                   fsub     st(1)
0067:    D9 E1                   fabs    ; |ST(0)|
0069:    D8 E9                   fsubr    st(1)
006B:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
006E:    DD 02                   fld      qword ptr [edx]
0070:    D8 E1                   fsub     st(1)
0072:    D9 E1                   fabs    ; |ST(0)|
0074:    D8 E9                   fsubr    st(1)
0076:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0079:    DD D8                   fstp     st(0)
007B:    DD 46 A4                fld      qword ptr [esi - 0x5c]
007E:    D9 EE                   fldz    ; Load 0.0
0080:    D8 D9                   fcomp    st(1)
0082:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0084:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0086:    73 0A                   jae      0x92    ; Conditional jump
0088:    DD 01                   fld      qword ptr [ecx]
008A:    D8 E1                   fsub     st(1)
008C:    D9 E1                   fabs    ; |ST(0)|
008E:    D8 E9                   fsubr    st(1)
0090:    EB 02                   jmp      0x94
0092:    DC 01                   fadd     qword ptr [ecx]
0094:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0097:    DD D8                   fstp     st(0)
0099:    90                      nop
009A:    90                      nop
009B:    90                      nop
009C:    DD 45 F0                fld      qword ptr [ebp - 0x10]
009F:    D9 EE                   fldz    ; Load 0.0
00A1:    D8 D9                   fcomp    st(1)
00A3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A5:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A7:    73 4D                   jae      0xf6    ; Conditional jump
00A9:    DD 02                   fld      qword ptr [edx]
00AB:    D8 D9                   fcomp    st(1)
00AD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AF:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00B1:    73 43                   jae      0xf6    ; Conditional jump
00B3:    DD 46 AC                fld      qword ptr [esi - 0x54]
00B6:    DC 02                   fadd     qword ptr [edx]
00B8:    D8 D9                   fcomp    st(1)
00BA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00BC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00BE:    73 1B                   jae      0xdb    ; Conditional jump
00C0:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00C3:    DC 46 AC                fadd     qword ptr [esi - 0x54]
00C6:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00C9:    DD 46 AC                fld      qword ptr [esi - 0x54]
00CC:    DC 66 D8                fsub     qword ptr [esi - 0x28]
00CF:    DC 02                   fadd     qword ptr [edx]
00D1:    D9 E1                   fabs    ; |ST(0)|
00D3:    DC 6E D8                fsubr    qword ptr [esi - 0x28]
00D6:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00D9:    EB 1B                   jmp      0xf6
00DB:    DD 02                   fld      qword ptr [edx]
00DD:    D9 E0                   fchs    ; -ST(0)
00DF:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00E2:    DD 03                   fld      qword ptr [ebx]
00E4:    DC 66 D8                fsub     qword ptr [esi - 0x28]
00E7:    DC 66 D8                fsub     qword ptr [esi - 0x28]
00EA:    DC 66 D8                fsub     qword ptr [esi - 0x28]
00ED:    D9 E1                   fabs    ; |ST(0)|
00EF:    DC 6E D8                fsubr    qword ptr [esi - 0x28]
00F2:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00F5:    90                      nop
00F6:    DD D8                   fstp     st(0)
00F8:    90                      nop
00F9:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00FC:    DD 45 D0                fld      qword ptr [ebp - 0x30]
00FF:    DD 45 C8                fld      qword ptr [ebp - 0x38]
0102:    D9 C0                   fld      st(0)
0104:    D8 4E BC                fmul     dword ptr [esi - 0x44]
0107:    D9 C2                   fld      st(2)
0109:    D8 4E B8                fmul     dword ptr [esi - 0x48]
010C:    DE C1                   faddp    st(1)
010E:    D9 C3                   fld      st(3)
0110:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
0113:    DE C1                   faddp    st(1)
0115:    D9 C1                   fld      st(1)
0117:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
011A:    D9 C3                   fld      st(3)
011C:    D8 4E D0                fmul     dword ptr [esi - 0x30]
011F:    DE C1                   faddp    st(1)
0121:    D9 C4                   fld      st(4)
0123:    D8 4E CC                fmul     dword ptr [esi - 0x34]
0126:    DE C1                   faddp    st(1)
0128:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
012A:    D8 4E C8                fmul     dword ptr [esi - 0x38]
012D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
012F:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
0132:    DE C3                   faddp    st(3)
0134:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0136:    D8 4E C0                fmul     dword ptr [esi - 0x40]
0139:    DE C2                   faddp    st(2)
013B:    90                      nop
013C:    D9 C0                   fld      st(0)
013E:    D8 C9                   fmul     st(1)
0140:    D9 C2                   fld      st(2)
0142:    D8 CB                   fmul     st(3)
0144:    DE C1                   faddp    st(1)
0146:    D9 C3                   fld      st(3)
0148:    D8 CC                   fmul     st(4)
014A:    DE C1                   faddp    st(1)
014C:    DC 4E 9C                fmul     qword ptr [esi - 0x64]
014F:    90                      nop
0150:    DC 56 E0                fcom     qword ptr [esi - 0x20]
0153:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0155:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0157:    73 07                   jae      0x160    ; Conditional jump
0159:    DD D8                   fstp     st(0)
015B:    DD 46 E8                fld      qword ptr [esi - 0x18]
015E:    EB 16                   jmp      0x176
0160:    D9 E8                   fld1    ; Load 1.0
0162:    D8 D1                   fcom     st(1)
0164:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0166:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0168:    72 07                   jb       0x171    ; Conditional jump
016A:    DD D8                   fstp     st(0)
016C:    DC 7E F0                fdivr    qword ptr [esi - 0x10]
016F:    EB 05                   jmp      0x176
0171:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0173:    DD 46 F0                fld      qword ptr [esi - 0x10]
0176:    DD 41 08                fld      qword ptr [ecx + 8]
0179:    D8 C9                   fmul     st(1)
017B:    DD 59 08                fstp     qword ptr [ecx + 8]
017E:    DC CB                   fmul     st(3), st(0)
0180:    DC CA                   fmul     st(2), st(0)
0182:    DE C9                   fmulp    st(1)
0184:    DC 47 A8                fadd     qword ptr [edi - 0x58]
0187:    DD 19                   fstp     qword ptr [ecx]
0189:    DC 47 A0                fadd     qword ptr [edi - 0x60]
018C:    DD 1A                   fstp     qword ptr [edx]
018E:    DC 47 98                fadd     qword ptr [edi - 0x68]
0191:    DD 1B                   fstp     qword ptr [ebx]
0193:    90                      nop
0194:    89 D8                   mov      eax, ebx
0196:    5F                      pop      edi
0197:    5E                      pop      esi
0198:    5B                      pop      ebx
0199:    8B E5                   mov      esp, ebp
019B:    5D                      pop      ebp
019C:    C2 08 00                ret      8
