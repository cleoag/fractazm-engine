; Disassembly: loxodrome2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 327 bytes, decoded: 327 bytes (100.0%)
; DEoption: 20

0000:    83 C4 D8                add      esp, -0x28
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
004B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
004F:    D9 E1                   fabs    ; |ST(0)|
0051:    DD 47 20                fld      qword ptr [edi + 0x20]
0054:    D8 D9                   fcomp    st(1)
0056:    9B                      wait
0057:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0059:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005B:    73 0A                   jae      0x67    ; Conditional jump
005D:    DD D8                   fstp     st(0)
005F:    DD 47 18                fld      qword ptr [edi + 0x18]
0062:    E9 84 00 00 00          jmp      0xeb
0067:    83 7F 88 00             cmp      dword ptr [edi - 0x78], 0
006B:    7D 05                   jge      0x72    ; Conditional jump
006D:    DC 4F 30                fmul     qword ptr [edi + 0x30]
0070:    D9 FE                   fsin    ; sin(ST(0))
0072:    83 7F 88 01             cmp      dword ptr [edi - 0x78], 1
0076:    74 0C                   je       0x84    ; Conditional jump
0078:    83 7F 88 FF             cmp      dword ptr [edi - 0x78], -1
007C:    74 06                   je       0x84    ; Conditional jump
007E:    D9 E0                   fchs    ; -ST(0)
0080:    D9 E8                   fld1    ; Load 1.0
0082:    DE C1                   faddp    st(1)
0084:    DD 47 B8                fld      qword ptr [edi - 0x48]
0087:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0089:    D9 F1                   fyl2x
008B:    8B 54 24 14             mov      edx, dword ptr [esp + 0x14]
008F:    81 E2 00 00 00 80       and      edx, 0x80000000
0095:    74 02                   je       0x99    ; Conditional jump
0097:    D9 E0                   fchs    ; -ST(0)
0099:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
009C:    D9 FB                   fsincos    ; ST(0)=cos, push sin
009E:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00A2:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00A6:    DD 04 24                fld      qword ptr [esp]
00A9:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00AD:    DD 44 24 08             fld      qword ptr [esp + 8]
00B1:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00B5:    D9 E0                   fchs    ; -ST(0)
00B7:    DE C1                   faddp    st(1)
00B9:    DD 04 24                fld      qword ptr [esp]
00BC:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00C0:    DD 44 24 08             fld      qword ptr [esp + 8]
00C4:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00C8:    DE C1                   faddp    st(1)
00CA:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00CC:    DC 0F                   fmul     qword ptr [edi]
00CE:    DC 77 AC                fdiv     qword ptr [edi - 0x54]
00D1:    D9 C0                   fld      st(0)
00D3:    D9 FC                   frndint    ; round(ST(0))
00D5:    DE E9                   fsubp    st(1)
00D7:    DC 4F AC                fmul     qword ptr [edi - 0x54]
00DA:    D9 E1                   fabs    ; |ST(0)|
00DC:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
00DF:    D9 E8                   fld1    ; Load 1.0
00E1:    DE E9                   fsubp    st(1)
00E3:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00E6:    DC 4F 08                fmul     qword ptr [edi + 8]
00E9:    D9 E0                   fchs    ; -ST(0)
00EB:    DD 04 24                fld      qword ptr [esp]
00EE:    D8 C8                   fmul     st(0)
00F0:    DD 44 24 08             fld      qword ptr [esp + 8]
00F4:    D8 C8                   fmul     st(0)
00F6:    DE C1                   faddp    st(1)
00F8:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00FC:    D8 C8                   fmul     st(0)
00FE:    DC 4F 94                fmul     qword ptr [edi - 0x6c]
0101:    DE C1                   faddp    st(1)
0103:    DC 67 8C                fsub     qword ptr [edi - 0x74]
0106:    D9 E1                   fabs    ; |ST(0)|
0108:    D9 FA                   fsqrt    ; sqrt(ST(0))
010A:    DC 67 9C                fsub     qword ptr [edi - 0x64]
010D:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0110:    D8 D1                   fcom     st(1)
0112:    9B                      wait
0113:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0115:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0117:    73 04                   jae      0x11d    ; Conditional jump
0119:    DD D8                   fstp     st(0)
011B:    EB 02                   jmp      0x11f
011D:    DD D9                   fstp     st(1)
011F:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0123:    D9 E1                   fabs    ; |ST(0)|
0125:    DC 67 28                fsub     qword ptr [edi + 0x28]
0128:    D8 D1                   fcom     st(1)
012A:    9B                      wait
012B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
012D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
012F:    73 04                   jae      0x135    ; Conditional jump
0131:    DD D8                   fstp     st(0)
0133:    EB 02                   jmp      0x137
0135:    DD D9                   fstp     st(1)
0137:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
013B:    75 03                   jne      0x140    ; Conditional jump
013D:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0140:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0143:    83 C4 28                add      esp, 0x28
0146:    C3                      ret
