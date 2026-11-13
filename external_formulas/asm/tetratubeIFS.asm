; Disassembly: tetratubeIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 364 bytes, decoded: 364 bytes (100.0%)
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
004E:    DD 44 24 08             fld      qword ptr [esp + 8]
0052:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0056:    D9 E0                   fchs    ; -ST(0)
0058:    D8 D1                   fcom     st(1)
005A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005E:    72 02                   jb       0x62    ; Conditional jump
0060:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0062:    D8 D2                   fcom     st(2)
0064:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0066:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0068:    72 02                   jb       0x6c    ; Conditional jump
006A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
006C:    D9 E0                   fchs    ; -ST(0)
006E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0070:    D9 E0                   fchs    ; -ST(0)
0072:    D8 D2                   fcom     st(2)
0074:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0076:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0078:    72 02                   jb       0x7c    ; Conditional jump
007A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007C:    D9 E0                   fchs    ; -ST(0)
007E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0080:    D8 D1                   fcom     st(1)
0082:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0084:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0086:    73 02                   jae      0x8a    ; Conditional jump
0088:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
008A:    D8 D2                   fcom     st(2)
008C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
008E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0090:    73 02                   jae      0x94    ; Conditional jump
0092:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0094:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0096:    D8 D2                   fcom     st(2)
0098:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
009C:    73 02                   jae      0xa0    ; Conditional jump
009E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00A0:    90                      nop
00A1:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A5:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00A9:    DD 1C 24                fstp     qword ptr [esp]
00AC:    DD 04 24                fld      qword ptr [esp]
00AF:    DC 4F B0                fmul     qword ptr [edi - 0x50]
00B2:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00B6:    DC 4F A8                fmul     qword ptr [edi - 0x58]
00B9:    DE C1                   faddp    st(1)
00BB:    DD 04 24                fld      qword ptr [esp]
00BE:    DC 4F A8                fmul     qword ptr [edi - 0x58]
00C1:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00C5:    DC 4F B0                fmul     qword ptr [edi - 0x50]
00C8:    D9 E0                   fchs    ; -ST(0)
00CA:    DE C1                   faddp    st(1)
00CC:    DD 1C 24                fstp     qword ptr [esp]
00CF:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00D3:    DD 04 24                fld      qword ptr [esp]
00D6:    DC 4F A0                fmul     qword ptr [edi - 0x60]
00D9:    DD 44 24 08             fld      qword ptr [esp + 8]
00DD:    DC 4F 98                fmul     qword ptr [edi - 0x68]
00E0:    DE C1                   faddp    st(1)
00E2:    DD 04 24                fld      qword ptr [esp]
00E5:    DC 4F 98                fmul     qword ptr [edi - 0x68]
00E8:    DD 44 24 08             fld      qword ptr [esp + 8]
00EC:    DC 4F A0                fmul     qword ptr [edi - 0x60]
00EF:    D9 E0                   fchs    ; -ST(0)
00F1:    DE C1                   faddp    st(1)
00F3:    DD 1C 24                fstp     qword ptr [esp]
00F6:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00FA:    DD 44 24 08             fld      qword ptr [esp + 8]
00FE:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0101:    DD 54 24 08             fst      qword ptr [esp + 8]
0105:    D8 C8                   fmul     st(0)
0107:    DD 44 24 10             fld      qword ptr [esp + 0x10]
010B:    DC 67 E0                fsub     qword ptr [edi - 0x20]
010E:    DD 54 24 10             fst      qword ptr [esp + 0x10]
0112:    D8 C8                   fmul     st(0)
0114:    DE C1                   faddp    st(1)
0116:    D9 FA                   fsqrt    ; sqrt(ST(0))
0118:    DC 67 F0                fsub     qword ptr [edi - 0x10]
011B:    DD 04 24                fld      qword ptr [esp]
011E:    D9 E1                   fabs    ; |ST(0)|
0120:    DC 67 8C                fsub     qword ptr [edi - 0x74]
0123:    D8 C8                   fmul     st(0)
0125:    DD 44 24 08             fld      qword ptr [esp + 8]
0129:    D8 C8                   fmul     st(0)
012B:    DE C1                   faddp    st(1)
012D:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0131:    D8 C8                   fmul     st(0)
0133:    DE C1                   faddp    st(1)
0135:    D9 FA                   fsqrt    ; sqrt(ST(0))
0137:    DC 67 84                fsub     qword ptr [edi - 0x7c]
013A:    D8 D1                   fcom     st(1)
013C:    9B                      wait
013D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
013F:    80 E4 41                and      ah, 0x41
0142:    74 0A                   je       0x14e    ; Conditional jump
0144:    DD D9                   fstp     st(1)
0146:    DD 87 74 FF FF FF       fld      qword ptr [edi - 0x8c]
014C:    EB 08                   jmp      0x156
014E:    DD D8                   fstp     st(0)
0150:    DD 87 7C FF FF FF       fld      qword ptr [edi - 0x84]
0156:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
015C:    83 7F 94 00             cmp      dword ptr [edi - 0x6c], 0
0160:    75 03                   jne      0x165    ; Conditional jump
0162:    DC 77 D0                fdiv     qword ptr [edi - 0x30]
0165:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0168:    83 C4 1C                add      esp, 0x1c
016B:    C3                      ret
