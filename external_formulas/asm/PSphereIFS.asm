; Disassembly: PSphereIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 274 bytes, decoded: 274 bytes (100.0%)
; DEoption: 20

0000:    83 C4 BA                add      esp, -0x46
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
004B:    83 7F A0 01             cmp      dword ptr [edi - 0x60], 1
004F:    75 0E                   jne      0x5f    ; Conditional jump
0051:    DD 04 24                fld      qword ptr [esp]
0054:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0058:    DD 1C 24                fstp     qword ptr [esp]
005B:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
005F:    83 7F A0 FF             cmp      dword ptr [edi - 0x60], -1
0063:    75 10                   jne      0x75    ; Conditional jump
0065:    DD 44 24 08             fld      qword ptr [esp + 8]
0069:    DD 44 24 10             fld      qword ptr [esp + 0x10]
006D:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0071:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0075:    83 7F 9C 00             cmp      dword ptr [edi - 0x64], 0
0079:    74 0A                   je       0x85    ; Conditional jump
007B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
007F:    D9 E0                   fchs    ; -ST(0)
0081:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0085:    DD 04 24                fld      qword ptr [esp]
0088:    D8 C8                   fmul     st(0)
008A:    DD 44 24 08             fld      qword ptr [esp + 8]
008E:    D8 C8                   fmul     st(0)
0090:    DE C1                   faddp    st(1)
0092:    DD 54 24 08             fst      qword ptr [esp + 8]
0096:    D9 FA                   fsqrt    ; sqrt(ST(0))
0098:    DD 1C 24                fstp     qword ptr [esp]
009B:    DD 47 F0                fld      qword ptr [edi - 0x10]
009E:    D8 C8                   fmul     st(0)
00A0:    DC 64 24 08             fsub     qword ptr [esp + 8]
00A4:    D9 C0                   fld      st(0)
00A6:    D9 E1                   fabs    ; |ST(0)|
00A8:    DE C1                   faddp    st(1)
00AA:    D9 FA                   fsqrt    ; sqrt(ST(0))
00AC:    D9 C0                   fld      st(0)
00AE:    DC 47 F0                fadd     qword ptr [edi - 0x10]
00B1:    DC 34 24                fdiv     qword ptr [esp]
00B4:    D9 ED                   fldln2    ; Load ln(2)
00B6:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00B8:    D9 F1                   fyl2x
00BA:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00BD:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00BF:    D9 E0                   fchs    ; -ST(0)
00C1:    DE C1                   faddp    st(1)
00C3:    DC 64 24 10             fsub     qword ptr [esp + 0x10]
00C7:    D9 E1                   fabs    ; |ST(0)|
00C9:    DC 67 AC                fsub     qword ptr [edi - 0x54]
00CC:    DD 04 24                fld      qword ptr [esp]
00CF:    DC 0F                   fmul     qword ptr [edi]
00D1:    DC 47 08                fadd     qword ptr [edi + 8]
00D4:    D9 E1                   fabs    ; |ST(0)|
00D6:    DE C9                   fmulp    st(1)
00D8:    DD 04 24                fld      qword ptr [esp]
00DB:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00DE:    D8 D1                   fcom     st(1)
00E0:    9B                      wait
00E1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00E5:    73 04                   jae      0xeb    ; Conditional jump
00E7:    DD D8                   fstp     st(0)
00E9:    EB 02                   jmp      0xed
00EB:    DD D9                   fstp     st(1)
00ED:    DD 04 24                fld      qword ptr [esp]
00F0:    DC 6F A4                fsubr    qword ptr [edi - 0x5c]
00F3:    D8 D1                   fcom     st(1)
00F5:    9B                      wait
00F6:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F8:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00FA:    73 04                   jae      0x100    ; Conditional jump
00FC:    DD D8                   fstp     st(0)
00FE:    EB 02                   jmp      0x102
0100:    DD D9                   fstp     st(1)
0102:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0106:    75 03                   jne      0x10b    ; Conditional jump
0108:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
010B:    DD 5E E0                fstp     qword ptr [esi - 0x20]
010E:    83 C4 46                add      esp, 0x46
0111:    C3                      ret
