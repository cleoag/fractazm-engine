; Disassembly: CommaIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 300 bytes, decoded: 300 bytes (100.0%)
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
004B:    83 7F A0 00             cmp      dword ptr [edi - 0x60], 0
004F:    74 0E                   je       0x5f    ; Conditional jump
0051:    DD 04 24                fld      qword ptr [esp]
0054:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0058:    DD 1C 24                fstp     qword ptr [esp]
005B:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
005F:    83 7F 9C 00             cmp      dword ptr [edi - 0x64], 0
0063:    74 10                   je       0x75    ; Conditional jump
0065:    DD 44 24 08             fld      qword ptr [esp + 8]
0069:    DD 44 24 10             fld      qword ptr [esp + 0x10]
006D:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0071:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0075:    8B 57 98                mov      edx, dword ptr [edi - 0x68]
0078:    83 E2 01                and      edx, 1
007B:    75 08                   jne      0x85    ; Conditional jump
007D:    DD 04 24                fld      qword ptr [esp]
0080:    D9 E0                   fchs    ; -ST(0)
0082:    DD 1C 24                fstp     qword ptr [esp]
0085:    8B 57 98                mov      edx, dword ptr [edi - 0x68]
0088:    83 E2 02                and      edx, 2
008B:    75 0A                   jne      0x97    ; Conditional jump
008D:    DD 44 24 08             fld      qword ptr [esp + 8]
0091:    D9 E0                   fchs    ; -ST(0)
0093:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0097:    8B 57 98                mov      edx, dword ptr [edi - 0x68]
009A:    83 E2 04                and      edx, 4
009D:    75 0A                   jne      0xa9    ; Conditional jump
009F:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00A3:    D9 E0                   fchs    ; -ST(0)
00A5:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A9:    DD 04 24                fld      qword ptr [esp]
00AC:    D8 C8                   fmul     st(0)
00AE:    DD 44 24 08             fld      qword ptr [esp + 8]
00B2:    D8 C8                   fmul     st(0)
00B4:    DE C1                   faddp    st(1)
00B6:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B8:    DD 44 24 08             fld      qword ptr [esp + 8]
00BC:    DD 04 24                fld      qword ptr [esp]
00BF:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00C1:    DC 0F                   fmul     qword ptr [edi]
00C3:    D9 C0                   fld      st(0)
00C5:    D8 C8                   fmul     st(0)
00C7:    D9 C0                   fld      st(0)
00C9:    D8 CA                   fmul     st(2)
00CB:    D9 E0                   fchs    ; -ST(0)
00CD:    DE C1                   faddp    st(1)
00CF:    DE C1                   faddp    st(1)
00D1:    DD 47 F0                fld      qword ptr [edi - 0x10]
00D4:    DE C9                   fmulp    st(1)
00D6:    DC 47 B8                fadd     qword ptr [edi - 0x48]
00D9:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00DD:    DC 67 AC                fsub     qword ptr [edi - 0x54]
00E0:    D8 C8                   fmul     st(0)
00E2:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00E6:    D8 C8                   fmul     st(0)
00E8:    DE C1                   faddp    st(1)
00EA:    D9 FA                   fsqrt    ; sqrt(ST(0))
00EC:    DC 64 24 18             fsub     qword ptr [esp + 0x18]
00F0:    DD 04 24                fld      qword ptr [esp]
00F3:    DC 47 AC                fadd     qword ptr [edi - 0x54]
00F6:    D8 C8                   fmul     st(0)
00F8:    DD 44 24 08             fld      qword ptr [esp + 8]
00FC:    D8 C8                   fmul     st(0)
00FE:    DE C1                   faddp    st(1)
0100:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0104:    D8 C8                   fmul     st(0)
0106:    DE C1                   faddp    st(1)
0108:    D9 FA                   fsqrt    ; sqrt(ST(0))
010A:    DC 67 A4                fsub     qword ptr [edi - 0x5c]
010D:    D8 D1                   fcom     st(1)
010F:    9B                      wait
0110:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0112:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0114:    73 04                   jae      0x11a    ; Conditional jump
0116:    DD D9                   fstp     st(1)
0118:    EB 02                   jmp      0x11c
011A:    DD D8                   fstp     st(0)
011C:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0120:    75 03                   jne      0x125    ; Conditional jump
0122:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0125:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0128:    83 C4 3C                add      esp, 0x3c
012B:    C3                      ret
