; Disassembly: ScarabIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 191 bytes, decoded: 191 bytes (100.0%)
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
004B:    DD 04 24                fld      qword ptr [esp]
004E:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0052:    D8 C8                   fmul     st(0)
0054:    DD 14 24                fst      qword ptr [esp]
0057:    DD 44 24 08             fld      qword ptr [esp + 8]
005B:    D8 C8                   fmul     st(0)
005D:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0061:    D8 C8                   fmul     st(0)
0063:    DE C1                   faddp    st(1)
0065:    DD 54 24 08             fst      qword ptr [esp + 8]
0069:    DE C1                   faddp    st(1)
006B:    D9 C0                   fld      st(0)
006D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006F:    DD 47 F0                fld      qword ptr [edi - 0x10]
0072:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0076:    DE C1                   faddp    st(1)
0078:    D8 C8                   fmul     st(0)
007A:    DE C9                   fmulp    st(1)
007C:    DD 04 24                fld      qword ptr [esp]
007F:    DC 4C 24 08             fmul     qword ptr [esp + 8]
0083:    DD 47 B8                fld      qword ptr [edi - 0x48]
0086:    D9 E1                   fabs    ; |ST(0)|
0088:    DD 54 24 18             fst      qword ptr [esp + 0x18]
008C:    D8 C8                   fmul     st(0)
008E:    DE C9                   fmulp    st(1)
0090:    DE C1                   faddp    st(1)
0092:    D9 FA                   fsqrt    ; sqrt(ST(0))
0094:    D9 FA                   fsqrt    ; sqrt(ST(0))
0096:    DD 04 24                fld      qword ptr [esp]
0099:    D8 C8                   fmul     st(0)
009B:    DD 44 24 08             fld      qword ptr [esp + 8]
009F:    D8 C8                   fmul     st(0)
00A1:    DE C1                   faddp    st(1)
00A3:    D9 FA                   fsqrt    ; sqrt(ST(0))
00A5:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00A9:    D9 FA                   fsqrt    ; sqrt(ST(0))
00AB:    D9 E0                   fchs    ; -ST(0)
00AD:    DE C1                   faddp    st(1)
00AF:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00B3:    75 03                   jne      0xb8    ; Conditional jump
00B5:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00B8:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00BB:    83 C4 26                add      esp, 0x26
00BE:    C3                      ret
