; Disassembly: Knots2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 216 bytes, decoded: 216 bytes (100.0%)
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
004B:    DD 44 24 08             fld      qword ptr [esp + 8]
004F:    DD 04 24                fld      qword ptr [esp]
0052:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0054:    DD 44 24 08             fld      qword ptr [esp + 8]
0058:    D8 C8                   fmul     st(0)
005A:    DD 04 24                fld      qword ptr [esp]
005D:    D8 C8                   fmul     st(0)
005F:    DE C1                   faddp    st(1)
0061:    D9 FA                   fsqrt    ; sqrt(ST(0))
0063:    DD 1C 24                fstp     qword ptr [esp]
0066:    DD 5C 24 08             fstp     qword ptr [esp + 8]
006A:    DD 47 10                fld      qword ptr [edi + 0x10]
006D:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0070:    8B 57 9C                mov      edx, dword ptr [edi - 0x64]
0073:    DD 44 24 08             fld      qword ptr [esp + 8]
0077:    DC 47 08                fadd     qword ptr [edi + 8]
007A:    DD 54 24 08             fst      qword ptr [esp + 8]
007E:    DA 77 B0                fidiv    dword ptr [edi - 0x50]
0081:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0085:    DA 4F AC                fimul    dword ptr [edi - 0x54]
0088:    D9 FF                   fcos    ; cos(ST(0))
008A:    DC 4F A0                fmul     qword ptr [edi - 0x60]
008D:    DC 04 24                fadd     qword ptr [esp]
0090:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0093:    D8 C8                   fmul     st(0)
0095:    DD 44 24 18             fld      qword ptr [esp + 0x18]
0099:    DA 4F A8                fimul    dword ptr [edi - 0x58]
009C:    D9 FE                   fsin    ; sin(ST(0))
009E:    DC 4F A0                fmul     qword ptr [edi - 0x60]
00A1:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
00A5:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00A8:    D8 C8                   fmul     st(0)
00AA:    DE C1                   faddp    st(1)
00AC:    D9 FA                   fsqrt    ; sqrt(ST(0))
00AE:    DC 56 E0                fcom     qword ptr [esi - 0x20]
00B1:    9B                      wait
00B2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B4:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00B6:    73 05                   jae      0xbd    ; Conditional jump
00B8:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00BB:    EB 02                   jmp      0xbf
00BD:    DD D8                   fstp     st(0)
00BF:    4A                      dec      edx
00C0:    7F B1                   jg       0x73    ; Conditional jump
00C2:    DD 46 E0                fld      qword ptr [esi - 0x20]
00C5:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00C8:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00CC:    75 03                   jne      0xd1    ; Conditional jump
00CE:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00D1:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00D4:    83 C4 32                add      esp, 0x32
00D7:    C3                      ret
