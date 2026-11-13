; Disassembly: Makin3D-3-4.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 190 bytes, decoded: 190 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    57                      push     edi
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    8B D8                   mov      ebx, eax
000E:    DD 03                   fld      qword ptr [ebx]
0010:    D8 C8                   fmul     st(0)
0012:    DD 02                   fld      qword ptr [edx]
0014:    D8 C8                   fmul     st(0)
0016:    DE E9                   fsubp    st(1)
0018:    DD 01                   fld      qword ptr [ecx]
001A:    D8 C8                   fmul     st(0)
001C:    DE E9                   fsubp    st(1)
001E:    D9 D0                   fnop
0020:    DD 03                   fld      qword ptr [ebx]
0022:    DC 0A                   fmul     qword ptr [edx]
0024:    D8 C0                   fadd     st(0)
0026:    D9 E8                   fld1    ; Load 1.0
0028:    DD 01                   fld      qword ptr [ecx]
002A:    D8 C8                   fmul     st(0)
002C:    83 7F F4 00             cmp      dword ptr [edi - 0xc], 0
0030:    7E 02                   jle      0x34    ; Conditional jump
0032:    EB 12                   jmp      0x46
0034:    83 7F F4 00             cmp      dword ptr [edi - 0xc], 0
0038:    74 0E                   je       0x48    ; Conditional jump
003A:    DD 01                   fld      qword ptr [ecx]
003C:    D9 EE                   fldz    ; Load 0.0
003E:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0040:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0042:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0044:    72 02                   jb       0x48    ; Conditional jump
0046:    D9 E0                   fchs    ; -ST(0)
0048:    DD 03                   fld      qword ptr [ebx]
004A:    D8 C8                   fmul     st(0)
004C:    DD 02                   fld      qword ptr [edx]
004E:    D8 C8                   fmul     st(0)
0050:    DE C1                   faddp    st(1)
0052:    DD 47 E8                fld      qword ptr [edi - 0x18]
0055:    D9 E1                   fabs    ; |ST(0)|
0057:    DE C1                   faddp    st(1)
0059:    DE F9                   fdivp    st(1)
005B:    DE C1                   faddp    st(1)
005D:    DE C9                   fmulp    st(1)
005F:    D9 D0                   fnop
0061:    DD 03                   fld      qword ptr [ebx]
0063:    DC 09                   fmul     qword ptr [ecx]
0065:    D8 C0                   fadd     st(0)
0067:    D9 E8                   fld1    ; Load 1.0
0069:    DD 02                   fld      qword ptr [edx]
006B:    D8 C8                   fmul     st(0)
006D:    83 7F F0 00             cmp      dword ptr [edi - 0x10], 0
0071:    7E 02                   jle      0x75    ; Conditional jump
0073:    EB 12                   jmp      0x87
0075:    83 7F F0 00             cmp      dword ptr [edi - 0x10], 0
0079:    74 0E                   je       0x89    ; Conditional jump
007B:    DD 02                   fld      qword ptr [edx]
007D:    D9 EE                   fldz    ; Load 0.0
007F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0081:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0083:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0085:    72 02                   jb       0x89    ; Conditional jump
0087:    D9 E0                   fchs    ; -ST(0)
0089:    DD 03                   fld      qword ptr [ebx]
008B:    D8 C8                   fmul     st(0)
008D:    DD 01                   fld      qword ptr [ecx]
008F:    D8 C8                   fmul     st(0)
0091:    DE C1                   faddp    st(1)
0093:    DD 47 E8                fld      qword ptr [edi - 0x18]
0096:    D9 E1                   fabs    ; |ST(0)|
0098:    DE C1                   faddp    st(1)
009A:    DE F9                   fdivp    st(1)
009C:    DE C1                   faddp    st(1)
009E:    DE C9                   fmulp    st(1)
00A0:    D9 D0                   fnop
00A2:    DC 46 28                fadd     qword ptr [esi + 0x28]
00A5:    DD 19                   fstp     qword ptr [ecx]
00A7:    DC 46 20                fadd     qword ptr [esi + 0x20]
00AA:    DD 1A                   fstp     qword ptr [edx]
00AC:    DC 46 18                fadd     qword ptr [esi + 0x18]
00AF:    DD 1B                   fstp     qword ptr [ebx]
00B1:    D9 D0                   fnop
00B3:    8B C3                   mov      eax, ebx
00B5:    5F                      pop      edi
00B6:    5E                      pop      esi
00B7:    5B                      pop      ebx
00B8:    89 EC                   mov      esp, ebp
00BA:    5D                      pop      ebp
00BB:    C2 08 00                ret      8
