; Disassembly: gear2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 246 bytes, decoded: 246 bytes (100.0%)
; DEoption: 20

0000:    83 C4 D0                add      esp, -0x30
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
004B:    DD 04 24                fld      qword ptr [esp]
004E:    D8 C8                   fmul     st(0)
0050:    DD 44 24 08             fld      qword ptr [esp + 8]
0054:    D8 C8                   fmul     st(0)
0056:    DE C1                   faddp    st(1)
0058:    D9 FA                   fsqrt    ; sqrt(ST(0))
005A:    DC 67 B8                fsub     qword ptr [edi - 0x48]
005D:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0061:    DD 44 24 08             fld      qword ptr [esp + 8]
0065:    DD 04 24                fld      qword ptr [esp]
0068:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
006A:    DC 0F                   fmul     qword ptr [edi]
006C:    DD 44 24 18             fld      qword ptr [esp + 0x18]
0070:    D9 E4                   ftst
0072:    9B                      wait
0073:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0075:    9E                      sahf
0076:    73 07                   jae      0x7f    ; Conditional jump
0078:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
007A:    DC 67 94                fsub     qword ptr [edi - 0x6c]
007D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
007F:    DD D8                   fstp     st(0)
0081:    DC 77 AC                fdiv     qword ptr [edi - 0x54]
0084:    D9 C0                   fld      st(0)
0086:    D9 FC                   frndint    ; round(ST(0))
0088:    DE E9                   fsubp    st(1)
008A:    DC 4F AC                fmul     qword ptr [edi - 0x54]
008D:    D9 E1                   fabs    ; |ST(0)|
008F:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
0092:    D9 E8                   fld1    ; Load 1.0
0094:    DE E9                   fsubp    st(1)
0096:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0099:    DC 4F 08                fmul     qword ptr [edi + 8]
009C:    D9 E0                   fchs    ; -ST(0)
009E:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00A2:    D9 E1                   fabs    ; |ST(0)|
00A4:    DC 67 A4                fsub     qword ptr [edi - 0x5c]
00A7:    D8 D1                   fcom     st(1)
00A9:    9B                      wait
00AA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00AE:    73 04                   jae      0xb4    ; Conditional jump
00B0:    DD D8                   fstp     st(0)
00B2:    EB 02                   jmp      0xb6
00B4:    DD D9                   fstp     st(1)
00B6:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00BA:    D9 E1                   fabs    ; |ST(0)|
00BC:    DC 67 8C                fsub     qword ptr [edi - 0x74]
00BF:    D8 D1                   fcom     st(1)
00C1:    9B                      wait
00C2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C4:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00C6:    72 04                   jb       0xcc    ; Conditional jump
00C8:    DD D8                   fstp     st(0)
00CA:    EB 02                   jmp      0xce
00CC:    DD D9                   fstp     st(1)
00CE:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00D2:    D9 E1                   fabs    ; |ST(0)|
00D4:    DC 67 9C                fsub     qword ptr [edi - 0x64]
00D7:    D8 D1                   fcom     st(1)
00D9:    9B                      wait
00DA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00DE:    73 04                   jae      0xe4    ; Conditional jump
00E0:    DD D8                   fstp     st(0)
00E2:    EB 02                   jmp      0xe6
00E4:    DD D9                   fstp     st(1)
00E6:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00EA:    75 03                   jne      0xef    ; Conditional jump
00EC:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00EF:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00F2:    83 C4 30                add      esp, 0x30
00F5:    C3                      ret
