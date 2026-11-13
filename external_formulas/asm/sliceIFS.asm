; Disassembly: sliceIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 180 bytes, decoded: 180 bytes (100.0%)
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
0080:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0084:    D8 C8                   fmul     st(0)
0086:    DE C1                   faddp    st(1)
0088:    D9 FA                   fsqrt    ; sqrt(ST(0))
008A:    DC 67 B8                fsub     qword ptr [edi - 0x48]
008D:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
0090:    D9 E1                   fabs    ; |ST(0)|
0092:    DC 67 A4                fsub     qword ptr [edi - 0x5c]
0095:    D8 D1                   fcom     st(1)
0097:    9B                      wait
0098:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
009C:    73 04                   jae      0xa2    ; Conditional jump
009E:    DD D8                   fstp     st(0)
00A0:    EB 02                   jmp      0xa4
00A2:    DD D9                   fstp     st(1)
00A4:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00A8:    75 03                   jne      0xad    ; Conditional jump
00AA:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00AD:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00B0:    83 C4 1C                add      esp, 0x1c
00B3:    C3                      ret
