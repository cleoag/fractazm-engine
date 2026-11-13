; Disassembly: TorusEllipIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 197 bytes, decoded: 197 bytes (100.0%)
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
004F:    75 11                   jne      0x62    ; Conditional jump
0051:    DD 04 24                fld      qword ptr [esp]
0054:    D8 C8                   fmul     st(0)
0056:    DD 44 24 08             fld      qword ptr [esp + 8]
005A:    D8 C8                   fmul     st(0)
005C:    DE C1                   faddp    st(1)
005E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0060:    EB 1A                   jmp      0x7c
0062:    DD 04 24                fld      qword ptr [esp]
0065:    D9 E1                   fabs    ; |ST(0)|
0067:    DD 44 24 08             fld      qword ptr [esp + 8]
006B:    D9 E1                   fabs    ; |ST(0)|
006D:    D8 D1                   fcom     st(1)
006F:    9B                      wait
0070:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0072:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0074:    73 04                   jae      0x7a    ; Conditional jump
0076:    DD D8                   fstp     st(0)
0078:    EB 02                   jmp      0x7c
007A:    DD D9                   fstp     st(1)
007C:    DC 67 F0                fsub     qword ptr [edi - 0x10]
007F:    83 7F AC 00             cmp      dword ptr [edi - 0x54], 0
0083:    75 0E                   jne      0x93    ; Conditional jump
0085:    D8 C8                   fmul     st(0)
0087:    DD 44 24 10             fld      qword ptr [esp + 0x10]
008B:    D8 C8                   fmul     st(0)
008D:    DE C1                   faddp    st(1)
008F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0091:    EB 17                   jmp      0xaa
0093:    D9 E1                   fabs    ; |ST(0)|
0095:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0099:    D9 E1                   fabs    ; |ST(0)|
009B:    D8 D1                   fcom     st(1)
009D:    9B                      wait
009E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A0:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A2:    73 04                   jae      0xa8    ; Conditional jump
00A4:    DD D8                   fstp     st(0)
00A6:    EB 02                   jmp      0xaa
00A8:    DD D9                   fstp     st(1)
00AA:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00AD:    DD 04 24                fld      qword ptr [esp]
00B0:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
00B3:    DE C1                   faddp    st(1)
00B5:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00B9:    75 03                   jne      0xbe    ; Conditional jump
00BB:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00BE:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00C1:    83 C4 1C                add      esp, 0x1c
00C4:    C3                      ret
