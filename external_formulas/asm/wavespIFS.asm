; Disassembly: wavespIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 162 bytes, decoded: 162 bytes (100.0%)
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
004B:    DD 04 24                fld      qword ptr [esp]
004E:    D8 C8                   fmul     st(0)
0050:    DD 44 24 08             fld      qword ptr [esp + 8]
0054:    D8 C8                   fmul     st(0)
0056:    DE C1                   faddp    st(1)
0058:    DD 47 AC                fld      qword ptr [edi - 0x54]
005B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
005D:    D9 E1                   fabs    ; |ST(0)|
005F:    D9 F1                   fyl2x
0061:    83 7F A8 00             cmp      dword ptr [edi - 0x58], 0
0065:    75 10                   jne      0x77    ; Conditional jump
0067:    D9 E8                   fld1    ; Load 1.0
0069:    D9 C1                   fld      st(1)
006B:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
006D:    D9 F0                   f2xm1
006F:    DE C1                   faddp    st(1)
0071:    D9 FD                   fscale
0073:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0075:    DD D8                   fstp     st(0)
0077:    DD 44 24 08             fld      qword ptr [esp + 8]
007B:    DD 04 24                fld      qword ptr [esp]
007E:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0080:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0083:    DE C1                   faddp    st(1)
0085:    D9 FE                   fsin    ; sin(ST(0))
0087:    D9 E1                   fabs    ; |ST(0)|
0089:    DC 4F B8                fmul     qword ptr [edi - 0x48]
008C:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
0090:    D9 E0                   fchs    ; -ST(0)
0092:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0096:    75 03                   jne      0x9b    ; Conditional jump
0098:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
009B:    DD 5E E0                fstp     qword ptr [esi - 0x20]
009E:    83 C4 1C                add      esp, 0x1c
00A1:    C3                      ret
