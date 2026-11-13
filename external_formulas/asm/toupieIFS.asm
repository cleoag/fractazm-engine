; Disassembly: toupieIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 274 bytes, decoded: 274 bytes (100.0%)
; DEoption: 20

0000:    83 C4 D4                add      esp, -0x2c
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
004B:    90                      nop
004C:    83 C7 D4                add      edi, -0x2c
004F:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0053:    DD 44 24 08             fld      qword ptr [esp + 8]
0057:    DD 04 24                fld      qword ptr [esp]
005A:    D9 C0                   fld      st(0)
005C:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
005F:    D9 C2                   fld      st(2)
0061:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0064:    DE C1                   faddp    st(1)
0066:    D9 C3                   fld      st(3)
0068:    D8 4F BC                fmul     dword ptr [edi - 0x44]
006B:    DE C1                   faddp    st(1)
006D:    D9 C1                   fld      st(1)
006F:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0072:    D9 C3                   fld      st(3)
0074:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0077:    DE C1                   faddp    st(1)
0079:    D9 C4                   fld      st(4)
007B:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
007E:    DE C1                   faddp    st(1)
0080:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0082:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0085:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0087:    D8 4F CC                fmul     dword ptr [edi - 0x34]
008A:    DE C3                   faddp    st(3)
008C:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008E:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0091:    DE C2                   faddp    st(2)
0093:    DD 1C 24                fstp     qword ptr [esp]
0096:    DD 5C 24 08             fstp     qword ptr [esp + 8]
009A:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
009E:    83 C7 2C                add      edi, 0x2c
00A1:    90                      nop
00A2:    DD 04 24                fld      qword ptr [esp]
00A5:    D8 C8                   fmul     st(0)
00A7:    DD 44 24 08             fld      qword ptr [esp + 8]
00AB:    D8 C8                   fmul     st(0)
00AD:    DE C1                   faddp    st(1)
00AF:    DD 14 24                fst      qword ptr [esp]
00B2:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B4:    DD 54 24 18             fst      qword ptr [esp + 0x18]
00B8:    D9 FA                   fsqrt    ; sqrt(ST(0))
00BA:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00BD:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00C1:    D9 E1                   fabs    ; |ST(0)|
00C3:    DC 6F F0                fsubr    qword ptr [edi - 0x10]
00C6:    DE E9                   fsubp    st(1)
00C8:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00CC:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00CF:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00D2:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00D5:    D8 D1                   fcom     st(1)
00D7:    9B                      wait
00D8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DA:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00DC:    73 04                   jae      0xe2    ; Conditional jump
00DE:    DD D8                   fstp     st(0)
00E0:    EB 02                   jmp      0xe4
00E2:    DD D9                   fstp     st(1)
00E4:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00E8:    D9 E1                   fabs    ; |ST(0)|
00EA:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00ED:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00F0:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00F3:    D8 D1                   fcom     st(1)
00F5:    9B                      wait
00F6:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F8:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00FA:    73 04                   jae      0x100    ; Conditional jump
00FC:    DD D8                   fstp     st(0)
00FE:    EB 02                   jmp      0x102
0100:    DD D9                   fstp     st(1)
0102:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0106:    75 03                   jne      0x10b    ; Conditional jump
0108:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
010B:    DD 5E E0                fstp     qword ptr [esi - 0x20]
010E:    83 C4 2C                add      esp, 0x2c
0111:    C3                      ret
