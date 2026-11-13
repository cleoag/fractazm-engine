; Disassembly: KleinBagelIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 250 bytes, decoded: 250 bytes (100.0%)
; DEoption: 20

0000:    83 C4 CE                add      esp, -0x32
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
0054:    DA 4F AC                fimul    dword ptr [edi - 0x54]
0057:    DC 0F                   fmul     qword ptr [edi]
0059:    D9 FB                   fsincos    ; ST(0)=cos, push sin
005B:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
005F:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0063:    DD 04 24                fld      qword ptr [esp]
0066:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
0069:    DD 1C 24                fstp     qword ptr [esp]
006C:    83 7F B0 00             cmp      dword ptr [edi - 0x50], 0
0070:    75 11                   jne      0x83    ; Conditional jump
0072:    DD 04 24                fld      qword ptr [esp]
0075:    D8 C8                   fmul     st(0)
0077:    DD 44 24 08             fld      qword ptr [esp + 8]
007B:    D8 C8                   fmul     st(0)
007D:    DE C1                   faddp    st(1)
007F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0081:    EB 1A                   jmp      0x9d
0083:    DD 04 24                fld      qword ptr [esp]
0086:    D9 E1                   fabs    ; |ST(0)|
0088:    DD 44 24 08             fld      qword ptr [esp + 8]
008C:    D9 E1                   fabs    ; |ST(0)|
008E:    D8 D1                   fcom     st(1)
0090:    9B                      wait
0091:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0093:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0095:    73 04                   jae      0x9b    ; Conditional jump
0097:    DD D8                   fstp     st(0)
0099:    EB 02                   jmp      0x9d
009B:    DD D9                   fstp     st(1)
009D:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00A0:    DD 1C 24                fstp     qword ptr [esp]
00A3:    DD 04 24                fld      qword ptr [esp]
00A6:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00AA:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00AE:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00B2:    D9 E0                   fchs    ; -ST(0)
00B4:    DE C1                   faddp    st(1)
00B6:    DD 04 24                fld      qword ptr [esp]
00B9:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00BD:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00C1:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00C5:    DE C1                   faddp    st(1)
00C7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00C9:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00CD:    D8 C8                   fmul     st(0)
00CF:    D9 C0                   fld      st(0)
00D1:    D8 C8                   fmul     st(0)
00D3:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D5:    D9 E0                   fchs    ; -ST(0)
00D7:    DE C1                   faddp    st(1)
00D9:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00DD:    D8 C8                   fmul     st(0)
00DF:    DE C1                   faddp    st(1)
00E1:    D9 E1                   fabs    ; |ST(0)|
00E3:    D9 FA                   fsqrt    ; sqrt(ST(0))
00E5:    DC 0F                   fmul     qword ptr [edi]
00E7:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00EA:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00EE:    75 03                   jne      0xf3    ; Conditional jump
00F0:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00F3:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00F6:    83 C4 32                add      esp, 0x32
00F9:    C3                      ret
