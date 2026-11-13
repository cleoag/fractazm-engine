; Disassembly: gumdropIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 293 bytes, decoded: 293 bytes (100.0%)
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
004B:    90                      nop
004C:    83 C7 D4                add      edi, -0x2c
004F:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0053:    DD 44 24 08             fld      qword ptr [esp + 8]
0057:    DD 04 24                fld      qword ptr [esp]
005A:    D9 C0                   fld      st(0)
005C:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
005F:    D9 C2                   fld      st(2)
0061:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0064:    DE C1                   faddp    st(1)
0066:    D9 C3                   fld      st(3)
0068:    D8 4F BC                fmul     dword ptr [edi - 0x44]
006B:    DE C1                   faddp    st(1)
006D:    D9 C1                   fld      st(1)
006F:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0072:    D9 C3                   fld      st(3)
0074:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0077:    DE C1                   faddp    st(1)
0079:    D9 C4                   fld      st(4)
007B:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
007E:    DE C1                   faddp    st(1)
0080:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0082:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0085:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0087:    D8 4F CC                fmul     dword ptr [edi - 0x34]
008A:    DE C3                   faddp    st(3)
008C:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008E:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0091:    DE C2                   faddp    st(2)
0093:    D8 C8                   fmul     st(0)
0095:    DD 1C 24                fstp     qword ptr [esp]
0098:    D8 C8                   fmul     st(0)
009A:    DD 5C 24 08             fstp     qword ptr [esp + 8]
009E:    D8 C8                   fmul     st(0)
00A0:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A4:    83 C7 2C                add      edi, 0x2c
00A7:    90                      nop
00A8:    D9 E8                   fld1    ; Load 1.0
00AA:    DD 47 F0                fld      qword ptr [edi - 0x10]
00AD:    D9 E1                   fabs    ; |ST(0)|
00AF:    DD 54 24 18             fst      qword ptr [esp + 0x18]
00B3:    DD 47 B8                fld      qword ptr [edi - 0x48]
00B6:    D9 E1                   fabs    ; |ST(0)|
00B8:    DD 54 24 20             fst      qword ptr [esp + 0x20]
00BC:    DE C1                   faddp    st(1)
00BE:    DE C1                   faddp    st(1)
00C0:    D9 E8                   fld1    ; Load 1.0
00C2:    DE F1                   fdivrp   st(1)
00C4:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
00C8:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00CC:    DC 44 24 08             fadd     qword ptr [esp + 8]
00D0:    D8 C8                   fmul     st(0)
00D2:    DD 04 24                fld      qword ptr [esp]
00D5:    D8 C8                   fmul     st(0)
00D7:    DE C1                   faddp    st(1)
00D9:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00DD:    DC 44 24 08             fadd     qword ptr [esp + 8]
00E1:    DC 0C 24                fmul     qword ptr [esp]
00E4:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00E8:    90                      nop
00E9:    DC C9                   fmul     st(1), st(0)
00EB:    DE C1                   faddp    st(1)
00ED:    DE C1                   faddp    st(1)
00EF:    DC 4C 24 28             fmul     qword ptr [esp + 0x28]
00F3:    D9 FA                   fsqrt    ; sqrt(ST(0))
00F5:    D9 FA                   fsqrt    ; sqrt(ST(0))
00F7:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00FB:    DC 44 24 08             fadd     qword ptr [esp + 8]
00FF:    DC 04 24                fadd     qword ptr [esp]
0102:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
0106:    DC 4C 24 28             fmul     qword ptr [esp + 0x28]
010A:    D9 FA                   fsqrt    ; sqrt(ST(0))
010C:    D9 FA                   fsqrt    ; sqrt(ST(0))
010E:    D9 E0                   fchs    ; -ST(0)
0110:    DE C1                   faddp    st(1)
0112:    DC 0F                   fmul     qword ptr [edi]
0114:    90                      nop
0115:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0119:    75 03                   jne      0x11e    ; Conditional jump
011B:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
011E:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0121:    83 C4 3C                add      esp, 0x3c
0124:    C3                      ret
