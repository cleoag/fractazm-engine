; Disassembly: boxIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 210 bytes, decoded: 210 bytes (100.0%)
; DEoption: 20

0000:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0005:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000A:    66 0F 59 47 D0          mulpd    xmm0, xmmword ptr [edi - 0x30]
000F:    F2 0F 59 4F D0          mulsd    xmm1, qword ptr [edi - 0x30]
0014:    66 0F 58 47 B0          addpd    xmm0, xmmword ptr [edi - 0x50]
0019:    F2 0F 58 4F C0          addsd    xmm1, qword ptr [edi - 0x40]
001E:    83 7F A0 00             cmp      dword ptr [edi - 0x60], 0
0022:    74 19                   je       0x3d    ; Conditional jump
0024:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
0029:    F2 0F 59 6F D0          mulsd    xmm5, qword ptr [edi - 0x30]
002E:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0033:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
0038:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
003D:    83 7F A4 00             cmp      dword ptr [edi - 0x5c], 0
0041:    74 3D                   je       0x80    ; Conditional jump
0043:    83 C4 E8                add      esp, -0x18
0046:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
004B:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
0051:    D9 47 A4                fld      dword ptr [edi - 0x5c]
0054:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
0058:    D9 FB                   fsincos    ; ST(0)=cos, push sin
005A:    DD 44 24 08             fld      qword ptr [esp + 8]
005E:    DD 04 24                fld      qword ptr [esp]
0061:    D9 C3                   fld      st(3)
0063:    D8 C9                   fmul     st(1)
0065:    D9 C2                   fld      st(2)
0067:    D8 CC                   fmul     st(4)
0069:    DE C1                   faddp    st(1)
006B:    DD 1C 24                fstp     qword ptr [esp]
006E:    DE CA                   fmulp    st(2)
0070:    DE CA                   fmulp    st(2)
0072:    DE E1                   fsubrp   st(1)
0074:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0078:    66 0F 10 04 24          movupd   xmm0, xmmword ptr [esp]
007D:    83 C4 18                add      esp, 0x18
0080:    F2 0F 10 D9             movsd    xmm3, xmm1
0084:    66 0F 54 07             andpd    xmm0, xmmword ptr [edi]
0088:    66 0F 54 0F             andpd    xmm1, xmmword ptr [edi]
008C:    F2 0F 59 5F A8          mulsd    xmm3, qword ptr [edi - 0x58]
0091:    66 0F 5C 47 E0          subpd    xmm0, xmmword ptr [edi - 0x20]
0096:    F2 0F 5C 4F F0          subsd    xmm1, qword ptr [edi - 0x10]
009B:    F2 0F 5C C3             subsd    xmm0, xmm3
009F:    F2 0F 5F C8             maxsd    xmm1, xmm0
00A3:    66 0F 15 C0             unpckhpd xmm0, xmm0
00A7:    F2 0F 5F C8             maxsd    xmm1, xmm0
00AB:    83 7F A0 00             cmp      dword ptr [edi - 0x60], 0
00AF:    75 05                   jne      0xb6    ; Conditional jump
00B1:    F2 0F 5E 4F D0          divsd    xmm1, qword ptr [edi - 0x30]
00B6:    8B 47 9C                mov      eax, dword ptr [edi - 0x64]
00B9:    F2 0F 11 4E E0          movsd    qword ptr [esi - 0x20], xmm1
00BE:    83 E0 03                and      eax, 3
00C1:    DD 44 C6 80             fld      qword ptr [esi + eax*8 - 0x80]
00C5:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
00C8:    D8 47 98                fadd     dword ptr [edi - 0x68]
00CB:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00D1:    C3                      ret
