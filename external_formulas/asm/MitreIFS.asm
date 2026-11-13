; Disassembly: MitreIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 327 bytes, decoded: 327 bytes (100.0%)
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
00AD:    D8 C8                   fmul     st(0)
00AF:    DD 44 24 08             fld      qword ptr [esp + 8]
00B3:    D8 C8                   fmul     st(0)
00B5:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00B9:    D8 C8                   fmul     st(0)
00BB:    DE C1                   faddp    st(1)
00BD:    DE C1                   faddp    st(1)
00BF:    DD 54 24 28             fst      qword ptr [esp + 0x28]
00C3:    90                      nop
00C4:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00C7:    DC 0C 24                fmul     qword ptr [esp]
00CA:    DC 0C 24                fmul     qword ptr [esp]
00CD:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00D1:    DD 47 F0                fld      qword ptr [edi - 0x10]
00D4:    90                      nop
00D5:    DD 44 24 08             fld      qword ptr [esp + 8]
00D9:    D8 C8                   fmul     st(0)
00DB:    D9 E0                   fchs    ; -ST(0)
00DD:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00E1:    D8 C8                   fmul     st(0)
00E3:    D9 E0                   fchs    ; -ST(0)
00E5:    DE C1                   faddp    st(1)
00E7:    DE C1                   faddp    st(1)
00E9:    DC 4C 24 08             fmul     qword ptr [esp + 8]
00ED:    DC 4C 24 08             fmul     qword ptr [esp + 8]
00F1:    D9 E4                   ftst
00F3:    9B                      wait
00F4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00F8:    73 0A                   jae      0x104    ; Conditional jump
00FA:    DC 6C 24 18             fsubr    qword ptr [esp + 0x18]
00FE:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0102:    EB 08                   jmp      0x10c
0104:    DC 44 24 20             fadd     qword ptr [esp + 0x20]
0108:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
010C:    DD 44 24 18             fld      qword ptr [esp + 0x18]
0110:    D9 FA                   fsqrt    ; sqrt(ST(0))
0112:    D9 FA                   fsqrt    ; sqrt(ST(0))
0114:    DD 44 24 20             fld      qword ptr [esp + 0x20]
0118:    D9 FA                   fsqrt    ; sqrt(ST(0))
011A:    D9 FA                   fsqrt    ; sqrt(ST(0))
011C:    D9 E0                   fchs    ; -ST(0)
011E:    DE C1                   faddp    st(1)
0120:    DD 44 24 28             fld      qword ptr [esp + 0x28]
0124:    D9 FA                   fsqrt    ; sqrt(ST(0))
0126:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0129:    D9 C0                   fld      st(0)
012B:    D9 E1                   fabs    ; |ST(0)|
012D:    DE C1                   faddp    st(1)
012F:    D8 C0                   fadd     st(0)
0131:    D8 C0                   fadd     st(0)
0133:    DE C1                   faddp    st(1)
0135:    DC 0F                   fmul     qword ptr [edi]
0137:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
013B:    75 03                   jne      0x140    ; Conditional jump
013D:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0140:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0143:    83 C4 46                add      esp, 0x46
0146:    C3                      ret
