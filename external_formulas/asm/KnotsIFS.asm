; Disassembly: KnotsIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 272 bytes, decoded: 272 bytes (100.0%)
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
004B:    DD 04 24                fld      qword ptr [esp]
004E:    DD 44 24 08             fld      qword ptr [esp + 8]
0052:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0054:    DB 47 AC                fild     dword ptr [edi - 0x54]
0057:    DB 47 A8                fild     dword ptr [edi - 0x58]
005A:    DA 77 B0                fidiv    dword ptr [edi - 0x50]
005D:    DE C1                   faddp    st(1)
005F:    DE C9                   fmulp    st(1)
0061:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0063:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0067:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
006B:    DD 04 24                fld      qword ptr [esp]
006E:    D8 C8                   fmul     st(0)
0070:    DD 44 24 08             fld      qword ptr [esp + 8]
0074:    D8 C8                   fmul     st(0)
0076:    DE C1                   faddp    st(1)
0078:    D9 FA                   fsqrt    ; sqrt(ST(0))
007A:    DC 67 F0                fsub     qword ptr [edi - 0x10]
007D:    DD 1C 24                fstp     qword ptr [esp]
0080:    DD 04 24                fld      qword ptr [esp]
0083:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0087:    DD 44 24 10             fld      qword ptr [esp + 0x10]
008B:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
008F:    D9 E0                   fchs    ; -ST(0)
0091:    DE C1                   faddp    st(1)
0093:    DD 04 24                fld      qword ptr [esp]
0096:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
009A:    DD 44 24 10             fld      qword ptr [esp + 0x10]
009E:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00A2:    DE C1                   faddp    st(1)
00A4:    DD 1C 24                fstp     qword ptr [esp]
00A7:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00AB:    DD 04 24                fld      qword ptr [esp]
00AE:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00B2:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00B4:    DC 6F 10                fsubr    qword ptr [edi + 0x10]
00B7:    DC 0F                   fmul     qword ptr [edi]
00B9:    DA 4F B0                fimul    dword ptr [edi - 0x50]
00BC:    D9 FC                   frndint    ; round(ST(0))
00BE:    DA 77 B0                fidiv    dword ptr [edi - 0x50]
00C1:    DC 4F 08                fmul     qword ptr [edi + 8]
00C4:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00C6:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00CA:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00CE:    DD 04 24                fld      qword ptr [esp]
00D1:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00D5:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00D9:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00DD:    D9 E0                   fchs    ; -ST(0)
00DF:    DE C1                   faddp    st(1)
00E1:    DC 47 A0                fadd     qword ptr [edi - 0x60]
00E4:    D8 C8                   fmul     st(0)
00E6:    DD 04 24                fld      qword ptr [esp]
00E9:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00ED:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00F1:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00F5:    DE C1                   faddp    st(1)
00F7:    D8 C8                   fmul     st(0)
00F9:    DE C1                   faddp    st(1)
00FB:    D9 FA                   fsqrt    ; sqrt(ST(0))
00FD:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0100:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0104:    75 03                   jne      0x109    ; Conditional jump
0106:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0109:    DD 5E E0                fstp     qword ptr [esi - 0x20]
010C:    83 C4 32                add      esp, 0x32
010F:    C3                      ret
