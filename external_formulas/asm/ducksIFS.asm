; Disassembly: ducksIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 315 bytes, decoded: 315 bytes (100.0%)
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
0067:    DC 47 10                fadd     qword ptr [edi + 0x10]
006A:    DC 4F 08                fmul     qword ptr [edi + 8]
006D:    90                      nop
006E:    DC 86 78 FF FF FF       fadd     qword ptr [esi - 0x88]
0074:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0076:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0078:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
007A:    73 0B                   jae      0x87    ; Conditional jump
007C:    DD 44 14 10             fld      qword ptr [esp + edx + 0x10]
0080:    D9 E0                   fchs    ; -ST(0)
0082:    E9 A4 00 00 00          jmp      0x12b
0087:    D9 EE                   fldz    ; Load 0.0
0089:    DD 04 14                fld      qword ptr [esp + edx]
008C:    DD 44 24 08             fld      qword ptr [esp + 8]
0090:    D9 C0                   fld      st(0)
0092:    D8 C8                   fmul     st(0)
0094:    D9 C2                   fld      st(2)
0096:    D8 C8                   fmul     st(0)
0098:    DE C1                   faddp    st(1)
009A:    D9 ED                   fldln2    ; Load ln(2)
009C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
009E:    D9 F1                   fyl2x
00A0:    DC 0F                   fmul     qword ptr [edi]
00A2:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
00A5:    D9 C1                   fld      st(1)
00A7:    D9 E1                   fabs    ; |ST(0)|
00A9:    D9 C3                   fld      st(3)
00AB:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00AD:    DC 47 9C                fadd     qword ptr [edi - 0x64]
00B0:    DD DA                   fstp     st(2)
00B2:    DD DA                   fstp     st(2)
00B4:    D9 C1                   fld      st(1)
00B6:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
00B9:    D9 C1                   fld      st(1)
00BB:    D8 4F 98                fmul     dword ptr [edi - 0x68]
00BE:    D9 E0                   fchs    ; -ST(0)
00C0:    DE C1                   faddp    st(1)
00C2:    D9 C2                   fld      st(2)
00C4:    D8 4F 98                fmul     dword ptr [edi - 0x68]
00C7:    D9 C2                   fld      st(2)
00C9:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
00CC:    DE C1                   faddp    st(1)
00CE:    DD DA                   fstp     st(2)
00D0:    DD DA                   fstp     st(2)
00D2:    D9 C0                   fld      st(0)
00D4:    D8 C8                   fmul     st(0)
00D6:    D9 C2                   fld      st(2)
00D8:    D8 C8                   fmul     st(0)
00DA:    DE C1                   faddp    st(1)
00DC:    D9 FA                   fsqrt    ; sqrt(ST(0))
00DE:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E0:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00E3:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E5:    DC C3                   fadd     st(3), st(0)
00E7:    DD D8                   fstp     st(0)
00E9:    FF 4C 24 18             dec      dword ptr [esp + 0x18]
00ED:    7F A1                   jg       0x90    ; Conditional jump
00EF:    DD D8                   fstp     st(0)
00F1:    DD D8                   fstp     st(0)
00F3:    D9 E8                   fld1    ; Load 1.0
00F5:    DE C1                   faddp    st(1)
00F7:    D9 ED                   fldln2    ; Load ln(2)
00F9:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00FB:    D9 F1                   fyl2x
00FD:    D9 C0                   fld      st(0)
00FF:    DD 47 8C                fld      qword ptr [edi - 0x74]
0102:    D9 EE                   fldz    ; Load 0.0
0104:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0106:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0108:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
010A:    72 08                   jb       0x114    ; Conditional jump
010C:    D8 C8                   fmul     st(0)
010E:    90                      nop
010F:    90                      nop
0110:    90                      nop
0111:    90                      nop
0112:    90                      nop
0113:    90                      nop
0114:    DC 4F 8C                fmul     qword ptr [edi - 0x74]
0117:    D9 E1                   fabs    ; |ST(0)|
0119:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
011F:    DA 77 B0                fidiv    dword ptr [edi - 0x50]
0122:    F7 DA                   neg      edx
0124:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0127:    DC 64 14 10             fsub     qword ptr [esp + edx + 0x10]
012B:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
012F:    75 03                   jne      0x134    ; Conditional jump
0131:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0134:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0137:    83 C4 1C                add      esp, 0x1c
013A:    C3                      ret
