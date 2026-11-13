; Disassembly: gnarly2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 312 bytes, decoded: 312 bytes (100.0%)
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
004B:    8B 57 AC                mov      edx, dword ptr [edi - 0x54]
004E:    83 E2 01                and      edx, 1
0051:    C1 E2 04                shl      edx, 4
0054:    DB 47 B0                fild     dword ptr [edi - 0x50]
0057:    DB 5C 24 18             fistp    dword ptr [esp + 0x18]
005B:    90                      nop
005C:    DD 44 14 10             fld      qword ptr [esp + edx + 0x10]
0060:    D9 E1                   fabs    ; |ST(0)|
0062:    DD 47 B8                fld      qword ptr [edi - 0x48]
0065:    D9 E1                   fabs    ; |ST(0)|
0067:    DC 47 08                fadd     qword ptr [edi + 8]
006A:    DC 0F                   fmul     qword ptr [edi]
006C:    DC 86 78 FF FF FF       fadd     qword ptr [esi - 0x88]
0072:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0074:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0076:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0078:    73 0B                   jae      0x85    ; Conditional jump
007A:    DD 44 14 10             fld      qword ptr [esp + edx + 0x10]
007E:    D9 E0                   fchs    ; -ST(0)
0080:    E9 99 00 00 00          jmp      0x11e
0085:    DD 44 24 08             fld      qword ptr [esp + 8]
0089:    DD 04 14                fld      qword ptr [esp + edx]
008C:    D9 C0                   fld      st(0)
008E:    D9 FF                   fcos    ; cos(ST(0))
0090:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0093:    D8 C1                   fadd     st(1)
0095:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
0098:    D9 FE                   fsin    ; sin(ST(0))
009A:    D9 C1                   fld      st(1)
009C:    D8 E3                   fsub     st(3)
009E:    DC 4F 94                fmul     qword ptr [edi - 0x6c]
00A1:    DE C1                   faddp    st(1)
00A3:    D9 FE                   fsin    ; sin(ST(0))
00A5:    DC 4F 9C                fmul     qword ptr [edi - 0x64]
00A8:    D8 C2                   fadd     st(2)
00AA:    D9 D0                   fnop
00AC:    D9 C2                   fld      st(2)
00AE:    D9 FF                   fcos    ; cos(ST(0))
00B0:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00B3:    D8 C3                   fadd     st(3)
00B5:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
00B8:    D9 FE                   fsin    ; sin(ST(0))
00BA:    D9 C2                   fld      st(2)
00BC:    D8 C4                   fadd     st(4)
00BE:    DC 4F 94                fmul     qword ptr [edi - 0x6c]
00C1:    DE C1                   faddp    st(1)
00C3:    D9 FE                   fsin    ; sin(ST(0))
00C5:    DC 4F 9C                fmul     qword ptr [edi - 0x64]
00C8:    D8 C2                   fadd     st(2)
00CA:    DD DA                   fstp     st(2)
00CC:    DD DA                   fstp     st(2)
00CE:    FF 4C 24 18             dec      dword ptr [esp + 0x18]
00D2:    7F B8                   jg       0x8c    ; Conditional jump
00D4:    D8 C8                   fmul     st(0)
00D6:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D8:    D8 C8                   fmul     st(0)
00DA:    DE C1                   faddp    st(1)
00DC:    D9 FA                   fsqrt    ; sqrt(ST(0))
00DE:    DD 44 24 08             fld      qword ptr [esp + 8]
00E2:    D8 C8                   fmul     st(0)
00E4:    DD 04 14                fld      qword ptr [esp + edx]
00E7:    D8 C8                   fmul     st(0)
00E9:    DE C1                   faddp    st(1)
00EB:    D9 FA                   fsqrt    ; sqrt(ST(0))
00ED:    D9 E0                   fchs    ; -ST(0)
00EF:    DE C1                   faddp    st(1)
00F1:    F7 DA                   neg      edx
00F3:    D9 C0                   fld      st(0)
00F5:    DD 47 8C                fld      qword ptr [edi - 0x74]
00F8:    D9 EE                   fldz    ; Load 0.0
00FA:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00FC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00FE:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0100:    72 0A                   jb       0x10c    ; Conditional jump
0102:    D9 E8                   fld1    ; Load 1.0
0104:    DC C1                   fadd     st(1), st(0)
0106:    DE C1                   faddp    st(1)
0108:    D9 E1                   fabs    ; |ST(0)|
010A:    D9 FA                   fsqrt    ; sqrt(ST(0))
010C:    DC 4F 8C                fmul     qword ptr [edi - 0x74]
010F:    D9 E1                   fabs    ; |ST(0)|
0111:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0117:    DC 4F B8                fmul     qword ptr [edi - 0x48]
011A:    DC 64 14 10             fsub     qword ptr [esp + edx + 0x10]
011E:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0122:    75 03                   jne      0x127    ; Conditional jump
0124:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0127:    DD 5E E0                fstp     qword ptr [esi - 0x20]
012A:    83 C4 1C                add      esp, 0x1c
012D:    C3                      ret
012E:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0131:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0134:    83 C4 1C                add      esp, 0x1c
0137:    C3                      ret
