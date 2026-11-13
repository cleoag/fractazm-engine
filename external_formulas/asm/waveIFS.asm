; Disassembly: waveIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 179 bytes, decoded: 179 bytes (100.0%)
; DEoption: 20

0000:    83 C4 E0                add      esp, -0x20
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
004B:    8B 57 AC                mov      edx, dword ptr [edi - 0x54]
004E:    83 E2 01                and      edx, 1
0051:    DD 47 B8                fld      qword ptr [edi - 0x48]
0054:    C1 E2 04                shl      edx, 4
0057:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
005B:    DD 04 14                fld      qword ptr [esp + edx]
005E:    83 7F B0 00             cmp      dword ptr [edi - 0x50], 0
0062:    74 25                   je       0x89    ; Conditional jump
0064:    D8 C8                   fmul     st(0)
0066:    DD 44 24 08             fld      qword ptr [esp + 8]
006A:    D8 C8                   fmul     st(0)
006C:    DE C1                   faddp    st(1)
006E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0070:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0073:    83 7F A8 00             cmp      dword ptr [edi - 0x58], 0
0077:    74 0C                   je       0x85    ; Conditional jump
0079:    D9 E8                   fld1    ; Load 1.0
007B:    D8 C1                   fadd     st(1)
007D:    DC 7C 24 18             fdivr    qword ptr [esp + 0x18]
0081:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0085:    D9 FF                   fcos    ; cos(ST(0))
0087:    EB 10                   jmp      0x99
0089:    DC 4F F0                fmul     qword ptr [edi - 0x10]
008C:    D9 FF                   fcos    ; cos(ST(0))
008E:    DD 44 24 08             fld      qword ptr [esp + 8]
0092:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0095:    D9 FF                   fcos    ; cos(ST(0))
0097:    DE C1                   faddp    st(1)
0099:    F7 DA                   neg      edx
009B:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
009F:    DC 64 14 10             fsub     qword ptr [esp + edx + 0x10]
00A3:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00A7:    75 03                   jne      0xac    ; Conditional jump
00A9:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00AC:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00AF:    83 C4 20                add      esp, 0x20
00B2:    C3                      ret
