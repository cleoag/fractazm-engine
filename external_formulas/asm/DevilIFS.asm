; Disassembly: DevilIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 330 bytes, decoded: 330 bytes (100.0%)
; DEoption: 20

0000:    83 C4 BA                add      esp, -0x46
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
004B:    D9 EE                   fldz    ; Load 0.0
004D:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0051:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0055:    83 C7 D4                add      edi, -0x2c
0058:    DD 44 24 10             fld      qword ptr [esp + 0x10]
005C:    DD 44 24 08             fld      qword ptr [esp + 8]
0060:    DD 04 24                fld      qword ptr [esp]
0063:    D9 C0                   fld      st(0)
0065:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0068:    D9 C2                   fld      st(2)
006A:    D8 4F C0                fmul     dword ptr [edi - 0x40]
006D:    DE C1                   faddp    st(1)
006F:    D9 C3                   fld      st(3)
0071:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0074:    DE C1                   faddp    st(1)
0076:    D9 C1                   fld      st(1)
0078:    D8 4F DC                fmul     dword ptr [edi - 0x24]
007B:    D9 C3                   fld      st(3)
007D:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0080:    DE C1                   faddp    st(1)
0082:    D9 C4                   fld      st(4)
0084:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0087:    DE C1                   faddp    st(1)
0089:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
008B:    D8 4F D0                fmul     dword ptr [edi - 0x30]
008E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0090:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0093:    DE C3                   faddp    st(3)
0095:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0097:    D8 4F C8                fmul     dword ptr [edi - 0x38]
009A:    DE C2                   faddp    st(2)
009C:    DD 1C 24                fstp     qword ptr [esp]
009F:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00A3:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A7:    83 C7 2C                add      edi, 0x2c
00AA:    DD 04 24                fld      qword ptr [esp]
00AD:    90                      nop
00AE:    D8 C8                   fmul     st(0)
00B0:    DD 1C 24                fstp     qword ptr [esp]
00B3:    90                      nop
00B4:    DD 44 24 08             fld      qword ptr [esp + 8]
00B8:    D8 C8                   fmul     st(0)
00BA:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00BE:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00C2:    D8 C8                   fmul     st(0)
00C4:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00C8:    DD 04 24                fld      qword ptr [esp]
00CB:    D8 C8                   fmul     st(0)
00CD:    DD 04 24                fld      qword ptr [esp]
00D0:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
00D4:    DC C1                   fadd     st(1), st(0)
00D6:    DE C1                   faddp    st(1)
00D8:    DD 44 24 08             fld      qword ptr [esp + 8]
00DC:    DC 4F 08                fmul     qword ptr [edi + 8]
00DF:    DE C1                   faddp    st(1)
00E1:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00E5:    D8 C8                   fmul     st(0)
00E7:    DE C1                   faddp    st(1)
00E9:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00ED:    DD 04 24                fld      qword ptr [esp]
00F0:    90                      nop
00F1:    DC 0F                   fmul     qword ptr [edi]
00F3:    90                      nop
00F4:    DD 44 24 08             fld      qword ptr [esp + 8]
00F8:    D8 C8                   fmul     st(0)
00FA:    DE C1                   faddp    st(1)
00FC:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0100:    DD 44 24 18             fld      qword ptr [esp + 0x18]
0104:    D9 FA                   fsqrt    ; sqrt(ST(0))
0106:    D9 FA                   fsqrt    ; sqrt(ST(0))
0108:    DD 44 24 20             fld      qword ptr [esp + 0x20]
010C:    D9 FA                   fsqrt    ; sqrt(ST(0))
010E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0110:    D9 E0                   fchs    ; -ST(0)
0112:    DE C1                   faddp    st(1)
0114:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0117:    DD 04 24                fld      qword ptr [esp]
011A:    DD 44 24 08             fld      qword ptr [esp + 8]
011E:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0122:    DE C1                   faddp    st(1)
0124:    DE C1                   faddp    st(1)
0126:    D9 FA                   fsqrt    ; sqrt(ST(0))
0128:    DC 67 F0                fsub     qword ptr [edi - 0x10]
012B:    D8 D1                   fcom     st(1)
012D:    9B                      wait
012E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0130:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0132:    73 04                   jae      0x138    ; Conditional jump
0134:    DD D8                   fstp     st(0)
0136:    EB 02                   jmp      0x13a
0138:    DD D9                   fstp     st(1)
013A:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
013E:    75 03                   jne      0x143    ; Conditional jump
0140:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0143:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0146:    83 C4 46                add      esp, 0x46
0149:    C3                      ret
