; Disassembly: PenN-gonIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 236 bytes, decoded: 236 bytes (100.0%)
; DEoption: 20

0000:    83 C4 C4                add      esp, -0x3c
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
004E:    DD 44 24 08             fld      qword ptr [esp + 8]
0052:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0054:    DD 54 24 28             fst      qword ptr [esp + 0x28]
0058:    DC 4F 18                fmul     qword ptr [edi + 0x18]
005B:    DA 4F AC                fimul    dword ptr [edi - 0x54]
005E:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
0061:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0063:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0067:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
006B:    DD 44 24 28             fld      qword ptr [esp + 0x28]
006F:    DC 6F 10                fsubr    qword ptr [edi + 0x10]
0072:    DC 0F                   fmul     qword ptr [edi]
0074:    DA 4F B0                fimul    dword ptr [edi - 0x50]
0077:    D9 FC                   frndint    ; round(ST(0))
0079:    DA 77 B0                fidiv    dword ptr [edi - 0x50]
007C:    DC 4F 08                fmul     qword ptr [edi + 8]
007F:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0081:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
0085:    DD 5C 24 30             fstp     qword ptr [esp + 0x30]
0089:    DD 04 24                fld      qword ptr [esp]
008C:    DC 4C 24 28             fmul     qword ptr [esp + 0x28]
0090:    DD 44 24 08             fld      qword ptr [esp + 8]
0094:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
0098:    DE C1                   faddp    st(1)
009A:    D9 E1                   fabs    ; |ST(0)|
009C:    DC 67 F0                fsub     qword ptr [edi - 0x10]
009F:    DD 1C 24                fstp     qword ptr [esp]
00A2:    DD 04 24                fld      qword ptr [esp]
00A5:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00A9:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00AD:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00B1:    D9 E0                   fchs    ; -ST(0)
00B3:    DE C1                   faddp    st(1)
00B5:    D9 E1                   fabs    ; |ST(0)|
00B7:    DD 04 24                fld      qword ptr [esp]
00BA:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00BE:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00C2:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00C6:    DE C1                   faddp    st(1)
00C8:    D9 E1                   fabs    ; |ST(0)|
00CA:    D8 D1                   fcom     st(1)
00CC:    9B                      wait
00CD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CF:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D1:    73 04                   jae      0xd7    ; Conditional jump
00D3:    DD D8                   fstp     st(0)
00D5:    EB 02                   jmp      0xd9
00D7:    DD D9                   fstp     st(1)
00D9:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00DC:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00E0:    75 03                   jne      0xe5    ; Conditional jump
00E2:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00E5:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00E8:    83 C4 3C                add      esp, 0x3c
00EB:    C3                      ret
