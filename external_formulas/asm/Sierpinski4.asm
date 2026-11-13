; Disassembly: Sierpinski4.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 334 bytes, decoded: 334 bytes (100.0%)
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
0024:    D8 D2                   fcom     st(2)
0026:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0028:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
002A:    72 02                   jb       0x2e    ; Conditional jump
002C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
002E:    D9 E0                   fchs    ; -ST(0)
0030:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0032:    D9 E0                   fchs    ; -ST(0)
0034:    D8 D2                   fcom     st(2)
0036:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0038:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
003A:    72 02                   jb       0x3e    ; Conditional jump
003C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003E:    D9 E0                   fchs    ; -ST(0)
0040:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0042:    90                      nop
0043:    D9 E0                   fchs    ; -ST(0)
0045:    D8 D3                   fcom     st(3)
0047:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0049:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
004B:    72 02                   jb       0x4f    ; Conditional jump
004D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004F:    D9 E0                   fchs    ; -ST(0)
0051:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0053:    D9 E0                   fchs    ; -ST(0)
0055:    D8 D3                   fcom     st(3)
0057:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0059:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005B:    72 02                   jb       0x5f    ; Conditional jump
005D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
005F:    D9 E0                   fchs    ; -ST(0)
0061:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0063:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0065:    D9 E0                   fchs    ; -ST(0)
0067:    D8 D3                   fcom     st(3)
0069:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006D:    72 02                   jb       0x71    ; Conditional jump
006F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0071:    D9 E0                   fchs    ; -ST(0)
0073:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0075:    90                      nop
0076:    83 EF 28                sub      edi, 0x28
0079:    D9 C0                   fld      st(0)
007B:    D8 4F F4                fmul     dword ptr [edi - 0xc]
007E:    D9 C2                   fld      st(2)
0080:    D8 4F F0                fmul     dword ptr [edi - 0x10]
0083:    DE C1                   faddp    st(1)
0085:    D9 C3                   fld      st(3)
0087:    D8 4F EC                fmul     dword ptr [edi - 0x14]
008A:    DE C1                   faddp    st(1)
008C:    D9 C4                   fld      st(4)
008E:    D8 4F E8                fmul     dword ptr [edi - 0x18]
0091:    DE C1                   faddp    st(1)
0093:    DD 1B                   fstp     qword ptr [ebx]
0095:    D9 C0                   fld      st(0)
0097:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
009A:    D9 C2                   fld      st(2)
009C:    D8 4F E0                fmul     dword ptr [edi - 0x20]
009F:    DE C1                   faddp    st(1)
00A1:    D9 C3                   fld      st(3)
00A3:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00A6:    DE C1                   faddp    st(1)
00A8:    D9 C4                   fld      st(4)
00AA:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00AD:    DE C1                   faddp    st(1)
00AF:    DD 1A                   fstp     qword ptr [edx]
00B1:    D9 C0                   fld      st(0)
00B3:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00B6:    D9 C2                   fld      st(2)
00B8:    D8 4F D0                fmul     dword ptr [edi - 0x30]
00BB:    DE C1                   faddp    st(1)
00BD:    D9 C3                   fld      st(3)
00BF:    D8 4F CC                fmul     dword ptr [edi - 0x34]
00C2:    DE C1                   faddp    st(1)
00C4:    D9 C4                   fld      st(4)
00C6:    D8 4F C8                fmul     dword ptr [edi - 0x38]
00C9:    DE C1                   faddp    st(1)
00CB:    DD 19                   fstp     qword ptr [ecx]
00CD:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
00D0:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D2:    D8 4F C0                fmul     dword ptr [edi - 0x40]
00D5:    DE C1                   faddp    st(1)
00D7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D9:    D8 4F BC                fmul     dword ptr [edi - 0x44]
00DC:    DE C1                   faddp    st(1)
00DE:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E0:    D8 4F B8                fmul     dword ptr [edi - 0x48]
00E3:    DE C1                   faddp    st(1)
00E5:    DD 59 08                fstp     qword ptr [ecx + 8]
00E8:    83 C7 28                add      edi, 0x28
00EB:    DD 41 08                fld      qword ptr [ecx + 8]
00EE:    DD 01                   fld      qword ptr [ecx]
00F0:    DD 02                   fld      qword ptr [edx]
00F2:    DD 03                   fld      qword ptr [ebx]
00F4:    DD 47 F0                fld      qword ptr [edi - 0x10]
00F7:    90                      nop
00F8:    90                      nop
00F9:    90                      nop
00FA:    D9 C0                   fld      st(0)
00FC:    DC 8E E8 00 00 00       fmul     qword ptr [esi + 0xe8]
0102:    DD 9E E8 00 00 00       fstp     qword ptr [esi + 0xe8]
0108:    DC CC                   fmul     st(4), st(0)
010A:    DC CB                   fmul     st(3), st(0)
010C:    DC CA                   fmul     st(2), st(0)
010E:    DC C9                   fmul     st(1), st(0)
0110:    D9 E8                   fld1    ; Load 1.0
0112:    DE E9                   fsubp    st(1)
0114:    D9 E0                   fchs    ; -ST(0)
0116:    DD 47 E8                fld      qword ptr [edi - 0x18]
0119:    D8 C9                   fmul     st(1)
011B:    D8 C2                   fadd     st(2)
011D:    DD 1B                   fstp     qword ptr [ebx]
011F:    DD 47 E0                fld      qword ptr [edi - 0x20]
0122:    D8 C9                   fmul     st(1)
0124:    D8 C3                   fadd     st(3)
0126:    DD 1A                   fstp     qword ptr [edx]
0128:    DD 47 D8                fld      qword ptr [edi - 0x28]
012B:    D8 C9                   fmul     st(1)
012D:    D8 C4                   fadd     st(4)
012F:    DD 19                   fstp     qword ptr [ecx]
0131:    DD 47 D0                fld      qword ptr [edi - 0x30]
0134:    D8 C9                   fmul     st(1)
0136:    D8 C5                   fadd     st(5)
0138:    DD 59 08                fstp     qword ptr [ecx + 8]
013B:    DD D8                   fstp     st(0)
013D:    DD D8                   fstp     st(0)
013F:    DD D8                   fstp     st(0)
0141:    DD D8                   fstp     st(0)
0143:    DD D8                   fstp     st(0)
0145:    89 D8                   mov      eax, ebx
0147:    5F                      pop      edi
0148:    5E                      pop      esi
0149:    5B                      pop      ebx
014A:    5D                      pop      ebp
014B:    C2 08 00                ret      8
