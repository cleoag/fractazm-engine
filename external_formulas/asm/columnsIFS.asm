; Disassembly: columnsIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 365 bytes, decoded: 365 bytes (100.0%)
; DEoption: 20

0000:    83 C4 D0                add      esp, -0x30
0003:    66 0F 10 46 90          movupd   xmm0, xmmword ptr [esi - 0x70]
0008:    F2 0F 10 4E 88          movsd    xmm1, qword ptr [esi - 0x78]
000D:    66 0F 59 47 D0          mulpd    xmm0, xmmword ptr [edi - 0x30]
0012:    F2 0F 59 4F D0          mulsd    xmm1, qword ptr [edi - 0x30]
0017:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
001C:    F2 0F 58 4F B8          addsd    xmm1, qword ptr [edi - 0x48]
0021:    83 7F 90 00             cmp      dword ptr [edi - 0x70], 0
0025:    74 19                   je       0x40    ; Conditional jump
0027:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002C:    F2 0F 59 6F D0          mulsd    xmm5, qword ptr [edi - 0x30]
0031:    66 0F 11 46 90          movupd   xmmword ptr [esi - 0x70], xmm0
0036:    F2 0F 11 4E 88          movsd    qword ptr [esi - 0x78], xmm1
003B:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0040:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0045:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
004B:    DD 04 24                fld      qword ptr [esp]
004E:    D9 E1                   fabs    ; |ST(0)|
0050:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0054:    D9 E1                   fabs    ; |ST(0)|
0056:    D9 C0                   fld      st(0)
0058:    D8 C2                   fadd     st(2)
005A:    D9 C1                   fld      st(1)
005C:    D8 E3                   fsub     st(3)
005E:    DC 67 A4                fsub     qword ptr [edi - 0x5c]
0061:    DD 1C 24                fstp     qword ptr [esp]
0064:    DC 67 9C                fsub     qword ptr [edi - 0x64]
0067:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
006B:    DD D8                   fstp     st(0)
006D:    DD D8                   fstp     st(0)
006F:    DD 04 24                fld      qword ptr [esp]
0072:    D9 E1                   fabs    ; |ST(0)|
0074:    D9 E0                   fchs    ; -ST(0)
0076:    DD 44 24 10             fld      qword ptr [esp + 0x10]
007A:    D9 C0                   fld      st(0)
007C:    D8 EA                   fsubr    st(2)
007E:    D9 C1                   fld      st(1)
0080:    D8 C3                   fadd     st(3)
0082:    D9 E0                   fchs    ; -ST(0)
0084:    DC 67 94                fsub     qword ptr [edi - 0x6c]
0087:    DC 0F                   fmul     qword ptr [edi]
0089:    DD 1C 24                fstp     qword ptr [esp]
008C:    DC 67 94                fsub     qword ptr [edi - 0x6c]
008F:    DC 0F                   fmul     qword ptr [edi]
0091:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0095:    DD D8                   fstp     st(0)
0097:    DD D8                   fstp     st(0)
0099:    DD 47 E8                fld      qword ptr [edi - 0x18]
009C:    DD 04 24                fld      qword ptr [esp]
009F:    D9 E4                   ftst
00A1:    9B                      wait
00A2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A4:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A6:    76 19                   jbe      0xc1    ; Conditional jump
00A8:    8B 57 AC                mov      edx, dword ptr [edi - 0x54]
00AB:    4A                      dec      edx
00AC:    7E 13                   jle      0xc1    ; Conditional jump
00AE:    81 FA E7 03 00 00       cmp      edx, 0x3e7
00B4:    7F 0B                   jg       0xc1    ; Conditional jump
00B6:    D8 E9                   fsubr    st(1)
00B8:    D9 E1                   fabs    ; |ST(0)|
00BA:    D8 E9                   fsubr    st(1)
00BC:    D9 E1                   fabs    ; |ST(0)|
00BE:    4A                      dec      edx
00BF:    7F F5                   jg       0xb6    ; Conditional jump
00C1:    DD D9                   fstp     st(1)
00C3:    8B 57 F4                mov      edx, dword ptr [edi - 0xc]
00C6:    81 E2 00 00 00 80       and      edx, 0x80000000
00CC:    75 38                   jne      0x106    ; Conditional jump
00CE:    D8 C8                   fmul     st(0)
00D0:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00D4:    8B 57 B4                mov      edx, dword ptr [edi - 0x4c]
00D7:    81 E2 00 00 00 80       and      edx, 0x80000000
00DD:    75 08                   jne      0xe7    ; Conditional jump
00DF:    DC 6F B0                fsubr    qword ptr [edi - 0x50]
00E2:    D9 E1                   fabs    ; |ST(0)|
00E4:    DC 6F B0                fsubr    qword ptr [edi - 0x50]
00E7:    D8 C8                   fmul     st(0)
00E9:    DE C1                   faddp    st(1)
00EB:    D9 FA                   fsqrt    ; sqrt(ST(0))
00ED:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00F0:    85 D2                   test     edx, edx
00F2:    74 10                   je       0x104    ; Conditional jump
00F4:    DD 44 24 08             fld      qword ptr [esp + 8]
00F8:    D8 C8                   fmul     st(0)
00FA:    DD 47 B0                fld      qword ptr [edi - 0x50]
00FD:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0100:    DE C9                   fmulp    st(1)
0102:    DE E9                   fsubp    st(1)
0104:    EB 3F                   jmp      0x145
0106:    D9 E1                   fabs    ; |ST(0)|
0108:    DD 44 24 10             fld      qword ptr [esp + 0x10]
010C:    8B 57 B4                mov      edx, dword ptr [edi - 0x4c]
010F:    81 E2 00 00 00 80       and      edx, 0x80000000
0115:    75 08                   jne      0x11f    ; Conditional jump
0117:    DC 6F B0                fsubr    qword ptr [edi - 0x50]
011A:    D9 E1                   fabs    ; |ST(0)|
011C:    DC 6F B0                fsubr    qword ptr [edi - 0x50]
011F:    D9 E1                   fabs    ; |ST(0)|
0121:    D8 D1                   fcom     st(1)
0123:    9B                      wait
0124:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0126:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0128:    73 04                   jae      0x12e    ; Conditional jump
012A:    DD D8                   fstp     st(0)
012C:    EB 02                   jmp      0x130
012E:    DD D9                   fstp     st(1)
0130:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0133:    85 D2                   test     edx, edx
0135:    74 0E                   je       0x145    ; Conditional jump
0137:    DD 44 24 08             fld      qword ptr [esp + 8]
013B:    DD 47 B0                fld      qword ptr [edi - 0x50]
013E:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0141:    DE C9                   fmulp    st(1)
0143:    DE C1                   faddp    st(1)
0145:    DD 44 24 08             fld      qword ptr [esp + 8]
0149:    D9 E1                   fabs    ; |ST(0)|
014B:    DC 67 E0                fsub     qword ptr [edi - 0x20]
014E:    D8 D1                   fcom     st(1)
0150:    9B                      wait
0151:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0153:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0155:    73 04                   jae      0x15b    ; Conditional jump
0157:    DD D8                   fstp     st(0)
0159:    EB 02                   jmp      0x15d
015B:    DD D9                   fstp     st(1)
015D:    83 7F 90 00             cmp      dword ptr [edi - 0x70], 0
0161:    75 03                   jne      0x166    ; Conditional jump
0163:    DC 77 D0                fdiv     qword ptr [edi - 0x30]
0166:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0169:    83 C4 30                add      esp, 0x30
016C:    C3                      ret
