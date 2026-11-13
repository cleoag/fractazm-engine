; Disassembly: Sierpinski3alt.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 219 bytes, decoded: 219 bytes (100.0%)
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
0013:    90                      nop
0014:    D9 E0                   fchs    ; -ST(0)
0016:    D8 D1                   fcom     st(1)
0018:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
001A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
001C:    72 02                   jb       0x20    ; Conditional jump
001E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0020:    D8 D2                   fcom     st(2)
0022:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0024:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0026:    72 02                   jb       0x2a    ; Conditional jump
0028:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
002A:    D9 E0                   fchs    ; -ST(0)
002C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002E:    D9 E0                   fchs    ; -ST(0)
0030:    D8 D2                   fcom     st(2)
0032:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0034:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0036:    72 02                   jb       0x3a    ; Conditional jump
0038:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003A:    D9 E0                   fchs    ; -ST(0)
003C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
003E:    90                      nop
003F:    DD 46 D0                fld      qword ptr [esi - 0x30]
0042:    DC C1                   fadd     st(1), st(0)
0044:    DD D8                   fstp     st(0)
0046:    DD 46 C8                fld      qword ptr [esi - 0x38]
0049:    DC C2                   fadd     st(2), st(0)
004B:    DD D8                   fstp     st(0)
004D:    DD 46 C0                fld      qword ptr [esi - 0x40]
0050:    DC C3                   fadd     st(3), st(0)
0052:    DD D8                   fstp     st(0)
0054:    90                      nop
0055:    D9 C0                   fld      st(0)
0057:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
005A:    D9 C2                   fld      st(2)
005C:    D8 4E A0                fmul     dword ptr [esi - 0x60]
005F:    DE C1                   faddp    st(1)
0061:    D9 C3                   fld      st(3)
0063:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
0066:    DE C1                   faddp    st(1)
0068:    D9 C1                   fld      st(1)
006A:    D8 4E BC                fmul     dword ptr [esi - 0x44]
006D:    D9 C3                   fld      st(3)
006F:    D8 4E B8                fmul     dword ptr [esi - 0x48]
0072:    DE C1                   faddp    st(1)
0074:    D9 C4                   fld      st(4)
0076:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
0079:    DE C1                   faddp    st(1)
007B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007D:    D8 4E B0                fmul     dword ptr [esi - 0x50]
0080:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0082:    D8 4E AC                fmul     dword ptr [esi - 0x54]
0085:    DE C3                   faddp    st(3)
0087:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0089:    D8 4E A8                fmul     dword ptr [esi - 0x58]
008C:    DE C2                   faddp    st(2)
008E:    90                      nop
008F:    DD 46 D0                fld      qword ptr [esi - 0x30]
0092:    DC E9                   fsub     st(1), st(0)
0094:    DD D8                   fstp     st(0)
0096:    DD 46 C8                fld      qword ptr [esi - 0x38]
0099:    DC EA                   fsub     st(2), st(0)
009B:    DD D8                   fstp     st(0)
009D:    DD 46 C0                fld      qword ptr [esi - 0x40]
00A0:    DC EB                   fsub     st(3), st(0)
00A2:    DD D8                   fstp     st(0)
00A4:    90                      nop
00A5:    DD 46 F0                fld      qword ptr [esi - 0x10]
00A8:    DD 41 08                fld      qword ptr [ecx + 8]
00AB:    D8 C9                   fmul     st(1)
00AD:    DD 59 08                fstp     qword ptr [ecx + 8]
00B0:    DC CB                   fmul     st(3), st(0)
00B2:    DC CA                   fmul     st(2), st(0)
00B4:    DC C9                   fmul     st(1), st(0)
00B6:    D9 E8                   fld1    ; Load 1.0
00B8:    DE E9                   fsubp    st(1)
00BA:    D9 C0                   fld      st(0)
00BC:    DC 4E D8                fmul     qword ptr [esi - 0x28]
00BF:    DE EC                   fsubp    st(4)
00C1:    D9 C0                   fld      st(0)
00C3:    DC 4E E0                fmul     qword ptr [esi - 0x20]
00C6:    DE EB                   fsubp    st(3)
00C8:    DC 4E E8                fmul     qword ptr [esi - 0x18]
00CB:    DE E9                   fsubp    st(1)
00CD:    DD 1B                   fstp     qword ptr [ebx]
00CF:    DD 1A                   fstp     qword ptr [edx]
00D1:    DD 19                   fstp     qword ptr [ecx]
00D3:    89 D8                   mov      eax, ebx
00D5:    5E                      pop      esi
00D6:    5B                      pop      ebx
00D7:    5D                      pop      ebp
00D8:    C2 08 00                ret      8
