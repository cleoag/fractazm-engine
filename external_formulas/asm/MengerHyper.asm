; Disassembly: MengerHyper.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 406 bytes, decoded: 406 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    89 C3                   mov      ebx, eax
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 01                   fld      qword ptr [ecx]
000F:    D9 E1                   fabs    ; |ST(0)|
0011:    DD 02                   fld      qword ptr [edx]
0013:    D9 E1                   fabs    ; |ST(0)|
0015:    DD 03                   fld      qword ptr [ebx]
0017:    D9 E1                   fabs    ; |ST(0)|
0019:    D9 C0                   fld      st(0)
001B:    D8 C8                   fmul     st(0)
001D:    D9 C2                   fld      st(2)
001F:    D8 C8                   fmul     st(0)
0021:    DE C1                   faddp    st(1)
0023:    D9 C3                   fld      st(3)
0025:    D8 C8                   fmul     st(0)
0027:    DE C1                   faddp    st(1)
0029:    90                      nop
002A:    DD 46 B8                fld      qword ptr [esi - 0x48]
002D:    DC 4E A8                fmul     qword ptr [esi - 0x58]
0030:    D8 D9                   fcomp    st(1)
0032:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0034:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0036:    73 0D                   jae      0x45    ; Conditional jump
0038:    90                      nop
0039:    90                      nop
003A:    90                      nop
003B:    90                      nop
003C:    DD D8                   fstp     st(0)
003E:    D9 E8                   fld1    ; Load 1.0
0040:    EB 1F                   jmp      0x61
0042:    90                      nop
0043:    90                      nop
0044:    90                      nop
0045:    DD 46 B0                fld      qword ptr [esi - 0x50]
0048:    DC 4E A8                fmul     qword ptr [esi - 0x58]
004B:    D8 D9                   fcomp    st(1)
004D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004F:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0051:    76 06                   jbe      0x59    ; Conditional jump
0053:    E8 17 00 00 00          call     0x6f
0058:    90                      nop
0059:    DD 46 B8                fld      qword ptr [esi - 0x48]
005C:    DC 4E A8                fmul     qword ptr [esi - 0x58]
005F:    DE F1                   fdivrp   st(1)
0061:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0063:    D8 C9                   fmul     st(1)
0065:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0067:    D8 C9                   fmul     st(1)
0069:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
006B:    D8 C9                   fmul     st(1)
006D:    EB 09                   jmp      0x78
006F:    DD D8                   fstp     st(0)
0071:    DD 46 B0                fld      qword ptr [esi - 0x50]
0074:    DC 4E A8                fmul     qword ptr [esi - 0x58]
0077:    C3                      ret
0078:    DD D9                   fstp     st(1)
007A:    D8 D1                   fcom     st(1)
007C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
007E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0080:    73 02                   jae      0x84    ; Conditional jump
0082:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0084:    D8 D2                   fcom     st(2)
0086:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0088:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
008A:    73 02                   jae      0x8e    ; Conditional jump
008C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
008E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0090:    D8 D2                   fcom     st(2)
0092:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0094:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0096:    73 02                   jae      0x9a    ; Conditional jump
0098:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
009A:    90                      nop
009B:    DC 46 C8                fadd     qword ptr [esi - 0x38]
009E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00A0:    DC 46 D0                fadd     qword ptr [esi - 0x30]
00A3:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00A5:    DC 46 C0                fadd     qword ptr [esi - 0x40]
00A8:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00AA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00AC:    D9 C0                   fld      st(0)
00AE:    D8 C8                   fmul     st(0)
00B0:    D9 C2                   fld      st(2)
00B2:    D8 C8                   fmul     st(0)
00B4:    DE C1                   faddp    st(1)
00B6:    D9 C3                   fld      st(3)
00B8:    D8 C8                   fmul     st(0)
00BA:    DE C1                   faddp    st(1)
00BC:    90                      nop
00BD:    DC 56 B8                fcom     qword ptr [esi - 0x48]
00C0:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C2:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00C4:    0F 87 AA 00 00 00       ja       0x174    ; Conditional jump
00CA:    DC 56 B0                fcom     qword ptr [esi - 0x50]
00CD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CF:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D1:    77 09                   ja       0xdc    ; Conditional jump
00D3:    90                      nop
00D4:    90                      nop
00D5:    90                      nop
00D6:    90                      nop
00D7:    E9 A2 00 00 00          jmp      0x17e
00DC:    DD 46 B8                fld      qword ptr [esi - 0x48]
00DF:    DE F1                   fdivrp   st(1)
00E1:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E3:    D8 C9                   fmul     st(1)
00E5:    DC 66 C8                fsub     qword ptr [esi - 0x38]
00E8:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00EA:    D8 C9                   fmul     st(1)
00EC:    DC 66 D0                fsub     qword ptr [esi - 0x30]
00EF:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00F1:    D8 C9                   fmul     st(1)
00F3:    DC 66 C0                fsub     qword ptr [esi - 0x40]
00F6:    83 7E A4 00             cmp      dword ptr [esi - 0x5c], 0
00FA:    0F 84 8B 00 00 00       je       0x18b    ; Conditional jump
0100:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0102:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0104:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0106:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0108:    90                      nop
0109:    D8 D1                   fcom     st(1)
010B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
010D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
010F:    73 02                   jae      0x113    ; Conditional jump
0111:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0113:    D8 D2                   fcom     st(2)
0115:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0117:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0119:    73 02                   jae      0x11d    ; Conditional jump
011B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
011D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
011F:    D8 D2                   fcom     st(2)
0121:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0123:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0125:    73 02                   jae      0x129    ; Conditional jump
0127:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0129:    90                      nop
012A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
012C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
012E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0130:    DD D8                   fstp     st(0)
0132:    DD 46 F0                fld      qword ptr [esi - 0x10]
0135:    D9 C0                   fld      st(0)
0137:    DC 49 08                fmul     qword ptr [ecx + 8]
013A:    DD 59 08                fstp     qword ptr [ecx + 8]
013D:    90                      nop
013E:    90                      nop
013F:    DC CA                   fmul     st(2), st(0)
0141:    DC C9                   fmul     st(1), st(0)
0143:    D9 C0                   fld      st(0)
0145:    D9 E8                   fld1    ; Load 1.0
0147:    DE E9                   fsubp    st(1)
0149:    D9 C0                   fld      st(0)
014B:    D9 C0                   fld      st(0)
014D:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0150:    DE ED                   fsubp    st(5)
0152:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0155:    DE EB                   fsubp    st(3)
0157:    D8 F1                   fdiv     st(1)
0159:    DC 4E D8                fmul     qword ptr [esi - 0x28]
015C:    DC EC                   fsub     st(4), st(0)
015E:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
0160:    D9 E1                   fabs    ; |ST(0)|
0162:    DE EC                   fsubp    st(4)
0164:    DE CB                   fmulp    st(3)
0166:    DD 1A                   fstp     qword ptr [edx]
0168:    DD 1B                   fstp     qword ptr [ebx]
016A:    DD 19                   fstp     qword ptr [ecx]
016C:    89 D8                   mov      eax, ebx
016E:    5E                      pop      esi
016F:    5B                      pop      ebx
0170:    5D                      pop      ebp
0171:    C2 08 00                ret      8
0174:    DD D8                   fstp     st(0)
0176:    D9 E8                   fld1    ; Load 1.0
0178:    E9 64 FF FF FF          jmp      0xe1
017D:    90                      nop
017E:    DD D8                   fstp     st(0)
0180:    90                      nop
0181:    90                      nop
0182:    90                      nop
0183:    DD 46 B0                fld      qword ptr [esi - 0x50]
0186:    E9 51 FF FF FF          jmp      0xdc
018B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
018D:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
018F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0191:    E9 9A FF FF FF          jmp      0x130
