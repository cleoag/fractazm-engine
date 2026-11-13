; Disassembly: Menger4.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 327 bytes, decoded: 327 bytes (100.0%)
; DEoption: 6

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    57                      push     edi
0006:    89 C3                   mov      ebx, eax
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 41 08                fld      qword ptr [ecx + 8]
0011:    D9 E1                   fabs    ; |ST(0)|
0013:    DD 01                   fld      qword ptr [ecx]
0015:    D9 E1                   fabs    ; |ST(0)|
0017:    DD 02                   fld      qword ptr [edx]
0019:    D9 E1                   fabs    ; |ST(0)|
001B:    DD 03                   fld      qword ptr [ebx]
001D:    D9 E1                   fabs    ; |ST(0)|
001F:    D8 D1                   fcom     st(1)
0021:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0023:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0025:    73 02                   jae      0x29    ; Conditional jump
0027:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0029:    D8 D2                   fcom     st(2)
002B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
002D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
002F:    73 02                   jae      0x33    ; Conditional jump
0031:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0033:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0035:    D8 D2                   fcom     st(2)
0037:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0039:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
003B:    73 02                   jae      0x3f    ; Conditional jump
003D:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003F:    90                      nop
0040:    90                      nop
0041:    90                      nop
0042:    D8 D3                   fcom     st(3)
0044:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0046:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0048:    73 02                   jae      0x4c    ; Conditional jump
004A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004C:    90                      nop
004D:    90                      nop
004E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0050:    90                      nop
0051:    90                      nop
0052:    D8 D3                   fcom     st(3)
0054:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0056:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0058:    73 02                   jae      0x5c    ; Conditional jump
005A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
005C:    90                      nop
005D:    90                      nop
005E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0060:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0062:    90                      nop
0063:    90                      nop
0064:    D8 D3                   fcom     st(3)
0066:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0068:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006A:    73 02                   jae      0x6e    ; Conditional jump
006C:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
006E:    90                      nop
006F:    90                      nop
0070:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0072:    83 EF 28                sub      edi, 0x28
0075:    D9 C0                   fld      st(0)
0077:    D8 4F F4                fmul     dword ptr [edi - 0xc]
007A:    D9 C2                   fld      st(2)
007C:    D8 4F F0                fmul     dword ptr [edi - 0x10]
007F:    DE C1                   faddp    st(1)
0081:    D9 C3                   fld      st(3)
0083:    D8 4F EC                fmul     dword ptr [edi - 0x14]
0086:    DE C1                   faddp    st(1)
0088:    D9 C4                   fld      st(4)
008A:    D8 4F E8                fmul     dword ptr [edi - 0x18]
008D:    DE C1                   faddp    st(1)
008F:    DD 1B                   fstp     qword ptr [ebx]
0091:    D9 C0                   fld      st(0)
0093:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
0096:    D9 C2                   fld      st(2)
0098:    D8 4F E0                fmul     dword ptr [edi - 0x20]
009B:    DE C1                   faddp    st(1)
009D:    D9 C3                   fld      st(3)
009F:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00A2:    DE C1                   faddp    st(1)
00A4:    D9 C4                   fld      st(4)
00A6:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00A9:    DE C1                   faddp    st(1)
00AB:    DD 1A                   fstp     qword ptr [edx]
00AD:    D9 C0                   fld      st(0)
00AF:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00B2:    D9 C2                   fld      st(2)
00B4:    D8 4F D0                fmul     dword ptr [edi - 0x30]
00B7:    DE C1                   faddp    st(1)
00B9:    D9 C3                   fld      st(3)
00BB:    D8 4F CC                fmul     dword ptr [edi - 0x34]
00BE:    DE C1                   faddp    st(1)
00C0:    D9 C4                   fld      st(4)
00C2:    D8 4F C8                fmul     dword ptr [edi - 0x38]
00C5:    DE C1                   faddp    st(1)
00C7:    DD 19                   fstp     qword ptr [ecx]
00C9:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
00CC:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00CE:    D8 4F C0                fmul     dword ptr [edi - 0x40]
00D1:    DE C1                   faddp    st(1)
00D3:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D5:    D8 4F BC                fmul     dword ptr [edi - 0x44]
00D8:    DE C1                   faddp    st(1)
00DA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00DC:    D8 4F B8                fmul     dword ptr [edi - 0x48]
00DF:    DE C1                   faddp    st(1)
00E1:    83 C7 28                add      edi, 0x28
00E4:    DD 47 F0                fld      qword ptr [edi - 0x10]
00E7:    DC C9                   fmul     st(1), st(0)
00E9:    D9 E8                   fld1    ; Load 1.0
00EB:    DE E9                   fsubp    st(1)
00ED:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00F0:    DE E9                   fsubp    st(1)
00F2:    DD 59 08                fstp     qword ptr [ecx + 8]
00F5:    DD 01                   fld      qword ptr [ecx]
00F7:    DD 02                   fld      qword ptr [edx]
00F9:    DD 03                   fld      qword ptr [ebx]
00FB:    DD 47 F0                fld      qword ptr [edi - 0x10]
00FE:    DD 86 E8 00 00 00       fld      qword ptr [esi + 0xe8]
0104:    D8 C9                   fmul     st(1)
0106:    DD 9E E8 00 00 00       fstp     qword ptr [esi + 0xe8]
010C:    90                      nop
010D:    90                      nop
010E:    DC CA                   fmul     st(2), st(0)
0110:    DC C9                   fmul     st(1), st(0)
0112:    D9 C0                   fld      st(0)
0114:    D9 E8                   fld1    ; Load 1.0
0116:    DE E9                   fsubp    st(1)
0118:    D9 C0                   fld      st(0)
011A:    D9 C0                   fld      st(0)
011C:    DC 4F E8                fmul     qword ptr [edi - 0x18]
011F:    DE ED                   fsubp    st(5)
0121:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0124:    DE EB                   fsubp    st(3)
0126:    D8 F1                   fdiv     st(1)
0128:    DC 4F D8                fmul     qword ptr [edi - 0x28]
012B:    DC EC                   fsub     st(4), st(0)
012D:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
012F:    D9 E1                   fabs    ; |ST(0)|
0131:    DE EC                   fsubp    st(4)
0133:    DE CB                   fmulp    st(3)
0135:    DD 1A                   fstp     qword ptr [edx]
0137:    DD 1B                   fstp     qword ptr [ebx]
0139:    DD 19                   fstp     qword ptr [ecx]
013B:    90                      nop
013C:    90                      nop
013D:    90                      nop
013E:    89 D8                   mov      eax, ebx
0140:    5F                      pop      edi
0141:    5E                      pop      esi
0142:    5B                      pop      ebx
0143:    5D                      pop      ebp
0144:    C2 08 00                ret      8
