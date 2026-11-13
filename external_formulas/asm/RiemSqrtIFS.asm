; Disassembly: RiemSqrtIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 222 bytes, decoded: 222 bytes (100.0%)
; DEoption: 20

0000:    83 C4 DA                add      esp, -0x26
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
005A:    D9 FA                   fsqrt    ; sqrt(ST(0))
005C:    DD 44 24 08             fld      qword ptr [esp + 8]
0060:    DD 04 24                fld      qword ptr [esp]
0063:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0065:    DA 4F B0                fimul    dword ptr [edi - 0x50]
0068:    DD 47 B8                fld      qword ptr [edi - 0x48]
006B:    DC 4F 08                fmul     qword ptr [edi + 8]
006E:    DE C1                   faddp    st(1)
0070:    DC 0F                   fmul     qword ptr [edi]
0072:    D9 C1                   fld      st(1)
0074:    DC 4F A0                fmul     qword ptr [edi - 0x60]
0077:    DE C1                   faddp    st(1)
0079:    D9 FF                   fcos    ; cos(ST(0))
007B:    D9 E1                   fabs    ; |ST(0)|
007D:    DE C9                   fmulp    st(1)
007F:    DC 4F A8                fmul     qword ptr [edi - 0x58]
0082:    D9 C0                   fld      st(0)
0084:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
0088:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
008A:    DC 64 24 10             fsub     qword ptr [esp + 0x10]
008E:    D8 D1                   fcom     st(1)
0090:    9B                      wait
0091:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0093:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0095:    73 04                   jae      0x9b    ; Conditional jump
0097:    DD D9                   fstp     st(1)
0099:    EB 02                   jmp      0x9d
009B:    DD D8                   fstp     st(0)
009D:    D9 E0                   fchs    ; -ST(0)
009F:    DD 04 24                fld      qword ptr [esp]
00A2:    D9 E1                   fabs    ; |ST(0)|
00A4:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00A7:    D8 D1                   fcom     st(1)
00A9:    9B                      wait
00AA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00AE:    73 04                   jae      0xb4    ; Conditional jump
00B0:    DD D8                   fstp     st(0)
00B2:    EB 02                   jmp      0xb6
00B4:    DD D9                   fstp     st(1)
00B6:    DD 44 24 08             fld      qword ptr [esp + 8]
00BA:    D9 E1                   fabs    ; |ST(0)|
00BC:    DC 67 F0                fsub     qword ptr [edi - 0x10]
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
00DA:    83 C4 26                add      esp, 0x26
00DD:    C3                      ret
