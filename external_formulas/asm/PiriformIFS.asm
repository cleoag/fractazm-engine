; Disassembly: PiriformIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 251 bytes, decoded: 251 bytes (100.0%)
; DEoption: 20

0000:    83 C4 DA                add      esp, -0x26
0003:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0008:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000D:    66 0F 59 47 E0          mulpd    xmm0, xmmword ptr [edi - 0x20]
0012:    F2 0F 59 4F E0          mulsd    xmm1, qword ptr [edi - 0x20]
0017:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
001C:    F2 0F 58 4F D0          addsd    xmm1, qword ptr [edi - 0x30]
0021:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0025:    74 19                   je       0x40    ; Conditional jump
0027:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002C:    F2 0F 59 6F E0          mulsd    xmm5, qword ptr [edi - 0x20]
0031:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0036:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
003B:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0040:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0045:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
004B:    83 7F A4 00             cmp      dword ptr [edi - 0x5c], 0
004F:    74 0E                   je       0x5f    ; Conditional jump
0051:    DD 04 24                fld      qword ptr [esp]
0054:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0058:    DD 1C 24                fstp     qword ptr [esp]
005B:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
005F:    DD 04 24                fld      qword ptr [esp]
0062:    83 7F A0 00             cmp      dword ptr [edi - 0x60], 0
0066:    74 04                   je       0x6c    ; Conditional jump
0068:    D9 E1                   fabs    ; |ST(0)|
006A:    EB 02                   jmp      0x6e
006C:    D9 E0                   fchs    ; -ST(0)
006E:    DD 14 24                fst      qword ptr [esp]
0071:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0075:    D8 C8                   fmul     st(0)
0077:    83 7F B0 00             cmp      dword ptr [edi - 0x50], 0
007B:    75 03                   jne      0x80    ; Conditional jump
007D:    DC 0C 24                fmul     qword ptr [esp]
0080:    83 7F B0 02             cmp      dword ptr [edi - 0x50], 2
0084:    74 03                   je       0x89    ; Conditional jump
0086:    DC 0C 24                fmul     qword ptr [esp]
0089:    D9 C0                   fld      st(0)
008B:    DC 0C 24                fmul     qword ptr [esp]
008E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0090:    DD 1C 24                fstp     qword ptr [esp]
0093:    DD 44 24 08             fld      qword ptr [esp + 8]
0097:    D8 C8                   fmul     st(0)
0099:    DE C1                   faddp    st(1)
009B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
009F:    D8 C8                   fmul     st(0)
00A1:    DE C1                   faddp    st(1)
00A3:    DC 47 B8                fadd     qword ptr [edi - 0x48]
00A6:    D9 C0                   fld      st(0)
00A8:    D9 E1                   fabs    ; |ST(0)|
00AA:    DE C1                   faddp    st(1)
00AC:    DC 0F                   fmul     qword ptr [edi]
00AE:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B0:    DD 04 24                fld      qword ptr [esp]
00B3:    DC 47 F0                fadd     qword ptr [edi - 0x10]
00B6:    D9 C0                   fld      st(0)
00B8:    D9 E1                   fabs    ; |ST(0)|
00BA:    DE C1                   faddp    st(1)
00BC:    DC 0F                   fmul     qword ptr [edi]
00BE:    D9 FA                   fsqrt    ; sqrt(ST(0))
00C0:    D9 E0                   fchs    ; -ST(0)
00C2:    DE C1                   faddp    st(1)
00C4:    DD 47 A8                fld      qword ptr [edi - 0x58]
00C7:    D9 EE                   fldz    ; Load 0.0
00C9:    D8 D9                   fcomp    st(1)
00CB:    9B                      wait
00CC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CE:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D0:    73 13                   jae      0xe5    ; Conditional jump
00D2:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00D6:    D9 E1                   fabs    ; |ST(0)|
00D8:    D8 E1                   fsub     st(1)
00DA:    DD D9                   fstp     st(1)
00DC:    D8 D1                   fcom     st(1)
00DE:    9B                      wait
00DF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E1:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00E3:    73 04                   jae      0xe9    ; Conditional jump
00E5:    DD D8                   fstp     st(0)
00E7:    EB 02                   jmp      0xeb
00E9:    DD D9                   fstp     st(1)
00EB:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00EF:    75 03                   jne      0xf4    ; Conditional jump
00F1:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00F4:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00F7:    83 C4 26                add      esp, 0x26
00FA:    C3                      ret
