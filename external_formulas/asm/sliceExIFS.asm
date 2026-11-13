; Disassembly: sliceExIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 334 bytes, decoded: 334 bytes (100.0%)
; DEoption: 20

0000:    83 C4 CE                add      esp, -0x32
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
004B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
004F:    DC 4F 9C                fmul     qword ptr [edi - 0x64]
0052:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0054:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0058:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
005C:    DD 04 24                fld      qword ptr [esp]
005F:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0063:    DD 44 24 08             fld      qword ptr [esp + 8]
0067:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
006B:    D9 E0                   fchs    ; -ST(0)
006D:    DE C1                   faddp    st(1)
006F:    DD 04 24                fld      qword ptr [esp]
0072:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
0076:    DD 44 24 08             fld      qword ptr [esp + 8]
007A:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
007E:    DE C1                   faddp    st(1)
0080:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0082:    DC 0F                   fmul     qword ptr [edi]
0084:    DC 77 AC                fdiv     qword ptr [edi - 0x54]
0087:    D9 C0                   fld      st(0)
0089:    D9 FC                   frndint    ; round(ST(0))
008B:    DE E9                   fsubp    st(1)
008D:    DC 4F AC                fmul     qword ptr [edi - 0x54]
0090:    D9 E1                   fabs    ; |ST(0)|
0092:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
0095:    D9 E8                   fld1    ; Load 1.0
0097:    DE E9                   fsubp    st(1)
0099:    DC 4F F0                fmul     qword ptr [edi - 0x10]
009C:    DC 4F 08                fmul     qword ptr [edi + 8]
009F:    83 7F 88 02             cmp      dword ptr [edi - 0x78], 2
00A3:    74 02                   je       0xa7    ; Conditional jump
00A5:    D9 E0                   fchs    ; -ST(0)
00A7:    DD 04 24                fld      qword ptr [esp]
00AA:    D8 C8                   fmul     st(0)
00AC:    DD 44 24 08             fld      qword ptr [esp + 8]
00B0:    D8 C8                   fmul     st(0)
00B2:    DE C1                   faddp    st(1)
00B4:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B6:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00BA:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00BC:    DC 0F                   fmul     qword ptr [edi]
00BE:    DC 77 8C                fdiv     qword ptr [edi - 0x74]
00C1:    D9 C0                   fld      st(0)
00C3:    D9 FC                   frndint    ; round(ST(0))
00C5:    DE E9                   fsubp    st(1)
00C7:    DC 4F 8C                fmul     qword ptr [edi - 0x74]
00CA:    D9 E1                   fabs    ; |ST(0)|
00CC:    DC 77 94                fdiv     qword ptr [edi - 0x6c]
00CF:    D9 E8                   fld1    ; Load 1.0
00D1:    DE E9                   fsubp    st(1)
00D3:    DC 4F 94                fmul     qword ptr [edi - 0x6c]
00D6:    DC 4F 08                fmul     qword ptr [edi + 8]
00D9:    83 7F 88 02             cmp      dword ptr [edi - 0x78], 2
00DD:    74 02                   je       0xe1    ; Conditional jump
00DF:    D9 E0                   fchs    ; -ST(0)
00E1:    83 7F 88 01             cmp      dword ptr [edi - 0x78], 1
00E5:    74 17                   je       0xfe    ; Conditional jump
00E7:    83 7F 88 02             cmp      dword ptr [edi - 0x78], 2
00EB:    74 11                   je       0xfe    ; Conditional jump
00ED:    D8 D1                   fcom     st(1)
00EF:    9B                      wait
00F0:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F2:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00F4:    73 04                   jae      0xfa    ; Conditional jump
00F6:    DD D8                   fstp     st(0)
00F8:    EB 02                   jmp      0xfc
00FA:    DD D9                   fstp     st(1)
00FC:    EB 0F                   jmp      0x10d
00FE:    D8 D1                   fcom     st(1)
0100:    9B                      wait
0101:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0103:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0105:    73 04                   jae      0x10b    ; Conditional jump
0107:    DD D9                   fstp     st(1)
0109:    EB 02                   jmp      0x10d
010B:    DD D8                   fstp     st(0)
010D:    DD 04 24                fld      qword ptr [esp]
0110:    D8 C8                   fmul     st(0)
0112:    DD 44 24 08             fld      qword ptr [esp + 8]
0116:    D8 C8                   fmul     st(0)
0118:    DE C1                   faddp    st(1)
011A:    DD 44 24 10             fld      qword ptr [esp + 0x10]
011E:    D8 C8                   fmul     st(0)
0120:    DE C1                   faddp    st(1)
0122:    D9 FA                   fsqrt    ; sqrt(ST(0))
0124:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0127:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
012A:    D9 E1                   fabs    ; |ST(0)|
012C:    DC 67 A4                fsub     qword ptr [edi - 0x5c]
012F:    D8 D1                   fcom     st(1)
0131:    9B                      wait
0132:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0134:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0136:    73 04                   jae      0x13c    ; Conditional jump
0138:    DD D8                   fstp     st(0)
013A:    EB 02                   jmp      0x13e
013C:    DD D9                   fstp     st(1)
013E:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0142:    75 03                   jne      0x147    ; Conditional jump
0144:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0147:    DD 5E E0                fstp     qword ptr [esi - 0x20]
014A:    83 C4 32                add      esp, 0x32
014D:    C3                      ret
