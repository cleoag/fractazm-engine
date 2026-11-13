; Disassembly: foldcut12-IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 382 bytes, decoded: 382 bytes (100.0%)
; DEoption: 20

0000:    83 C4 B0                add      esp, -0x50
0003:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0008:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000D:    66 0F 59 47 E0          mulpd    xmm0, xmmword ptr [edi - 0x20]
0012:    F2 0F 59 4F E0          mulsd    xmm1, qword ptr [edi - 0x20]
0017:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
001C:    F2 0F 58 4F D0          addsd    xmm1, qword ptr [edi - 0x30]
0021:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
0025:    74 19                   je       0x40    ; Conditional jump
0027:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002C:    F2 0F 59 6F E0          mulsd    xmm5, qword ptr [edi - 0x20]
0031:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0036:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
003B:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0040:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0045:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
004B:    BA 02 00 00 00          mov      edx, 2
0050:    DD 04 24                fld      qword ptr [esp]
0053:    D9 E1                   fabs    ; |ST(0)|
0055:    DD 14 24                fst      qword ptr [esp]
0058:    DC 0F                   fmul     qword ptr [edi]
005A:    DD 44 24 08             fld      qword ptr [esp + 8]
005E:    D9 E1                   fabs    ; |ST(0)|
0060:    DD 54 24 08             fst      qword ptr [esp + 8]
0064:    DC 4F 08                fmul     qword ptr [edi + 8]
0067:    DD 44 24 10             fld      qword ptr [esp + 0x10]
006B:    D9 E1                   fabs    ; |ST(0)|
006D:    DD 54 24 10             fst      qword ptr [esp + 0x10]
0071:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0074:    DE C1                   faddp    st(1)
0076:    DE C1                   faddp    st(1)
0078:    D9 C0                   fld      st(0)
007A:    D9 E1                   fabs    ; |ST(0)|
007C:    DE C1                   faddp    st(1)
007E:    D9 E0                   fchs    ; -ST(0)
0080:    D9 C0                   fld      st(0)
0082:    DC 0F                   fmul     qword ptr [edi]
0084:    DC 04 24                fadd     qword ptr [esp]
0087:    DD 1C 24                fstp     qword ptr [esp]
008A:    D9 C0                   fld      st(0)
008C:    DC 4F 08                fmul     qword ptr [edi + 8]
008F:    DC 44 24 08             fadd     qword ptr [esp + 8]
0093:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0097:    D9 C0                   fld      st(0)
0099:    DC 4F 10                fmul     qword ptr [edi + 0x10]
009C:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
00A0:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A4:    DD D8                   fstp     st(0)
00A6:    DD 04 24                fld      qword ptr [esp]
00A9:    DC 4F 08                fmul     qword ptr [edi + 8]
00AC:    DD 44 24 08             fld      qword ptr [esp + 8]
00B0:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00B3:    D9 E0                   fchs    ; -ST(0)
00B5:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00B9:    DC 0F                   fmul     qword ptr [edi]
00BB:    D9 E0                   fchs    ; -ST(0)
00BD:    DE C1                   faddp    st(1)
00BF:    DE C1                   faddp    st(1)
00C1:    D9 C0                   fld      st(0)
00C3:    D9 E1                   fabs    ; |ST(0)|
00C5:    DE C1                   faddp    st(1)
00C7:    D9 C0                   fld      st(0)
00C9:    DC 4F 08                fmul     qword ptr [edi + 8]
00CC:    D9 E0                   fchs    ; -ST(0)
00CE:    DC 04 24                fadd     qword ptr [esp]
00D1:    DD 1C 24                fstp     qword ptr [esp]
00D4:    D9 C0                   fld      st(0)
00D6:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00D9:    DC 44 24 08             fadd     qword ptr [esp + 8]
00DD:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00E1:    D9 C0                   fld      st(0)
00E3:    DC 0F                   fmul     qword ptr [edi]
00E5:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
00E9:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00ED:    DD D8                   fstp     st(0)
00EF:    4A                      dec      edx
00F0:    0F 8F 5A FF FF FF       jg       0x50    ; Conditional jump
00F6:    DD 04 24                fld      qword ptr [esp]
00F9:    D9 E1                   fabs    ; |ST(0)|
00FB:    DD 44 24 08             fld      qword ptr [esp + 8]
00FF:    D9 E1                   fabs    ; |ST(0)|
0101:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0105:    D9 E1                   fabs    ; |ST(0)|
0107:    DD 47 A4                fld      qword ptr [edi - 0x5c]
010A:    DC CB                   fmul     st(3), st(0)
010C:    DD D8                   fstp     st(0)
010E:    DD 47 AC                fld      qword ptr [edi - 0x54]
0111:    DC CA                   fmul     st(2), st(0)
0113:    DD D8                   fstp     st(0)
0115:    DD 47 9C                fld      qword ptr [edi - 0x64]
0118:    DC C9                   fmul     st(1), st(0)
011A:    DD D8                   fstp     st(0)
011C:    DE C1                   faddp    st(1)
011E:    DE C1                   faddp    st(1)
0120:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0123:    DD 04 24                fld      qword ptr [esp]
0126:    D8 C8                   fmul     st(0)
0128:    DD 44 24 08             fld      qword ptr [esp + 8]
012C:    D8 C8                   fmul     st(0)
012E:    DE C1                   faddp    st(1)
0130:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0134:    D8 C8                   fmul     st(0)
0136:    DE C1                   faddp    st(1)
0138:    D9 FA                   fsqrt    ; sqrt(ST(0))
013A:    DD 47 B4                fld      qword ptr [edi - 0x4c]
013D:    D9 E1                   fabs    ; |ST(0)|
013F:    DE E9                   fsubp    st(1)
0141:    8B 57 B8                mov      edx, dword ptr [edi - 0x48]
0144:    81 E2 00 00 00 80       and      edx, 0x80000000
014A:    75 12                   jne      0x15e    ; Conditional jump
014C:    D8 D1                   fcom     st(1)
014E:    9B                      wait
014F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0151:    80 E4 41                and      ah, 0x41
0154:    74 04                   je       0x15a    ; Conditional jump
0156:    DD D9                   fstp     st(1)
0158:    EB 14                   jmp      0x16e
015A:    DD D8                   fstp     st(0)
015C:    EB 10                   jmp      0x16e
015E:    D8 D1                   fcom     st(1)
0160:    9B                      wait
0161:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0163:    80 E4 41                and      ah, 0x41
0166:    74 04                   je       0x16c    ; Conditional jump
0168:    DD D8                   fstp     st(0)
016A:    EB 02                   jmp      0x16e
016C:    DD D9                   fstp     st(1)
016E:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
0172:    75 03                   jne      0x177    ; Conditional jump
0174:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0177:    DD 5E E0                fstp     qword ptr [esi - 0x20]
017A:    83 C4 50                add      esp, 0x50
017D:    C3                      ret
