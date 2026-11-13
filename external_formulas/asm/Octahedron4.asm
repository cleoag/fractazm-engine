; Disassembly: Octahedron4.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 343 bytes, decoded: 343 bytes (100.0%)
; DEoption: 6

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    57                      push     edi
0006:    89 C3                   mov      ebx, eax
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    90                      nop
000F:    DD 41 08                fld      qword ptr [ecx + 8]
0012:    D9 E1                   fabs    ; |ST(0)|
0014:    DD 01                   fld      qword ptr [ecx]
0016:    D9 E1                   fabs    ; |ST(0)|
0018:    DD 02                   fld      qword ptr [edx]
001A:    D9 E1                   fabs    ; |ST(0)|
001C:    DD 03                   fld      qword ptr [ebx]
001E:    D9 E1                   fabs    ; |ST(0)|
0020:    D8 D1                   fcom     st(1)
0022:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0024:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0026:    73 02                   jae      0x2a    ; Conditional jump
0028:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002A:    D9 E0                   fchs    ; -ST(0)
002C:    D8 D1                   fcom     st(1)
002E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0030:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0032:    72 02                   jb       0x36    ; Conditional jump
0034:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0036:    D9 E0                   fchs    ; -ST(0)
0038:    D8 D2                   fcom     st(2)
003A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
003C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
003E:    73 02                   jae      0x42    ; Conditional jump
0040:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0042:    D9 E0                   fchs    ; -ST(0)
0044:    D8 D2                   fcom     st(2)
0046:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0048:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
004A:    72 02                   jb       0x4e    ; Conditional jump
004C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
004E:    D9 E0                   fchs    ; -ST(0)
0050:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0052:    D8 D3                   fcom     st(3)
0054:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0056:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0058:    73 02                   jae      0x5c    ; Conditional jump
005A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
005C:    90                      nop
005D:    90                      nop
005E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0060:    90                      nop
0061:    90                      nop
0062:    D8 D3                   fcom     st(3)
0064:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0066:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0068:    73 02                   jae      0x6c    ; Conditional jump
006A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
006C:    90                      nop
006D:    90                      nop
006E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0070:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0072:    90                      nop
0073:    90                      nop
0074:    D8 D3                   fcom     st(3)
0076:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0078:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
007A:    73 02                   jae      0x7e    ; Conditional jump
007C:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
007E:    90                      nop
007F:    83 EF 28                sub      edi, 0x28
0082:    D9 C0                   fld      st(0)
0084:    D8 4F F4                fmul     dword ptr [edi - 0xc]
0087:    D9 C2                   fld      st(2)
0089:    D8 4F F0                fmul     dword ptr [edi - 0x10]
008C:    DE C1                   faddp    st(1)
008E:    D9 C3                   fld      st(3)
0090:    D8 4F EC                fmul     dword ptr [edi - 0x14]
0093:    DE C1                   faddp    st(1)
0095:    D9 C4                   fld      st(4)
0097:    D8 4F E8                fmul     dword ptr [edi - 0x18]
009A:    DE C1                   faddp    st(1)
009C:    DD 1B                   fstp     qword ptr [ebx]
009E:    D9 C0                   fld      st(0)
00A0:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
00A3:    D9 C2                   fld      st(2)
00A5:    D8 4F E0                fmul     dword ptr [edi - 0x20]
00A8:    DE C1                   faddp    st(1)
00AA:    D9 C3                   fld      st(3)
00AC:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00AF:    DE C1                   faddp    st(1)
00B1:    D9 C4                   fld      st(4)
00B3:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00B6:    DE C1                   faddp    st(1)
00B8:    DD 1A                   fstp     qword ptr [edx]
00BA:    D9 C0                   fld      st(0)
00BC:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00BF:    D9 C2                   fld      st(2)
00C1:    D8 4F D0                fmul     dword ptr [edi - 0x30]
00C4:    DE C1                   faddp    st(1)
00C6:    D9 C3                   fld      st(3)
00C8:    D8 4F CC                fmul     dword ptr [edi - 0x34]
00CB:    DE C1                   faddp    st(1)
00CD:    D9 C4                   fld      st(4)
00CF:    D8 4F C8                fmul     dword ptr [edi - 0x38]
00D2:    DE C1                   faddp    st(1)
00D4:    DD 19                   fstp     qword ptr [ecx]
00D6:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
00D9:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00DB:    D8 4F C0                fmul     dword ptr [edi - 0x40]
00DE:    DE C1                   faddp    st(1)
00E0:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E2:    D8 4F BC                fmul     dword ptr [edi - 0x44]
00E5:    DE C1                   faddp    st(1)
00E7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E9:    D8 4F B8                fmul     dword ptr [edi - 0x48]
00EC:    DE C1                   faddp    st(1)
00EE:    DD 59 08                fstp     qword ptr [ecx + 8]
00F1:    83 C7 28                add      edi, 0x28
00F4:    DD 41 08                fld      qword ptr [ecx + 8]
00F7:    DD 01                   fld      qword ptr [ecx]
00F9:    DD 02                   fld      qword ptr [edx]
00FB:    DD 03                   fld      qword ptr [ebx]
00FD:    DD 47 F0                fld      qword ptr [edi - 0x10]
0100:    90                      nop
0101:    90                      nop
0102:    90                      nop
0103:    D9 C0                   fld      st(0)
0105:    DC 8E E8 00 00 00       fmul     qword ptr [esi + 0xe8]
010B:    DD 9E E8 00 00 00       fstp     qword ptr [esi + 0xe8]
0111:    DC CC                   fmul     st(4), st(0)
0113:    DC CB                   fmul     st(3), st(0)
0115:    DC CA                   fmul     st(2), st(0)
0117:    DC C9                   fmul     st(1), st(0)
0119:    D9 E8                   fld1    ; Load 1.0
011B:    DE E9                   fsubp    st(1)
011D:    D9 E0                   fchs    ; -ST(0)
011F:    DD 47 E8                fld      qword ptr [edi - 0x18]
0122:    D8 C9                   fmul     st(1)
0124:    D8 C2                   fadd     st(2)
0126:    DD 1B                   fstp     qword ptr [ebx]
0128:    DD 47 E0                fld      qword ptr [edi - 0x20]
012B:    D8 C9                   fmul     st(1)
012D:    D8 C3                   fadd     st(3)
012F:    DD 1A                   fstp     qword ptr [edx]
0131:    DD 47 D8                fld      qword ptr [edi - 0x28]
0134:    D8 C9                   fmul     st(1)
0136:    D8 C4                   fadd     st(4)
0138:    DD 19                   fstp     qword ptr [ecx]
013A:    DD 47 D0                fld      qword ptr [edi - 0x30]
013D:    D8 C9                   fmul     st(1)
013F:    D8 C5                   fadd     st(5)
0141:    DD 59 08                fstp     qword ptr [ecx + 8]
0144:    DD D8                   fstp     st(0)
0146:    DD D8                   fstp     st(0)
0148:    DD D8                   fstp     st(0)
014A:    DD D8                   fstp     st(0)
014C:    DD D8                   fstp     st(0)
014E:    89 D8                   mov      eax, ebx
0150:    5F                      pop      edi
0151:    5E                      pop      esi
0152:    5B                      pop      ebx
0153:    5D                      pop      ebp
0154:    C2 08 00                ret      8
