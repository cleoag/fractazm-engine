; Disassembly: LimpetIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 292 bytes, decoded: 292 bytes (100.0%)
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
004B:    83 C7 CC                add      edi, -0x34
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
009D:    83 C7 34                add      edi, 0x34
00A0:    90                      nop
00A1:    DD 04 24                fld      qword ptr [esp]
00A4:    D8 C8                   fmul     st(0)
00A6:    DD 44 24 08             fld      qword ptr [esp + 8]
00AA:    D8 C8                   fmul     st(0)
00AC:    DE C1                   faddp    st(1)
00AE:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B0:    DD 1C 24                fstp     qword ptr [esp]
00B3:    90                      nop
00B4:    DD 47 F0                fld      qword ptr [edi - 0x10]
00B7:    DD 04 24                fld      qword ptr [esp]
00BA:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
00BE:    DE C1                   faddp    st(1)
00C0:    90                      nop
00C1:    90                      nop
00C2:    DD 04 24                fld      qword ptr [esp]
00C5:    DC 4F AC                fmul     qword ptr [edi - 0x54]
00C8:    DE C1                   faddp    st(1)
00CA:    D8 C8                   fmul     st(0)
00CC:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00D0:    D8 C8                   fmul     st(0)
00D2:    DE C1                   faddp    st(1)
00D4:    D9 FA                   fsqrt    ; sqrt(ST(0))
00D6:    D9 FA                   fsqrt    ; sqrt(ST(0))
00D8:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00DB:    DD 04 24                fld      qword ptr [esp]
00DE:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00E1:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00E4:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00E7:    D8 D1                   fcom     st(1)
00E9:    9B                      wait
00EA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00EC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00EE:    73 04                   jae      0xf4    ; Conditional jump
00F0:    DD D8                   fstp     st(0)
00F2:    EB 02                   jmp      0xf6
00F4:    DD D9                   fstp     st(1)
00F6:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00FA:    D9 E1                   fabs    ; |ST(0)|
00FC:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00FF:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0102:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0105:    D8 D1                   fcom     st(1)
0107:    9B                      wait
0108:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
010A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
010C:    73 04                   jae      0x112    ; Conditional jump
010E:    DD D8                   fstp     st(0)
0110:    EB 02                   jmp      0x114
0112:    DD D9                   fstp     st(1)
0114:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0118:    75 03                   jne      0x11d    ; Conditional jump
011A:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
011D:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0120:    83 C4 1C                add      esp, 0x1c
0123:    C3                      ret
