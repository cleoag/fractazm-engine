; Disassembly: HalfOct4a.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 341 bytes, decoded: 341 bytes (100.0%)
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
0011:    DD 01                   fld      qword ptr [ecx]
0013:    DD 02                   fld      qword ptr [edx]
0015:    DD 03                   fld      qword ptr [ebx]
0017:    90                      nop
0018:    D9 E0                   fchs    ; -ST(0)
001A:    D8 D1                   fcom     st(1)
001C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
001E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0020:    72 02                   jb       0x24    ; Conditional jump
0022:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0024:    D9 E0                   fchs    ; -ST(0)
0026:    D8 D1                   fcom     st(1)
0028:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
002A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
002C:    73 02                   jae      0x30    ; Conditional jump
002E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0030:    90                      nop
0031:    D9 E0                   fchs    ; -ST(0)
0033:    D8 D2                   fcom     st(2)
0035:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0037:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0039:    72 02                   jb       0x3d    ; Conditional jump
003B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003D:    D9 E0                   fchs    ; -ST(0)
003F:    D8 D2                   fcom     st(2)
0041:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0043:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0045:    72 02                   jb       0x49    ; Conditional jump
0047:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0049:    90                      nop
004A:    90                      nop
004B:    90                      nop
004C:    D8 D3                   fcom     st(3)
004E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0050:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0052:    72 02                   jb       0x56    ; Conditional jump
0054:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0056:    90                      nop
0057:    90                      nop
0058:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
005A:    90                      nop
005B:    90                      nop
005C:    D8 D3                   fcom     st(3)
005E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0060:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0062:    72 02                   jb       0x66    ; Conditional jump
0064:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0066:    90                      nop
0067:    90                      nop
0068:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
006C:    90                      nop
006D:    90                      nop
006E:    D8 D3                   fcom     st(3)
0070:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0072:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0074:    73 02                   jae      0x78    ; Conditional jump
0076:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0078:    90                      nop
0079:    90                      nop
007A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007C:    90                      nop
007D:    83 EF 28                sub      edi, 0x28
0080:    D9 C0                   fld      st(0)
0082:    D8 4F F4                fmul     dword ptr [edi - 0xc]
0085:    D9 C2                   fld      st(2)
0087:    D8 4F F0                fmul     dword ptr [edi - 0x10]
008A:    DE C1                   faddp    st(1)
008C:    D9 C3                   fld      st(3)
008E:    D8 4F EC                fmul     dword ptr [edi - 0x14]
0091:    DE C1                   faddp    st(1)
0093:    D9 C4                   fld      st(4)
0095:    D8 4F E8                fmul     dword ptr [edi - 0x18]
0098:    DE C1                   faddp    st(1)
009A:    DD 1B                   fstp     qword ptr [ebx]
009C:    D9 C0                   fld      st(0)
009E:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
00A1:    D9 C2                   fld      st(2)
00A3:    D8 4F E0                fmul     dword ptr [edi - 0x20]
00A6:    DE C1                   faddp    st(1)
00A8:    D9 C3                   fld      st(3)
00AA:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00AD:    DE C1                   faddp    st(1)
00AF:    D9 C4                   fld      st(4)
00B1:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00B4:    DE C1                   faddp    st(1)
00B6:    DD 1A                   fstp     qword ptr [edx]
00B8:    D9 C0                   fld      st(0)
00BA:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00BD:    D9 C2                   fld      st(2)
00BF:    D8 4F D0                fmul     dword ptr [edi - 0x30]
00C2:    DE C1                   faddp    st(1)
00C4:    D9 C3                   fld      st(3)
00C6:    D8 4F CC                fmul     dword ptr [edi - 0x34]
00C9:    DE C1                   faddp    st(1)
00CB:    D9 C4                   fld      st(4)
00CD:    D8 4F C8                fmul     dword ptr [edi - 0x38]
00D0:    DE C1                   faddp    st(1)
00D2:    DD 19                   fstp     qword ptr [ecx]
00D4:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
00D7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D9:    D8 4F C0                fmul     dword ptr [edi - 0x40]
00DC:    DE C1                   faddp    st(1)
00DE:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E0:    D8 4F BC                fmul     dword ptr [edi - 0x44]
00E3:    DE C1                   faddp    st(1)
00E5:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E7:    D8 4F B8                fmul     dword ptr [edi - 0x48]
00EA:    DE C1                   faddp    st(1)
00EC:    DD 59 08                fstp     qword ptr [ecx + 8]
00EF:    83 C7 28                add      edi, 0x28
00F2:    DD 41 08                fld      qword ptr [ecx + 8]
00F5:    DD 01                   fld      qword ptr [ecx]
00F7:    DD 02                   fld      qword ptr [edx]
00F9:    DD 03                   fld      qword ptr [ebx]
00FB:    DD 47 F0                fld      qword ptr [edi - 0x10]
00FE:    90                      nop
00FF:    90                      nop
0100:    90                      nop
0101:    D9 C0                   fld      st(0)
0103:    DC 8E E8 00 00 00       fmul     qword ptr [esi + 0xe8]
0109:    DD 9E E8 00 00 00       fstp     qword ptr [esi + 0xe8]
010F:    DC CC                   fmul     st(4), st(0)
0111:    DC CB                   fmul     st(3), st(0)
0113:    DC CA                   fmul     st(2), st(0)
0115:    DC C9                   fmul     st(1), st(0)
0117:    D9 E8                   fld1    ; Load 1.0
0119:    DE E9                   fsubp    st(1)
011B:    D9 E0                   fchs    ; -ST(0)
011D:    DD 47 E8                fld      qword ptr [edi - 0x18]
0120:    D8 C9                   fmul     st(1)
0122:    D8 C2                   fadd     st(2)
0124:    DD 1B                   fstp     qword ptr [ebx]
0126:    DD 47 E0                fld      qword ptr [edi - 0x20]
0129:    D8 C9                   fmul     st(1)
012B:    D8 C3                   fadd     st(3)
012D:    DD 1A                   fstp     qword ptr [edx]
012F:    DD 47 D8                fld      qword ptr [edi - 0x28]
0132:    D8 C9                   fmul     st(1)
0134:    D8 C4                   fadd     st(4)
0136:    DD 19                   fstp     qword ptr [ecx]
0138:    DD 47 D0                fld      qword ptr [edi - 0x30]
013B:    D8 C9                   fmul     st(1)
013D:    D8 C5                   fadd     st(5)
013F:    DD 59 08                fstp     qword ptr [ecx + 8]
0142:    DD D8                   fstp     st(0)
0144:    DD D8                   fstp     st(0)
0146:    DD D8                   fstp     st(0)
0148:    DD D8                   fstp     st(0)
014A:    DD D8                   fstp     st(0)
014C:    89 D8                   mov      eax, ebx
014E:    5F                      pop      edi
014F:    5E                      pop      esi
0150:    5B                      pop      ebx
0151:    5D                      pop      ebp
0152:    C2 08 00                ret      8
