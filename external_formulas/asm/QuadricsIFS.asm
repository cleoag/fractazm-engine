; Disassembly: QuadricsIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 222 bytes, decoded: 222 bytes (100.0%)
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
004B:    83 7F 9C 00             cmp      dword ptr [edi - 0x64], 0
004F:    74 0E                   je       0x5f    ; Conditional jump
0051:    DD 04 24                fld      qword ptr [esp]
0054:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0058:    DD 1C 24                fstp     qword ptr [esp]
005B:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
005F:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0063:    D8 C8                   fmul     st(0)
0065:    DD 44 24 08             fld      qword ptr [esp + 8]
0069:    D8 C8                   fmul     st(0)
006B:    DD 04 24                fld      qword ptr [esp]
006E:    D8 C8                   fmul     st(0)
0070:    DC 4F AC                fmul     qword ptr [edi - 0x54]
0073:    DE C1                   faddp    st(1)
0075:    DE C1                   faddp    st(1)
0077:    DC 67 A4                fsub     qword ptr [edi - 0x5c]
007A:    D9 E1                   fabs    ; |ST(0)|
007C:    D9 FA                   fsqrt    ; sqrt(ST(0))
007E:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0081:    DD 04 24                fld      qword ptr [esp]
0084:    83 7F A0 00             cmp      dword ptr [edi - 0x60], 0
0088:    75 05                   jne      0x8f    ; Conditional jump
008A:    D9 E1                   fabs    ; |ST(0)|
008C:    DC 67 F0                fsub     qword ptr [edi - 0x10]
008F:    DD 44 24 08             fld      qword ptr [esp + 8]
0093:    D9 E1                   fabs    ; |ST(0)|
0095:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0098:    D8 D1                   fcom     st(1)
009A:    9B                      wait
009B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
009F:    73 04                   jae      0xa5    ; Conditional jump
00A1:    DD D8                   fstp     st(0)
00A3:    EB 02                   jmp      0xa7
00A5:    DD D9                   fstp     st(1)
00A7:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00AB:    D9 E1                   fabs    ; |ST(0)|
00AD:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00B0:    D8 D1                   fcom     st(1)
00B2:    9B                      wait
00B3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B5:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00B7:    73 04                   jae      0xbd    ; Conditional jump
00B9:    DD D8                   fstp     st(0)
00BB:    EB 02                   jmp      0xbf
00BD:    DD D9                   fstp     st(1)
00BF:    D8 D1                   fcom     st(1)
00C1:    9B                      wait
00C2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C4:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00C6:    73 04                   jae      0xcc    ; Conditional jump
00C8:    DD D8                   fstp     st(0)
00CA:    EB 02                   jmp      0xce
00CC:    DD D9                   fstp     st(1)
00CE:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00D2:    75 03                   jne      0xd7    ; Conditional jump
00D4:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00D7:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00DA:    83 C4 1C                add      esp, 0x1c
00DD:    C3                      ret
