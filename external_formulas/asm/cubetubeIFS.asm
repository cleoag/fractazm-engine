; Disassembly: cubetubeIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 333 bytes, decoded: 333 bytes (100.0%)
; DEoption: 20

0000:    83 C4 E4                add      esp, -0x1c
0003:    66 0F 10 46 90          movupd   xmm0, xmmword ptr [esi - 0x70]
0008:    F2 0F 10 4E 88          movsd    xmm1, qword ptr [esi - 0x78]
000D:    66 0F 59 47 D0          mulpd    xmm0, xmmword ptr [edi - 0x30]
0012:    F2 0F 59 4F D0          mulsd    xmm1, qword ptr [edi - 0x30]
0017:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
001C:    F2 0F 58 4F B8          addsd    xmm1, qword ptr [edi - 0x48]
0021:    83 7F 94 00             cmp      dword ptr [edi - 0x6c], 0
0025:    74 19                   je       0x40    ; Conditional jump
0027:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002C:    F2 0F 59 6F D0          mulsd    xmm5, qword ptr [edi - 0x30]
0031:    66 0F 11 46 90          movupd   xmmword ptr [esi - 0x70], xmm0
0036:    F2 0F 11 4E 88          movsd    qword ptr [esi - 0x78], xmm1
003B:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0040:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0045:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
004B:    DD 04 24                fld      qword ptr [esp]
004E:    D9 E1                   fabs    ; |ST(0)|
0050:    DD 44 24 08             fld      qword ptr [esp + 8]
0054:    D9 E1                   fabs    ; |ST(0)|
0056:    DD 44 24 10             fld      qword ptr [esp + 0x10]
005A:    D9 E1                   fabs    ; |ST(0)|
005C:    D8 D1                   fcom     st(1)
005E:    9B                      wait
005F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0061:    80 E4 41                and      ah, 0x41
0064:    74 02                   je       0x68    ; Conditional jump
0066:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0068:    D8 D2                   fcom     st(2)
006A:    9B                      wait
006B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006D:    80 E4 41                and      ah, 0x41
0070:    74 02                   je       0x74    ; Conditional jump
0072:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0074:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0076:    D8 D2                   fcom     st(2)
0078:    9B                      wait
0079:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
007B:    80 E4 41                and      ah, 0x41
007E:    74 02                   je       0x82    ; Conditional jump
0080:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0082:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0086:    DD 5C 24 08             fstp     qword ptr [esp + 8]
008A:    DD 1C 24                fstp     qword ptr [esp]
008D:    DD 04 24                fld      qword ptr [esp]
0090:    DC 4F B0                fmul     qword ptr [edi - 0x50]
0093:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0097:    DC 4F A8                fmul     qword ptr [edi - 0x58]
009A:    DE C1                   faddp    st(1)
009C:    DD 04 24                fld      qword ptr [esp]
009F:    DC 4F A8                fmul     qword ptr [edi - 0x58]
00A2:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00A6:    DC 4F B0                fmul     qword ptr [edi - 0x50]
00A9:    D9 E0                   fchs    ; -ST(0)
00AB:    DE C1                   faddp    st(1)
00AD:    DD 1C 24                fstp     qword ptr [esp]
00B0:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00B4:    DD 04 24                fld      qword ptr [esp]
00B7:    DC 4F A0                fmul     qword ptr [edi - 0x60]
00BA:    DD 44 24 08             fld      qword ptr [esp + 8]
00BE:    DC 4F 98                fmul     qword ptr [edi - 0x68]
00C1:    DE C1                   faddp    st(1)
00C3:    DD 04 24                fld      qword ptr [esp]
00C6:    DC 4F 98                fmul     qword ptr [edi - 0x68]
00C9:    DD 44 24 08             fld      qword ptr [esp + 8]
00CD:    DC 4F A0                fmul     qword ptr [edi - 0x60]
00D0:    D9 E0                   fchs    ; -ST(0)
00D2:    DE C1                   faddp    st(1)
00D4:    DD 1C 24                fstp     qword ptr [esp]
00D7:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00DB:    DD 44 24 08             fld      qword ptr [esp + 8]
00DF:    DC 67 E8                fsub     qword ptr [edi - 0x18]
00E2:    DD 54 24 08             fst      qword ptr [esp + 8]
00E6:    D8 C8                   fmul     st(0)
00E8:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00EC:    DC 67 E0                fsub     qword ptr [edi - 0x20]
00EF:    DD 54 24 10             fst      qword ptr [esp + 0x10]
00F3:    D8 C8                   fmul     st(0)
00F5:    DE C1                   faddp    st(1)
00F7:    D9 FA                   fsqrt    ; sqrt(ST(0))
00F9:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00FC:    DD 04 24                fld      qword ptr [esp]
00FF:    D9 E1                   fabs    ; |ST(0)|
0101:    DC 67 8C                fsub     qword ptr [edi - 0x74]
0104:    D8 C8                   fmul     st(0)
0106:    DD 44 24 08             fld      qword ptr [esp + 8]
010A:    D8 C8                   fmul     st(0)
010C:    DE C1                   faddp    st(1)
010E:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0112:    D8 C8                   fmul     st(0)
0114:    DE C1                   faddp    st(1)
0116:    D9 FA                   fsqrt    ; sqrt(ST(0))
0118:    DC 67 84                fsub     qword ptr [edi - 0x7c]
011B:    D8 D1                   fcom     st(1)
011D:    9B                      wait
011E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0120:    80 E4 41                and      ah, 0x41
0123:    74 0A                   je       0x12f    ; Conditional jump
0125:    DD D9                   fstp     st(1)
0127:    DD 87 74 FF FF FF       fld      qword ptr [edi - 0x8c]
012D:    EB 08                   jmp      0x137
012F:    DD D8                   fstp     st(0)
0131:    DD 87 7C FF FF FF       fld      qword ptr [edi - 0x84]
0137:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
013D:    83 7F 94 00             cmp      dword ptr [edi - 0x6c], 0
0141:    75 03                   jne      0x146    ; Conditional jump
0143:    DC 77 D0                fdiv     qword ptr [edi - 0x30]
0146:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0149:    83 C4 1C                add      esp, 0x1c
014C:    C3                      ret
