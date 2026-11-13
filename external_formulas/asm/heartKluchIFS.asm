; Disassembly: heartKluchIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 315 bytes, decoded: 315 bytes (100.0%)
; DEoption: 20

0000:    83 C4 CC                add      esp, -0x34
0003:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0008:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000D:    66 0F 59 47 E0          mulpd    xmm0, xmmword ptr [edi - 0x20]
0012:    F2 0F 59 4F E0          mulsd    xmm1, qword ptr [edi - 0x20]
0017:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
001C:    F2 0F 58 4F D0          addsd    xmm1, qword ptr [edi - 0x30]
0021:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
0025:    74 19                   je       0x40    ; Conditional jump
0027:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002C:    F2 0F 59 6F E0          mulsd    xmm5, qword ptr [edi - 0x20]
0031:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0036:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
003B:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0040:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0045:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
004B:    83 C7 D4                add      edi, -0x2c
004E:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0052:    DD 44 24 08             fld      qword ptr [esp + 8]
0056:    DD 04 24                fld      qword ptr [esp]
0059:    D9 C0                   fld      st(0)
005B:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
005E:    D9 C2                   fld      st(2)
0060:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0063:    DE C1                   faddp    st(1)
0065:    D9 C3                   fld      st(3)
0067:    D8 4F BC                fmul     dword ptr [edi - 0x44]
006A:    DE C1                   faddp    st(1)
006C:    D9 C1                   fld      st(1)
006E:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0071:    D9 C3                   fld      st(3)
0073:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0076:    DE C1                   faddp    st(1)
0078:    D9 C4                   fld      st(4)
007A:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
007D:    DE C1                   faddp    st(1)
007F:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0081:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0084:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0086:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0089:    DE C3                   faddp    st(3)
008B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008D:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0090:    DE C2                   faddp    st(2)
0092:    DD 1C 24                fstp     qword ptr [esp]
0095:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0099:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
009D:    83 C7 2C                add      edi, 0x2c
00A0:    DD 44 24 08             fld      qword ptr [esp + 8]
00A4:    DC 27                   fsub     qword ptr [edi]
00A6:    D9 E1                   fabs    ; |ST(0)|
00A8:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00AC:    DD 04 24                fld      qword ptr [esp]
00AF:    D8 C8                   fmul     st(0)
00B1:    DD 54 24 10             fst      qword ptr [esp + 0x10]
00B5:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00B7:    D8 C8                   fmul     st(0)
00B9:    DD 14 24                fst      qword ptr [esp]
00BC:    DD 44 24 08             fld      qword ptr [esp + 8]
00C0:    D8 C8                   fmul     st(0)
00C2:    DE C1                   faddp    st(1)
00C4:    DE C1                   faddp    st(1)
00C6:    D9 FA                   fsqrt    ; sqrt(ST(0))
00C8:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00CA:    D8 C8                   fmul     st(0)
00CC:    D8 C8                   fmul     st(0)
00CE:    DC 47 08                fadd     qword ptr [edi + 8]
00D1:    DD 04 24                fld      qword ptr [esp]
00D4:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00D7:    DE F1                   fdivrp   st(1)
00D9:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
00DD:    D9 FA                   fsqrt    ; sqrt(ST(0))
00DF:    D9 FA                   fsqrt    ; sqrt(ST(0))
00E1:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00E4:    DD 44 24 08             fld      qword ptr [esp + 8]
00E8:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00EB:    DE C1                   faddp    st(1)
00ED:    D9 E1                   fabs    ; |ST(0)|
00EF:    D8 C8                   fmul     st(0)
00F1:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
00F5:    DD 04 24                fld      qword ptr [esp]
00F8:    DC 4F 28                fmul     qword ptr [edi + 0x28]
00FB:    DE C1                   faddp    st(1)
00FD:    D9 FA                   fsqrt    ; sqrt(ST(0))
00FF:    D9 E8                   fld1    ; Load 1.0
0101:    DE E9                   fsubp    st(1)
0103:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0105:    DC 67 30                fsub     qword ptr [edi + 0x30]
0108:    D8 D1                   fcom     st(1)
010A:    9B                      wait
010B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
010D:    80 E4 41                and      ah, 0x41
0110:    75 04                   jne      0x116    ; Conditional jump
0112:    DD D9                   fstp     st(1)
0114:    EB 02                   jmp      0x118
0116:    DD D8                   fstp     st(0)
0118:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
011C:    75 03                   jne      0x121    ; Conditional jump
011E:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0121:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0124:    DD 47 F0                fld      qword ptr [edi - 0x10]
0127:    DB 46 E8                fild     dword ptr [esi - 0x18]
012A:    DC 4F B4                fmul     qword ptr [edi - 0x4c]
012D:    DE C1                   faddp    st(1)
012F:    D9 E1                   fabs    ; |ST(0)|
0131:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0137:    83 C4 34                add      esp, 0x34
013A:    C3                      ret
