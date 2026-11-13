; Disassembly: SphCageIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 355 bytes, decoded: 355 bytes (100.0%)
; DEoption: 20

0000:    83 C4 C4                add      esp, -0x3c
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
004B:    8B 57 B0                mov      edx, dword ptr [edi - 0x50]
004E:    83 FA 02                cmp      edx, 2
0051:    7D 05                   jge      0x58    ; Conditional jump
0053:    BA 02 00 00 00          mov      edx, 2
0058:    81 FA E8 03 00 00       cmp      edx, 0x3e8
005E:    7E 05                   jle      0x65    ; Conditional jump
0060:    BA E8 03 00 00          mov      edx, 0x3e8
0065:    89 54 24 34             mov      dword ptr [esp + 0x34], edx
0069:    DD 47 F0                fld      qword ptr [edi - 0x10]
006C:    D9 C0                   fld      st(0)
006E:    D9 E0                   fchs    ; -ST(0)
0070:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0074:    DA 74 24 34             fidiv    dword ptr [esp + 0x34]
0078:    DC 4F 18                fmul     qword ptr [edi + 0x18]
007B:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
007F:    D1 EA                   shr      edx, 1
0081:    DD 04 24                fld      qword ptr [esp]
0084:    DD 44 24 08             fld      qword ptr [esp + 8]
0088:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
008A:    DC 6F 10                fsubr    qword ptr [edi + 0x10]
008D:    DC 0F                   fmul     qword ptr [edi]
008F:    DA 4F AC                fimul    dword ptr [edi - 0x54]
0092:    D9 FC                   frndint    ; round(ST(0))
0094:    DA 77 AC                fidiv    dword ptr [edi - 0x54]
0097:    DC 4F 08                fmul     qword ptr [edi + 8]
009A:    D9 FB                   fsincos    ; ST(0)=cos, push sin
009C:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
00A0:    DD 5C 24 30             fstp     qword ptr [esp + 0x30]
00A4:    DD 04 24                fld      qword ptr [esp]
00A7:    DC 4C 24 28             fmul     qword ptr [esp + 0x28]
00AB:    DD 44 24 08             fld      qword ptr [esp + 8]
00AF:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
00B3:    DE C1                   faddp    st(1)
00B5:    D8 C8                   fmul     st(0)
00B7:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00BB:    D8 C8                   fmul     st(0)
00BD:    DE C1                   faddp    st(1)
00BF:    D9 FA                   fsqrt    ; sqrt(ST(0))
00C1:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00C4:    D8 C8                   fmul     st(0)
00C6:    DD 04 24                fld      qword ptr [esp]
00C9:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
00CD:    DD 44 24 08             fld      qword ptr [esp + 8]
00D1:    DC 4C 24 28             fmul     qword ptr [esp + 0x28]
00D5:    D9 E0                   fchs    ; -ST(0)
00D7:    DE C1                   faddp    st(1)
00D9:    D8 C8                   fmul     st(0)
00DB:    DE C1                   faddp    st(1)
00DD:    D9 FA                   fsqrt    ; sqrt(ST(0))
00DF:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00E2:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00E6:    D9 E1                   fabs    ; |ST(0)|
00E8:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00EB:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00EF:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00F2:    4A                      dec      edx
00F3:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00F7:    D9 C0                   fld      st(0)
00F9:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
00FD:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0101:    DC 44 24 20             fadd     qword ptr [esp + 0x20]
0105:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0109:    DD 04 24                fld      qword ptr [esp]
010C:    D8 C8                   fmul     st(0)
010E:    DD 44 24 08             fld      qword ptr [esp + 8]
0112:    D8 C8                   fmul     st(0)
0114:    DE C1                   faddp    st(1)
0116:    D9 FA                   fsqrt    ; sqrt(ST(0))
0118:    DD 47 F0                fld      qword ptr [edi - 0x10]
011B:    D8 C8                   fmul     st(0)
011D:    DD 44 24 20             fld      qword ptr [esp + 0x20]
0121:    D8 C8                   fmul     st(0)
0123:    D9 E0                   fchs    ; -ST(0)
0125:    DE C1                   faddp    st(1)
0127:    D9 FA                   fsqrt    ; sqrt(ST(0))
0129:    D9 E0                   fchs    ; -ST(0)
012B:    DE C1                   faddp    st(1)
012D:    D8 C8                   fmul     st(0)
012F:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0133:    D8 C8                   fmul     st(0)
0135:    DE C1                   faddp    st(1)
0137:    D9 FA                   fsqrt    ; sqrt(ST(0))
0139:    DC 67 B8                fsub     qword ptr [edi - 0x48]
013C:    DD 46 E0                fld      qword ptr [esi - 0x20]
013F:    D8 D1                   fcom     st(1)
0141:    9B                      wait
0142:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0144:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0146:    73 04                   jae      0x14c    ; Conditional jump
0148:    DD D9                   fstp     st(1)
014A:    EB 02                   jmp      0x14e
014C:    DD D8                   fstp     st(0)
014E:    83 FA 00                cmp      edx, 0
0151:    7F 9C                   jg       0xef    ; Conditional jump
0153:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0157:    75 03                   jne      0x15c    ; Conditional jump
0159:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
015C:    DD 5E E0                fstp     qword ptr [esi - 0x20]
015F:    83 C4 3C                add      esp, 0x3c
0162:    C3                      ret
