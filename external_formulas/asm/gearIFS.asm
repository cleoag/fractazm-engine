; Disassembly: gearIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 230 bytes, decoded: 230 bytes (100.0%)
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
004B:    DD 44 24 08             fld      qword ptr [esp + 8]
004F:    DD 04 24                fld      qword ptr [esp]
0052:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0054:    DC 0F                   fmul     qword ptr [edi]
0056:    DC 77 AC                fdiv     qword ptr [edi - 0x54]
0059:    D9 C0                   fld      st(0)
005B:    D9 FC                   frndint    ; round(ST(0))
005D:    DE E9                   fsubp    st(1)
005F:    DC 4F AC                fmul     qword ptr [edi - 0x54]
0062:    D9 E1                   fabs    ; |ST(0)|
0064:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
0067:    D9 E8                   fld1    ; Load 1.0
0069:    DE E9                   fsubp    st(1)
006B:    DC 4F F0                fmul     qword ptr [edi - 0x10]
006E:    DC 4F 08                fmul     qword ptr [edi + 8]
0071:    D9 E0                   fchs    ; -ST(0)
0073:    DD 04 24                fld      qword ptr [esp]
0076:    D8 C8                   fmul     st(0)
0078:    DD 44 24 08             fld      qword ptr [esp + 8]
007C:    D8 C8                   fmul     st(0)
007E:    DE C1                   faddp    st(1)
0080:    D9 FA                   fsqrt    ; sqrt(ST(0))
0082:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0086:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0089:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
008C:    D9 E1                   fabs    ; |ST(0)|
008E:    DC 67 A4                fsub     qword ptr [edi - 0x5c]
0091:    D8 D1                   fcom     st(1)
0093:    9B                      wait
0094:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0096:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0098:    73 04                   jae      0x9e    ; Conditional jump
009A:    DD D8                   fstp     st(0)
009C:    EB 02                   jmp      0xa0
009E:    DD D9                   fstp     st(1)
00A0:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00A4:    DC 67 94                fsub     qword ptr [edi - 0x6c]
00A7:    DC 47 8C                fadd     qword ptr [edi - 0x74]
00AA:    D9 E1                   fabs    ; |ST(0)|
00AC:    DC 67 8C                fsub     qword ptr [edi - 0x74]
00AF:    D8 D1                   fcom     st(1)
00B1:    9B                      wait
00B2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B4:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00B6:    72 04                   jb       0xbc    ; Conditional jump
00B8:    DD D8                   fstp     st(0)
00BA:    EB 02                   jmp      0xbe
00BC:    DD D9                   fstp     st(1)
00BE:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00C2:    D9 E1                   fabs    ; |ST(0)|
00C4:    DC 67 9C                fsub     qword ptr [edi - 0x64]
00C7:    D8 D1                   fcom     st(1)
00C9:    9B                      wait
00CA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00CE:    73 04                   jae      0xd4    ; Conditional jump
00D0:    DD D8                   fstp     st(0)
00D2:    EB 02                   jmp      0xd6
00D4:    DD D9                   fstp     st(1)
00D6:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00DA:    75 03                   jne      0xdf    ; Conditional jump
00DC:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00DF:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00E2:    83 C4 30                add      esp, 0x30
00E5:    C3                      ret
