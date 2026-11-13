; Disassembly: gnarlyIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 367 bytes, decoded: 367 bytes (100.0%)
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
0080:    E9 DA 00 00 00          jmp      0x15f
0085:    DD 44 24 08             fld      qword ptr [esp + 8]
0089:    DD 04 14                fld      qword ptr [esp + edx]
008C:    D9 C0                   fld      st(0)
008E:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0091:    D9 FE                   fsin    ; sin(ST(0))
0093:    D8 C1                   fadd     st(1)
0095:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
0098:    D9 FE                   fsin    ; sin(ST(0))
009A:    D8 C1                   fadd     st(1)
009C:    D9 FE                   fsin    ; sin(ST(0))
009E:    DC 4F 9C                fmul     qword ptr [edi - 0x64]
00A1:    D8 C2                   fadd     st(2)
00A3:    D9 C2                   fld      st(2)
00A5:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00A8:    D9 FE                   fsin    ; sin(ST(0))
00AA:    D8 C3                   fadd     st(3)
00AC:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
00AF:    D9 FE                   fsin    ; sin(ST(0))
00B1:    D8 C3                   fadd     st(3)
00B3:    D9 FE                   fsin    ; sin(ST(0))
00B5:    DC 4F 94                fmul     qword ptr [edi - 0x6c]
00B8:    D8 C2                   fadd     st(2)
00BA:    DD DB                   fstp     st(3)
00BC:    DD D9                   fstp     st(1)
00BE:    FF 4C 24 18             dec      dword ptr [esp + 0x18]
00C2:    7F C8                   jg       0x8c    ; Conditional jump
00C4:    D8 C8                   fmul     st(0)
00C6:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00C8:    D8 C8                   fmul     st(0)
00CA:    DE C1                   faddp    st(1)
00CC:    D9 FA                   fsqrt    ; sqrt(ST(0))
00CE:    DD 44 24 08             fld      qword ptr [esp + 8]
00D2:    D8 C8                   fmul     st(0)
00D4:    DD 04 14                fld      qword ptr [esp + edx]
00D7:    D8 C8                   fmul     st(0)
00D9:    DE C1                   faddp    st(1)
00DB:    D9 FA                   fsqrt    ; sqrt(ST(0))
00DD:    D9 E0                   fchs    ; -ST(0)
00DF:    DE C1                   faddp    st(1)
00E1:    F7 DA                   neg      edx
00E3:    D9 C0                   fld      st(0)
00E5:    DD 47 8C                fld      qword ptr [edi - 0x74]
00E8:    D9 EE                   fldz    ; Load 0.0
00EA:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00EC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00EE:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00F0:    72 0A                   jb       0xfc    ; Conditional jump
00F2:    D9 E8                   fld1    ; Load 1.0
00F4:    DC C1                   fadd     st(1), st(0)
00F6:    DE C1                   faddp    st(1)
00F8:    D9 E1                   fabs    ; |ST(0)|
00FA:    D9 FA                   fsqrt    ; sqrt(ST(0))
00FC:    DC 4F 8C                fmul     qword ptr [edi - 0x74]
00FF:    D9 E1                   fabs    ; |ST(0)|
0101:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0107:    DC 4F B8                fmul     qword ptr [edi - 0x48]
010A:    DC 64 14 10             fsub     qword ptr [esp + edx + 0x10]
010E:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0112:    75 03                   jne      0x117    ; Conditional jump
0114:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0117:    DD 5E E0                fstp     qword ptr [esi - 0x20]
011A:    83 C4 1C                add      esp, 0x1c
011D:    C3                      ret
011E:    47                      inc      edi
011F:    8C D9                   mov      ecx, ds
0121:    EE                      out      dx, al
0122:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0124:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0126:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0128:    72 0A                   jb       0x134    ; Conditional jump
012A:    D9 E8                   fld1    ; Load 1.0
012C:    DC C1                   fadd     st(1), st(0)
012E:    DE C1                   faddp    st(1)
0130:    D9 E1                   fabs    ; |ST(0)|
0132:    D9 FA                   fsqrt    ; sqrt(ST(0))
0134:    DC 4F 8C                fmul     qword ptr [edi - 0x74]
0137:    D9 E1                   fabs    ; |ST(0)|
0139:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
013F:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0142:    DC 64 14 10             fsub     qword ptr [esp + edx + 0x10]
0146:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
014A:    75 03                   jne      0x14f    ; Conditional jump
014C:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
014F:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0152:    83 C4 1C                add      esp, 0x1c
0155:    C3                      ret
0156:    F7 DA                   neg      edx
0158:    DC 4F B8                fmul     qword ptr [edi - 0x48]
015B:    DC 64 14 10             fsub     qword ptr [esp + edx + 0x10]
015F:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0163:    75 03                   jne      0x168    ; Conditional jump
0165:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0168:    DD 5E E0                fstp     qword ptr [esi - 0x20]
016B:    83 C4 1C                add      esp, 0x1c
016E:    C3                      ret
