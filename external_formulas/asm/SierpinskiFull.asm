; Disassembly: SierpinskiFull.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 407 bytes, decoded: 407 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    89 C3                   mov      ebx, eax
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 01                   fld      qword ptr [ecx]
000F:    DD 02                   fld      qword ptr [edx]
0011:    DD 03                   fld      qword ptr [ebx]
0013:    D9 C0                   fld      st(0)
0015:    D8 4E BC                fmul     dword ptr [esi - 0x44]
0018:    D9 C2                   fld      st(2)
001A:    D8 4E B8                fmul     dword ptr [esi - 0x48]
001D:    DE C1                   faddp    st(1)
001F:    D9 C3                   fld      st(3)
0021:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
0024:    DE C1                   faddp    st(1)
0026:    D9 C1                   fld      st(1)
0028:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
002B:    D9 C3                   fld      st(3)
002D:    D8 4E D0                fmul     dword ptr [esi - 0x30]
0030:    DE C1                   faddp    st(1)
0032:    D9 C4                   fld      st(4)
0034:    D8 4E CC                fmul     dword ptr [esi - 0x34]
0037:    DE C1                   faddp    st(1)
0039:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003B:    D8 4E C8                fmul     dword ptr [esi - 0x38]
003E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0040:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
0043:    DE C3                   faddp    st(3)
0045:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0047:    D8 4E C0                fmul     dword ptr [esi - 0x40]
004A:    DE C2                   faddp    st(2)
004C:    83 BE 7C FF FF FF 00    cmp      dword ptr [esi - 0x84], 0
0053:    0F 85 4D 00 00 00       jne      0xa6    ; Conditional jump
0059:    90                      nop
005A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
005C:    D8 D1                   fcom     st(1)
005E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0060:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0062:    73 02                   jae      0x66    ; Conditional jump
0064:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0066:    D8 D2                   fcom     st(2)
0068:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006C:    73 02                   jae      0x70    ; Conditional jump
006E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0070:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0072:    D8 D2                   fcom     st(2)
0074:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0076:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0078:    73 02                   jae      0x7c    ; Conditional jump
007A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007C:    D9 E0                   fchs    ; -ST(0)
007E:    D8 D1                   fcom     st(1)
0080:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0082:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0084:    72 02                   jb       0x88    ; Conditional jump
0086:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0088:    D8 D2                   fcom     st(2)
008A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
008C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
008E:    72 02                   jb       0x92    ; Conditional jump
0090:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0092:    D9 E0                   fchs    ; -ST(0)
0094:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0096:    D9 E0                   fchs    ; -ST(0)
0098:    D8 D2                   fcom     st(2)
009A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
009E:    72 02                   jb       0xa2    ; Conditional jump
00A0:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00A2:    D9 E0                   fchs    ; -ST(0)
00A4:    EB 4B                   jmp      0xf1
00A6:    D9 E0                   fchs    ; -ST(0)
00A8:    D8 D1                   fcom     st(1)
00AA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00AE:    72 02                   jb       0xb2    ; Conditional jump
00B0:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00B2:    D8 D2                   fcom     st(2)
00B4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00B8:    72 02                   jb       0xbc    ; Conditional jump
00BA:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00BC:    D9 E0                   fchs    ; -ST(0)
00BE:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00C0:    D9 E0                   fchs    ; -ST(0)
00C2:    D8 D2                   fcom     st(2)
00C4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00C8:    72 02                   jb       0xcc    ; Conditional jump
00CA:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00CC:    D9 E0                   fchs    ; -ST(0)
00CE:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D0:    D8 D1                   fcom     st(1)
00D2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00D4:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D6:    73 02                   jae      0xda    ; Conditional jump
00D8:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00DA:    D8 D2                   fcom     st(2)
00DC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DE:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00E0:    73 02                   jae      0xe4    ; Conditional jump
00E2:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00E4:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E6:    D8 D2                   fcom     st(2)
00E8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00EA:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00EC:    73 02                   jae      0xf0    ; Conditional jump
00EE:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00F0:    90                      nop
00F1:    DD 46 88                fld      qword ptr [esi - 0x78]
00F4:    D9 EE                   fldz    ; Load 0.0
00F6:    D8 D9                   fcomp    st(1)
00F8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00FA:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00FC:    73 0B                   jae      0x109    ; Conditional jump
00FE:    D8 E2                   fsub     st(2)
0100:    D9 E1                   fabs    ; |ST(0)|
0102:    D9 E0                   fchs    ; -ST(0)
0104:    DC 46 88                fadd     qword ptr [esi - 0x78]
0107:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0109:    DD D8                   fstp     st(0)
010B:    90                      nop
010C:    90                      nop
010D:    DD 46 80                fld      qword ptr [esi - 0x80]
0110:    D9 EE                   fldz    ; Load 0.0
0112:    D8 D9                   fcomp    st(1)
0114:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0116:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0118:    73 0B                   jae      0x125    ; Conditional jump
011A:    D8 E1                   fsub     st(1)
011C:    D9 E1                   fabs    ; |ST(0)|
011E:    D9 E0                   fchs    ; -ST(0)
0120:    DC 46 80                fadd     qword ptr [esi - 0x80]
0123:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0125:    DD D8                   fstp     st(0)
0127:    90                      nop
0128:    D9 C0                   fld      st(0)
012A:    D8 4E 98                fmul     dword ptr [esi - 0x68]
012D:    D9 C2                   fld      st(2)
012F:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
0132:    DE C1                   faddp    st(1)
0134:    D9 C3                   fld      st(3)
0136:    D8 4E 90                fmul     dword ptr [esi - 0x70]
0139:    DE C1                   faddp    st(1)
013B:    D9 C1                   fld      st(1)
013D:    D8 4E B0                fmul     dword ptr [esi - 0x50]
0140:    D9 C3                   fld      st(3)
0142:    D8 4E AC                fmul     dword ptr [esi - 0x54]
0145:    DE C1                   faddp    st(1)
0147:    D9 C4                   fld      st(4)
0149:    D8 4E A8                fmul     dword ptr [esi - 0x58]
014C:    DE C1                   faddp    st(1)
014E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0150:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
0153:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0155:    D8 4E A0                fmul     dword ptr [esi - 0x60]
0158:    DE C3                   faddp    st(3)
015A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
015C:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
015F:    DE C2                   faddp    st(2)
0161:    DD 46 F0                fld      qword ptr [esi - 0x10]
0164:    DD 41 08                fld      qword ptr [ecx + 8]
0167:    D8 C9                   fmul     st(1)
0169:    DD 59 08                fstp     qword ptr [ecx + 8]
016C:    DC CB                   fmul     st(3), st(0)
016E:    DC CA                   fmul     st(2), st(0)
0170:    DC C9                   fmul     st(1), st(0)
0172:    D9 E8                   fld1    ; Load 1.0
0174:    DE E9                   fsubp    st(1)
0176:    D9 C0                   fld      st(0)
0178:    DC 4E D8                fmul     qword ptr [esi - 0x28]
017B:    DE EC                   fsubp    st(4)
017D:    D9 C0                   fld      st(0)
017F:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0182:    DE EB                   fsubp    st(3)
0184:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0187:    DE E9                   fsubp    st(1)
0189:    DD 1B                   fstp     qword ptr [ebx]
018B:    DD 1A                   fstp     qword ptr [edx]
018D:    DD 19                   fstp     qword ptr [ecx]
018F:    89 D8                   mov      eax, ebx
0191:    5E                      pop      esi
0192:    5B                      pop      ebx
0193:    5D                      pop      ebp
0194:    C2 08 00                ret      8
