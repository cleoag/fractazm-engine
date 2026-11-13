; Disassembly: diniIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 413 bytes, decoded: 413 bytes (100.0%)
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
004B:    DD 47 10                fld      qword ptr [edi + 0x10]
004E:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
0052:    83 7F A0 01             cmp      dword ptr [edi - 0x60], 1
0056:    75 0E                   jne      0x66    ; Conditional jump
0058:    DD 04 24                fld      qword ptr [esp]
005B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
005F:    DD 1C 24                fstp     qword ptr [esp]
0062:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0066:    83 7F A0 FF             cmp      dword ptr [edi - 0x60], -1
006A:    75 10                   jne      0x7c    ; Conditional jump
006C:    DD 44 24 08             fld      qword ptr [esp + 8]
0070:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0074:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0078:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
007C:    83 7F 9C 00             cmp      dword ptr [edi - 0x64], 0
0080:    74 0A                   je       0x8c    ; Conditional jump
0082:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0086:    D9 E0                   fchs    ; -ST(0)
0088:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
008C:    BA 03 00 00 00          mov      edx, 3
0091:    DD 44 24 08             fld      qword ptr [esp + 8]
0095:    D9 E0                   fchs    ; -ST(0)
0097:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
009B:    DD 04 24                fld      qword ptr [esp]
009E:    D9 E0                   fchs    ; -ST(0)
00A0:    DD 5C 24 30             fstp     qword ptr [esp + 0x30]
00A4:    DD 44 24 08             fld      qword ptr [esp + 8]
00A8:    DC 24 24                fsub     qword ptr [esp]
00AB:    DD 44 24 08             fld      qword ptr [esp + 8]
00AF:    DC 04 24                fadd     qword ptr [esp]
00B2:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00B4:    DC 4F 94                fmul     qword ptr [edi - 0x6c]
00B7:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00BB:    DD 04 24                fld      qword ptr [esp]
00BE:    D8 C8                   fmul     st(0)
00C0:    DD 44 24 08             fld      qword ptr [esp + 8]
00C4:    D8 C8                   fmul     st(0)
00C6:    DE C1                   faddp    st(1)
00C8:    DD 54 24 08             fst      qword ptr [esp + 8]
00CC:    D9 FA                   fsqrt    ; sqrt(ST(0))
00CE:    DD 1C 24                fstp     qword ptr [esp]
00D1:    DD 47 F0                fld      qword ptr [edi - 0x10]
00D4:    D8 C8                   fmul     st(0)
00D6:    DC 64 24 08             fsub     qword ptr [esp + 8]
00DA:    D9 C0                   fld      st(0)
00DC:    D9 E1                   fabs    ; |ST(0)|
00DE:    DE C1                   faddp    st(1)
00E0:    D9 FA                   fsqrt    ; sqrt(ST(0))
00E2:    D9 C0                   fld      st(0)
00E4:    DC 47 F0                fadd     qword ptr [edi - 0x10]
00E7:    DC 34 24                fdiv     qword ptr [esp]
00EA:    D9 ED                   fldln2    ; Load ln(2)
00EC:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00EE:    D9 F1                   fyl2x
00F0:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00F3:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00F5:    D9 E0                   fchs    ; -ST(0)
00F7:    DE C1                   faddp    st(1)
00F9:    DC 64 24 18             fsub     qword ptr [esp + 0x18]
00FD:    DC 64 24 10             fsub     qword ptr [esp + 0x10]
0101:    D9 E1                   fabs    ; |ST(0)|
0103:    DC 67 AC                fsub     qword ptr [edi - 0x54]
0106:    DD 04 24                fld      qword ptr [esp]
0109:    DC 0F                   fmul     qword ptr [edi]
010B:    DC 47 08                fadd     qword ptr [edi + 8]
010E:    D9 E1                   fabs    ; |ST(0)|
0110:    DE C9                   fmulp    st(1)
0112:    DD 04 24                fld      qword ptr [esp]
0115:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0118:    D8 D1                   fcom     st(1)
011A:    9B                      wait
011B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
011D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
011F:    73 04                   jae      0x125    ; Conditional jump
0121:    DD D8                   fstp     st(0)
0123:    EB 02                   jmp      0x127
0125:    DD D9                   fstp     st(1)
0127:    DD 04 24                fld      qword ptr [esp]
012A:    DC 6F A4                fsubr    qword ptr [edi - 0x5c]
012D:    D8 D1                   fcom     st(1)
012F:    9B                      wait
0130:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0132:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0134:    73 04                   jae      0x13a    ; Conditional jump
0136:    DD D8                   fstp     st(0)
0138:    EB 02                   jmp      0x13c
013A:    DD D9                   fstp     st(1)
013C:    DD 44 24 20             fld      qword ptr [esp + 0x20]
0140:    D8 D1                   fcom     st(1)
0142:    9B                      wait
0143:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0145:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0147:    73 04                   jae      0x14d    ; Conditional jump
0149:    DD D8                   fstp     st(0)
014B:    EB 02                   jmp      0x14f
014D:    DD D9                   fstp     st(1)
014F:    DC 54 24 28             fcom     qword ptr [esp + 0x28]
0153:    9B                      wait
0154:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0156:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0158:    73 06                   jae      0x160    ; Conditional jump
015A:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
015E:    EB 02                   jmp      0x162
0160:    DD D8                   fstp     st(0)
0162:    DD 44 24 30             fld      qword ptr [esp + 0x30]
0166:    DD 1C 24                fstp     qword ptr [esp]
0169:    DD 44 24 20             fld      qword ptr [esp + 0x20]
016D:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0171:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0175:    DD 47 94                fld      qword ptr [edi - 0x6c]
0178:    D9 EB                   fldpi    ; Load pi
017A:    DE C9                   fmulp    st(1)
017C:    DE C1                   faddp    st(1)
017E:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0182:    4A                      dec      edx
0183:    0F 8F 08 FF FF FF       jg       0x91    ; Conditional jump
0189:    DD 44 24 28             fld      qword ptr [esp + 0x28]
018D:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0191:    75 03                   jne      0x196    ; Conditional jump
0193:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0196:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0199:    83 C4 46                add      esp, 0x46
019C:    C3                      ret
