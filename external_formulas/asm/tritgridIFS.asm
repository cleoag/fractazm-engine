; Disassembly: tritgridIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 295 bytes, decoded: 295 bytes (100.0%)
; DEoption: 20

0000:    83 C4 B0                add      esp, -0x50
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
004C:    90                      nop
004D:    90                      nop
004E:    90                      nop
004F:    90                      nop
0050:    90                      nop
0051:    83 C7 D4                add      edi, -0x2c
0054:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0058:    DD 44 24 08             fld      qword ptr [esp + 8]
005C:    DD 04 24                fld      qword ptr [esp]
005F:    D9 C0                   fld      st(0)
0061:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0064:    D9 C2                   fld      st(2)
0066:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0069:    DE C1                   faddp    st(1)
006B:    D9 C3                   fld      st(3)
006D:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0070:    DE C1                   faddp    st(1)
0072:    D9 C1                   fld      st(1)
0074:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0077:    D9 C3                   fld      st(3)
0079:    D8 4F D8                fmul     dword ptr [edi - 0x28]
007C:    DE C1                   faddp    st(1)
007E:    D9 C4                   fld      st(4)
0080:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0083:    DE C1                   faddp    st(1)
0085:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0087:    D8 4F D0                fmul     dword ptr [edi - 0x30]
008A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008C:    D8 4F CC                fmul     dword ptr [edi - 0x34]
008F:    DE C3                   faddp    st(3)
0091:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0093:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0096:    DE C2                   faddp    st(2)
0098:    DD 1C 24                fstp     qword ptr [esp]
009B:    DD 5C 24 08             fstp     qword ptr [esp + 8]
009F:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A3:    83 C7 2C                add      edi, 0x2c
00A6:    DD 04 24                fld      qword ptr [esp]
00A9:    DC 6F B8                fsubr    qword ptr [edi - 0x48]
00AC:    D9 C0                   fld      st(0)
00AE:    DB 5C 24 20             fistp    dword ptr [esp + 0x20]
00B2:    DB 44 24 20             fild     dword ptr [esp + 0x20]
00B6:    DE E9                   fsubp    st(1)
00B8:    D9 E1                   fabs    ; |ST(0)|
00BA:    90                      nop
00BB:    DD 04 24                fld      qword ptr [esp]
00BE:    DC 0F                   fmul     qword ptr [edi]
00C0:    DD 44 24 08             fld      qword ptr [esp + 8]
00C4:    DC 4F 08                fmul     qword ptr [edi + 8]
00C7:    D9 C1                   fld      st(1)
00C9:    D8 C1                   fadd     st(1)
00CB:    D9 C0                   fld      st(0)
00CD:    DB 5C 24 24             fistp    dword ptr [esp + 0x24]
00D1:    DB 44 24 24             fild     dword ptr [esp + 0x24]
00D5:    DE E9                   fsubp    st(1)
00D7:    D9 E1                   fabs    ; |ST(0)|
00D9:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00DB:    DE E9                   fsubp    st(1)
00DD:    DB 54 24 28             fist     dword ptr [esp + 0x28]
00E1:    DB 44 24 28             fild     dword ptr [esp + 0x28]
00E5:    DE E9                   fsubp    st(1)
00E7:    D9 E1                   fabs    ; |ST(0)|
00E9:    90                      nop
00EA:    D8 D1                   fcom     st(1)
00EC:    9B                      wait
00ED:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00EF:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00F1:    72 04                   jb       0xf7    ; Conditional jump
00F3:    DD D8                   fstp     st(0)
00F5:    EB 02                   jmp      0xf9
00F7:    DD D9                   fstp     st(1)
00F9:    D8 D1                   fcom     st(1)
00FB:    9B                      wait
00FC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00FE:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0100:    72 04                   jb       0x106    ; Conditional jump
0102:    DD D8                   fstp     st(0)
0104:    EB 02                   jmp      0x108
0106:    DD D9                   fstp     st(1)
0108:    D8 C8                   fmul     st(0)
010A:    DD 44 24 10             fld      qword ptr [esp + 0x10]
010E:    D8 C8                   fmul     st(0)
0110:    DE C1                   faddp    st(1)
0112:    D9 FA                   fsqrt    ; sqrt(ST(0))
0114:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0117:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
011B:    75 03                   jne      0x120    ; Conditional jump
011D:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0120:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0123:    83 C4 50                add      esp, 0x50
0126:    C3                      ret
