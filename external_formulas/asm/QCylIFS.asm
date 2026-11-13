; Disassembly: QCylIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 156 bytes, decoded: 156 bytes (100.0%)
; DEoption: 20

0000:    83 C4 E4                add      esp, -0x1c
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
004F:    D9 E1                   fabs    ; |ST(0)|
0051:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0055:    DD 04 24                fld      qword ptr [esp]
0058:    D8 C8                   fmul     st(0)
005A:    DD 44 24 08             fld      qword ptr [esp + 8]
005E:    D8 C8                   fmul     st(0)
0060:    DE C1                   faddp    st(1)
0062:    D9 FA                   fsqrt    ; sqrt(ST(0))
0064:    DD 14 24                fst      qword ptr [esp]
0067:    D9 C0                   fld      st(0)
0069:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
006D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006F:    D9 E8                   fld1    ; Load 1.0
0071:    D9 E0                   fchs    ; -ST(0)
0073:    DE C1                   faddp    st(1)
0075:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0078:    DE C1                   faddp    st(1)
007A:    DC 67 B8                fsub     qword ptr [edi - 0x48]
007D:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0081:    DC 67 AC                fsub     qword ptr [edi - 0x54]
0084:    D9 C0                   fld      st(0)
0086:    D9 E1                   fabs    ; |ST(0)|
0088:    DE C1                   faddp    st(1)
008A:    DE C1                   faddp    st(1)
008C:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0090:    75 03                   jne      0x95    ; Conditional jump
0092:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0095:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0098:    83 C4 1C                add      esp, 0x1c
009B:    C3                      ret
