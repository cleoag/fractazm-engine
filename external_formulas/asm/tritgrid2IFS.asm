; Disassembly: tritgrid2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 417 bytes, decoded: 417 bytes (100.0%)
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
00A0:    DD 04 24                fld      qword ptr [esp]
00A3:    DC 6F B8                fsubr    qword ptr [edi - 0x48]
00A6:    D9 C0                   fld      st(0)
00A8:    DB 54 24 18             fist     dword ptr [esp + 0x18]
00AC:    DC 07                   fadd     qword ptr [edi]
00AE:    DB 5C 24 20             fistp    dword ptr [esp + 0x20]
00B2:    DB 44 24 18             fild     dword ptr [esp + 0x18]
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
00CD:    DB 54 24 18             fist     dword ptr [esp + 0x18]
00D1:    DC 07                   fadd     qword ptr [edi]
00D3:    DB 5C 24 24             fistp    dword ptr [esp + 0x24]
00D7:    DB 44 24 18             fild     dword ptr [esp + 0x18]
00DB:    DE E9                   fsubp    st(1)
00DD:    D9 E1                   fabs    ; |ST(0)|
00DF:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00E1:    DE E9                   fsubp    st(1)
00E3:    D9 C0                   fld      st(0)
00E5:    DB 54 24 18             fist     dword ptr [esp + 0x18]
00E9:    DC 07                   fadd     qword ptr [edi]
00EB:    DB 5C 24 28             fistp    dword ptr [esp + 0x28]
00EF:    DB 44 24 18             fild     dword ptr [esp + 0x18]
00F3:    DE E9                   fsubp    st(1)
00F5:    D9 E1                   fabs    ; |ST(0)|
00F7:    90                      nop
00F8:    89 4C 24 30             mov      dword ptr [esp + 0x30], ecx
00FC:    8B 44 24 28             mov      eax, dword ptr [esp + 0x28]
0100:    05 00 10 00 00          add      eax, 0x1000
0105:    33 D2                   xor      edx, edx
0107:    B9 03 00 00 00          mov      ecx, 3
010C:    F7 F1                   div      ecx
010E:    90                      nop
010F:    90                      nop
0110:    90                      nop
0111:    84 D2                   test     dl, dl
0113:    90                      nop
0114:    75 07                   jne      0x11d    ; Conditional jump
0116:    DD 47 10                fld      qword ptr [edi + 0x10]
0119:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
011B:    DD D8                   fstp     st(0)
011D:    8B 44 24 20             mov      eax, dword ptr [esp + 0x20]
0121:    05 00 10 00 00          add      eax, 0x1000
0126:    33 D2                   xor      edx, edx
0128:    B9 03 00 00 00          mov      ecx, 3
012D:    F7 F1                   div      ecx
012F:    90                      nop
0130:    90                      nop
0131:    90                      nop
0132:    84 D2                   test     dl, dl
0134:    90                      nop
0135:    75 07                   jne      0x13e    ; Conditional jump
0137:    DD 47 10                fld      qword ptr [edi + 0x10]
013A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
013C:    DD D8                   fstp     st(0)
013E:    8B 44 24 24             mov      eax, dword ptr [esp + 0x24]
0142:    05 00 10 00 00          add      eax, 0x1000
0147:    33 D2                   xor      edx, edx
0149:    B9 03 00 00 00          mov      ecx, 3
014E:    F7 F1                   div      ecx
0150:    90                      nop
0151:    90                      nop
0152:    90                      nop
0153:    84 D2                   test     dl, dl
0155:    90                      nop
0156:    75 07                   jne      0x15f    ; Conditional jump
0158:    DD 47 10                fld      qword ptr [edi + 0x10]
015B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
015D:    DD D8                   fstp     st(0)
015F:    90                      nop
0160:    8B 4C 24 30             mov      ecx, dword ptr [esp + 0x30]
0164:    D8 D1                   fcom     st(1)
0166:    9B                      wait
0167:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0169:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
016B:    72 04                   jb       0x171    ; Conditional jump
016D:    DD D8                   fstp     st(0)
016F:    EB 02                   jmp      0x173
0171:    DD D9                   fstp     st(1)
0173:    D8 D1                   fcom     st(1)
0175:    9B                      wait
0176:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0178:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
017A:    72 04                   jb       0x180    ; Conditional jump
017C:    DD D8                   fstp     st(0)
017E:    EB 02                   jmp      0x182
0180:    DD D9                   fstp     st(1)
0182:    D8 C8                   fmul     st(0)
0184:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0188:    D8 C8                   fmul     st(0)
018A:    DE C1                   faddp    st(1)
018C:    D9 FA                   fsqrt    ; sqrt(ST(0))
018E:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0191:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0195:    75 03                   jne      0x19a    ; Conditional jump
0197:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
019A:    DD 5E E0                fstp     qword ptr [esi - 0x20]
019D:    83 C4 50                add      esp, 0x50
01A0:    C3                      ret
