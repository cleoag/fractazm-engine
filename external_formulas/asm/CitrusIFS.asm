; Disassembly: CitrusIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 345 bytes, decoded: 345 bytes (100.0%)
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
00AA:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00AE:    D8 C8                   fmul     st(0)
00B0:    DD 44 24 08             fld      qword ptr [esp + 8]
00B4:    D8 C8                   fmul     st(0)
00B6:    DE C1                   faddp    st(1)
00B8:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00BB:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00BF:    DD 04 24                fld      qword ptr [esp]
00C2:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00C5:    D9 C0                   fld      st(0)
00C7:    D8 C8                   fmul     st(0)
00C9:    DE C9                   fmulp    st(1)
00CB:    DC 0C 24                fmul     qword ptr [esp]
00CE:    DC 0C 24                fmul     qword ptr [esp]
00D1:    DC 0C 24                fmul     qword ptr [esp]
00D4:    90                      nop
00D5:    D9 E4                   ftst
00D7:    9B                      wait
00D8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DA:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00DC:    73 0A                   jae      0xe8    ; Conditional jump
00DE:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
00E2:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00E6:    EB 08                   jmp      0xf0
00E8:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
00EC:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00F0:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00F4:    D9 FA                   fsqrt    ; sqrt(ST(0))
00F6:    D9 FA                   fsqrt    ; sqrt(ST(0))
00F8:    DD 44 24 20             fld      qword ptr [esp + 0x20]
00FC:    D9 FA                   fsqrt    ; sqrt(ST(0))
00FE:    D9 FA                   fsqrt    ; sqrt(ST(0))
0100:    D9 E0                   fchs    ; -ST(0)
0102:    DE C1                   faddp    st(1)
0104:    DD 04 24                fld      qword ptr [esp]
0107:    D8 C8                   fmul     st(0)
0109:    DD 44 24 08             fld      qword ptr [esp + 8]
010D:    D8 C8                   fmul     st(0)
010F:    DE C1                   faddp    st(1)
0111:    D9 FA                   fsqrt    ; sqrt(ST(0))
0113:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0116:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0119:    DC 67 B8                fsub     qword ptr [edi - 0x48]
011C:    D8 D1                   fcom     st(1)
011E:    9B                      wait
011F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0121:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0123:    73 04                   jae      0x129    ; Conditional jump
0125:    DD D8                   fstp     st(0)
0127:    EB 02                   jmp      0x12b
0129:    DD D9                   fstp     st(1)
012B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
012F:    D9 E1                   fabs    ; |ST(0)|
0131:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0134:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0137:    DC 67 B8                fsub     qword ptr [edi - 0x48]
013A:    D8 D1                   fcom     st(1)
013C:    9B                      wait
013D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
013F:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0141:    73 04                   jae      0x147    ; Conditional jump
0143:    DD D8                   fstp     st(0)
0145:    EB 02                   jmp      0x149
0147:    DD D9                   fstp     st(1)
0149:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
014D:    75 03                   jne      0x152    ; Conditional jump
014F:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0152:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0155:    83 C4 46                add      esp, 0x46
0158:    C3                      ret
