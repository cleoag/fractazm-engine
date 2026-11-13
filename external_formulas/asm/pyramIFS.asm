; Disassembly: pyramIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 275 bytes, decoded: 275 bytes (100.0%)
; DEoption: 20

0000:    83 C4 D8                add      esp, -0x28
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
004B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
004F:    DC 4F AC                fmul     qword ptr [edi - 0x54]
0052:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0054:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0058:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
005C:    DD 04 24                fld      qword ptr [esp]
005F:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0063:    DD 44 24 08             fld      qword ptr [esp + 8]
0067:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
006B:    D9 E0                   fchs    ; -ST(0)
006D:    DE C1                   faddp    st(1)
006F:    DD 04 24                fld      qword ptr [esp]
0072:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
0076:    DD 44 24 08             fld      qword ptr [esp + 8]
007A:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
007E:    DE C1                   faddp    st(1)
0080:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0084:    DD 1C 24                fstp     qword ptr [esp]
0087:    DD 04 24                fld      qword ptr [esp]
008A:    DC 44 24 08             fadd     qword ptr [esp + 8]
008E:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
0092:    DD 04 24                fld      qword ptr [esp]
0095:    DC 64 24 08             fsub     qword ptr [esp + 8]
0099:    DC 64 24 10             fsub     qword ptr [esp + 0x10]
009D:    D8 D1                   fcom     st(1)
009F:    9B                      wait
00A0:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A2:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A4:    73 04                   jae      0xaa    ; Conditional jump
00A6:    DD D8                   fstp     st(0)
00A8:    EB 02                   jmp      0xac
00AA:    DD D9                   fstp     st(1)
00AC:    DD 44 24 08             fld      qword ptr [esp + 8]
00B0:    DC 24 24                fsub     qword ptr [esp]
00B3:    DC 64 24 10             fsub     qword ptr [esp + 0x10]
00B7:    D8 D1                   fcom     st(1)
00B9:    9B                      wait
00BA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00BC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00BE:    73 04                   jae      0xc4    ; Conditional jump
00C0:    DD D8                   fstp     st(0)
00C2:    EB 02                   jmp      0xc6
00C4:    DD D9                   fstp     st(1)
00C6:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00CA:    DC 24 24                fsub     qword ptr [esp]
00CD:    DC 64 24 08             fsub     qword ptr [esp + 8]
00D1:    D8 D1                   fcom     st(1)
00D3:    9B                      wait
00D4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00D6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D8:    73 04                   jae      0xde    ; Conditional jump
00DA:    DD D8                   fstp     st(0)
00DC:    EB 02                   jmp      0xe0
00DE:    DD D9                   fstp     st(1)
00E0:    DD 54 24 18             fst      qword ptr [esp + 0x18]
00E4:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00E7:    DD 04 24                fld      qword ptr [esp]
00EA:    D8 C8                   fmul     st(0)
00EC:    DD 44 24 08             fld      qword ptr [esp + 8]
00F0:    D8 C8                   fmul     st(0)
00F2:    DE C1                   faddp    st(1)
00F4:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00F8:    D8 C8                   fmul     st(0)
00FA:    DE C1                   faddp    st(1)
00FC:    D9 FA                   fsqrt    ; sqrt(ST(0))
00FE:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0101:    DE C1                   faddp    st(1)
0103:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0107:    75 03                   jne      0x10c    ; Conditional jump
0109:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
010C:    DD 5E E0                fstp     qword ptr [esi - 0x20]
010F:    83 C4 28                add      esp, 0x28
0112:    C3                      ret
