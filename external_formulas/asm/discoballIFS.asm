; Disassembly: discoballIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 229 bytes, decoded: 229 bytes (100.0%)
; DEoption: 20

0000:    83 C4 D8                add      esp, -0x28
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
004B:    DD 04 24                fld      qword ptr [esp]
004E:    DD 44 24 08             fld      qword ptr [esp + 8]
0052:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0054:    DC 6F 10                fsubr    qword ptr [edi + 0x10]
0057:    DC 0F                   fmul     qword ptr [edi]
0059:    DA 4F F4                fimul    dword ptr [edi - 0xc]
005C:    D9 FC                   frndint    ; round(ST(0))
005E:    DA 77 F4                fidiv    dword ptr [edi - 0xc]
0061:    DC 4F 08                fmul     qword ptr [edi + 8]
0064:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0066:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
006A:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
006E:    DD 04 24                fld      qword ptr [esp]
0071:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0075:    DD 44 24 08             fld      qword ptr [esp + 8]
0079:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
007D:    D9 E0                   fchs    ; -ST(0)
007F:    DE C1                   faddp    st(1)
0081:    DD 04 24                fld      qword ptr [esp]
0084:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
0088:    DD 44 24 08             fld      qword ptr [esp + 8]
008C:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0090:    DE C1                   faddp    st(1)
0092:    DD 1C 24                fstp     qword ptr [esp]
0095:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0099:    DD 04 24                fld      qword ptr [esp]
009C:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00A0:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00A2:    DC 6F 10                fsubr    qword ptr [edi + 0x10]
00A5:    DC 0F                   fmul     qword ptr [edi]
00A7:    DA 4F F0                fimul    dword ptr [edi - 0x10]
00AA:    D9 FC                   frndint    ; round(ST(0))
00AC:    DA 77 F0                fidiv    dword ptr [edi - 0x10]
00AF:    DC 4F 08                fmul     qword ptr [edi + 8]
00B2:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00B4:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00B8:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00BC:    DD 04 24                fld      qword ptr [esp]
00BF:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00C3:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00C7:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00CB:    DE C1                   faddp    st(1)
00CD:    DC 67 B4                fsub     qword ptr [edi - 0x4c]
00D0:    D9 E1                   fabs    ; |ST(0)|
00D2:    DC 67 AC                fsub     qword ptr [edi - 0x54]
00D5:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
00D9:    75 03                   jne      0xde    ; Conditional jump
00DB:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00DE:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00E1:    83 C4 28                add      esp, 0x28
00E4:    C3                      ret
