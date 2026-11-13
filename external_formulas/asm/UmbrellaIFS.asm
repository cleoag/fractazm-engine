; Disassembly: UmbrellaIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 259 bytes, decoded: 259 bytes (100.0%)
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
004B:    83 7F B0 00             cmp      dword ptr [edi - 0x50], 0
004F:    74 0E                   je       0x5f    ; Conditional jump
0051:    DD 04 24                fld      qword ptr [esp]
0054:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0058:    DD 1C 24                fstp     qword ptr [esp]
005B:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
005F:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0063:    DC 4C 24 08             fmul     qword ptr [esp + 8]
0067:    DC 4C 24 08             fmul     qword ptr [esp + 8]
006B:    DD 04 24                fld      qword ptr [esp]
006E:    D8 C8                   fmul     st(0)
0070:    DE E1                   fsubrp   st(1)
0072:    DC 47 B8                fadd     qword ptr [edi - 0x48]
0075:    D9 E1                   fabs    ; |ST(0)|
0077:    DC 67 B8                fsub     qword ptr [edi - 0x48]
007A:    DC 0F                   fmul     qword ptr [edi]
007C:    D9 E8                   fld1    ; Load 1.0
007E:    D8 D9                   fcomp    st(1)
0080:    9B                      wait
0081:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0083:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0085:    73 04                   jae      0x8b    ; Conditional jump
0087:    D9 FA                   fsqrt    ; sqrt(ST(0))
0089:    D9 FA                   fsqrt    ; sqrt(ST(0))
008B:    DD 04 24                fld      qword ptr [esp]
008E:    D9 E1                   fabs    ; |ST(0)|
0090:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0093:    DD 44 24 08             fld      qword ptr [esp + 8]
0097:    D9 E1                   fabs    ; |ST(0)|
0099:    DC 67 F0                fsub     qword ptr [edi - 0x10]
009C:    D8 D1                   fcom     st(1)
009E:    9B                      wait
009F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A1:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A3:    73 04                   jae      0xa9    ; Conditional jump
00A5:    DD D8                   fstp     st(0)
00A7:    EB 02                   jmp      0xab
00A9:    DD D9                   fstp     st(1)
00AB:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00AF:    D9 E1                   fabs    ; |ST(0)|
00B1:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00B4:    D8 D1                   fcom     st(1)
00B6:    9B                      wait
00B7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B9:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00BB:    73 04                   jae      0xc1    ; Conditional jump
00BD:    DD D8                   fstp     st(0)
00BF:    EB 02                   jmp      0xc3
00C1:    DD D9                   fstp     st(1)
00C3:    D8 D1                   fcom     st(1)
00C5:    9B                      wait
00C6:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C8:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00CA:    73 04                   jae      0xd0    ; Conditional jump
00CC:    DD D8                   fstp     st(0)
00CE:    EB 02                   jmp      0xd2
00D0:    DD D9                   fstp     st(1)
00D2:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00D6:    75 03                   jne      0xdb    ; Conditional jump
00D8:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00DB:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00DE:    DD 44 24 08             fld      qword ptr [esp + 8]
00E2:    DC 0C 24                fmul     qword ptr [esp]
00E5:    DC 64 24 10             fsub     qword ptr [esp + 0x10]
00E9:    DC 4F A8                fmul     qword ptr [edi - 0x58]
00EC:    DB 46 E8                fild     dword ptr [esi - 0x18]
00EF:    DC 4F A0                fmul     qword ptr [edi - 0x60]
00F2:    DE C1                   faddp    st(1)
00F4:    DC 47 98                fadd     qword ptr [edi - 0x68]
00F7:    D9 E1                   fabs    ; |ST(0)|
00F9:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00FF:    83 C4 1C                add      esp, 0x1c
0102:    C3                      ret
