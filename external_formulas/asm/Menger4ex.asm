; Disassembly: Menger4ex.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 378 bytes, decoded: 378 bytes (100.0%)
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
0072:    90                      nop
0073:    DD 47 80                fld      qword ptr [edi - 0x80]
0076:    DE C1                   faddp    st(1)
0078:    DD 47 88                fld      qword ptr [edi - 0x78]
007B:    DE C2                   faddp    st(2)
007D:    DD 87 78 FF FF FF       fld      qword ptr [edi - 0x88]
0083:    DE C3                   faddp    st(3)
0085:    DD 87 70 FF FF FF       fld      qword ptr [edi - 0x90]
008B:    DE C4                   faddp    st(4)
008D:    90                      nop
008E:    83 EF 28                sub      edi, 0x28
0091:    D9 C0                   fld      st(0)
0093:    D8 4F F4                fmul     dword ptr [edi - 0xc]
0096:    D9 C2                   fld      st(2)
0098:    D8 4F F0                fmul     dword ptr [edi - 0x10]
009B:    DE C1                   faddp    st(1)
009D:    D9 C3                   fld      st(3)
009F:    D8 4F EC                fmul     dword ptr [edi - 0x14]
00A2:    DE C1                   faddp    st(1)
00A4:    D9 C4                   fld      st(4)
00A6:    D8 4F E8                fmul     dword ptr [edi - 0x18]
00A9:    DE C1                   faddp    st(1)
00AB:    DD 1B                   fstp     qword ptr [ebx]
00AD:    D9 C0                   fld      st(0)
00AF:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
00B2:    D9 C2                   fld      st(2)
00B4:    D8 4F E0                fmul     dword ptr [edi - 0x20]
00B7:    DE C1                   faddp    st(1)
00B9:    D9 C3                   fld      st(3)
00BB:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00BE:    DE C1                   faddp    st(1)
00C0:    D9 C4                   fld      st(4)
00C2:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00C5:    DE C1                   faddp    st(1)
00C7:    DD 1A                   fstp     qword ptr [edx]
00C9:    D9 C0                   fld      st(0)
00CB:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00CE:    D9 C2                   fld      st(2)
00D0:    D8 4F D0                fmul     dword ptr [edi - 0x30]
00D3:    DE C1                   faddp    st(1)
00D5:    D9 C3                   fld      st(3)
00D7:    D8 4F CC                fmul     dword ptr [edi - 0x34]
00DA:    DE C1                   faddp    st(1)
00DC:    D9 C4                   fld      st(4)
00DE:    D8 4F C8                fmul     dword ptr [edi - 0x38]
00E1:    DE C1                   faddp    st(1)
00E3:    DD 19                   fstp     qword ptr [ecx]
00E5:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
00E8:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00EA:    D8 4F C0                fmul     dword ptr [edi - 0x40]
00ED:    DE C1                   faddp    st(1)
00EF:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00F1:    D8 4F BC                fmul     dword ptr [edi - 0x44]
00F4:    DE C1                   faddp    st(1)
00F6:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00F8:    D8 4F B8                fmul     dword ptr [edi - 0x48]
00FB:    DE C1                   faddp    st(1)
00FD:    83 C7 28                add      edi, 0x28
0100:    90                      nop
0101:    90                      nop
0102:    DD 87 70 FF FF FF       fld      qword ptr [edi - 0x90]
0108:    DE E9                   fsubp    st(1)
010A:    DD 47 F0                fld      qword ptr [edi - 0x10]
010D:    DC C9                   fmul     st(1), st(0)
010F:    D9 E8                   fld1    ; Load 1.0
0111:    DE E9                   fsubp    st(1)
0113:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0116:    DE E9                   fsubp    st(1)
0118:    DD 59 08                fstp     qword ptr [ecx + 8]
011B:    DD 01                   fld      qword ptr [ecx]
011D:    DC A7 78 FF FF FF       fsub     qword ptr [edi - 0x88]
0123:    DD 02                   fld      qword ptr [edx]
0125:    DC 67 88                fsub     qword ptr [edi - 0x78]
0128:    DD 03                   fld      qword ptr [ebx]
012A:    DC 67 80                fsub     qword ptr [edi - 0x80]
012D:    90                      nop
012E:    DD 47 F0                fld      qword ptr [edi - 0x10]
0131:    DD 86 E8 00 00 00       fld      qword ptr [esi + 0xe8]
0137:    D8 C9                   fmul     st(1)
0139:    DD 9E E8 00 00 00       fstp     qword ptr [esi + 0xe8]
013F:    90                      nop
0140:    90                      nop
0141:    DC CA                   fmul     st(2), st(0)
0143:    DC C9                   fmul     st(1), st(0)
0145:    D9 C0                   fld      st(0)
0147:    D9 E8                   fld1    ; Load 1.0
0149:    DE E9                   fsubp    st(1)
014B:    D9 C0                   fld      st(0)
014D:    D9 C0                   fld      st(0)
014F:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0152:    DE ED                   fsubp    st(5)
0154:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0157:    DE EB                   fsubp    st(3)
0159:    D8 F1                   fdiv     st(1)
015B:    DC 4F D8                fmul     qword ptr [edi - 0x28]
015E:    DC EC                   fsub     st(4), st(0)
0160:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
0162:    D9 E1                   fabs    ; |ST(0)|
0164:    DE EC                   fsubp    st(4)
0166:    DE CB                   fmulp    st(3)
0168:    DD 1A                   fstp     qword ptr [edx]
016A:    DD 1B                   fstp     qword ptr [ebx]
016C:    DD 19                   fstp     qword ptr [ecx]
016E:    90                      nop
016F:    90                      nop
0170:    90                      nop
0171:    89 D8                   mov      eax, ebx
0173:    5F                      pop      edi
0174:    5E                      pop      esi
0175:    5B                      pop      ebx
0176:    5D                      pop      ebp
0177:    C2 08 00                ret      8
