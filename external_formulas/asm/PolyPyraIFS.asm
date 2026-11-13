; Disassembly: PolyPyraIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 283 bytes, decoded: 283 bytes (100.0%)
; DEoption: 20

0000:    83 C4 BE                add      esp, -0x42
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
0054:    DC 6F 10                fsubr    qword ptr [edi + 0x10]
0057:    DC 0F                   fmul     qword ptr [edi]
0059:    DA 4F B0                fimul    dword ptr [edi - 0x50]
005C:    DB 5C 24 30             fistp    dword ptr [esp + 0x30]
0060:    DB 44 24 30             fild     dword ptr [esp + 0x30]
0064:    DA 77 B0                fidiv    dword ptr [edi - 0x50]
0067:    DC 4F 08                fmul     qword ptr [edi + 8]
006A:    D9 FB                   fsincos    ; ST(0)=cos, push sin
006C:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0070:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0074:    DD 04 24                fld      qword ptr [esp]
0077:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
007B:    DD 44 24 08             fld      qword ptr [esp + 8]
007F:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
0083:    D9 E0                   fchs    ; -ST(0)
0085:    DE C1                   faddp    st(1)
0087:    DD 04 24                fld      qword ptr [esp]
008A:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
008E:    DD 44 24 08             fld      qword ptr [esp + 8]
0092:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0096:    DE C1                   faddp    st(1)
0098:    DC 4F 98                fmul     qword ptr [edi - 0x68]
009B:    D9 E1                   fabs    ; |ST(0)|
009D:    DE C1                   faddp    st(1)
009F:    DD 1C 24                fstp     qword ptr [esp]
00A2:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00A6:    DD 54 24 28             fst      qword ptr [esp + 0x28]
00AA:    DD 47 A0                fld      qword ptr [edi - 0x60]
00AD:    DE C1                   faddp    st(1)
00AF:    D9 E0                   fchs    ; -ST(0)
00B1:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00B5:    DD 47 18                fld      qword ptr [edi + 0x18]
00B8:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00BB:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00BD:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00C1:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00C5:    DD 04 24                fld      qword ptr [esp]
00C8:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00CC:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00D0:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00D4:    DE C1                   faddp    st(1)
00D6:    DD 44 24 28             fld      qword ptr [esp + 0x28]
00DA:    DC 67 A8                fsub     qword ptr [edi - 0x58]
00DD:    D9 E1                   fabs    ; |ST(0)|
00DF:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00E2:    D8 D1                   fcom     st(1)
00E4:    9B                      wait
00E5:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E7:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00E9:    73 04                   jae      0xef    ; Conditional jump
00EB:    DD D8                   fstp     st(0)
00ED:    EB 02                   jmp      0xf1
00EF:    DD D9                   fstp     st(1)
00F1:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00F5:    75 03                   jne      0xfa    ; Conditional jump
00F7:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00FA:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00FD:    DB 46 E8                fild     dword ptr [esi - 0x18]
0100:    DC 4F 90                fmul     qword ptr [edi - 0x70]
0103:    DC 47 88                fadd     qword ptr [edi - 0x78]
0106:    DB 44 24 30             fild     dword ptr [esp + 0x30]
010A:    DC 4F 80                fmul     qword ptr [edi - 0x80]
010D:    DE C1                   faddp    st(1)
010F:    D9 E1                   fabs    ; |ST(0)|
0111:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0117:    83 C4 42                add      esp, 0x42
011A:    C3                      ret
