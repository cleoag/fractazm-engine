; Disassembly: TorusIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 217 bytes, decoded: 217 bytes (100.0%)
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
004B:    DD 04 24                fld      qword ptr [esp]
004E:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
0051:    DD 1C 24                fstp     qword ptr [esp]
0054:    83 7F B0 00             cmp      dword ptr [edi - 0x50], 0
0058:    75 11                   jne      0x6b    ; Conditional jump
005A:    DD 04 24                fld      qword ptr [esp]
005D:    D8 C8                   fmul     st(0)
005F:    DD 44 24 08             fld      qword ptr [esp + 8]
0063:    D8 C8                   fmul     st(0)
0065:    DE C1                   faddp    st(1)
0067:    D9 FA                   fsqrt    ; sqrt(ST(0))
0069:    EB 1A                   jmp      0x85
006B:    DD 04 24                fld      qword ptr [esp]
006E:    D9 E1                   fabs    ; |ST(0)|
0070:    DD 44 24 08             fld      qword ptr [esp + 8]
0074:    D9 E1                   fabs    ; |ST(0)|
0076:    D8 D1                   fcom     st(1)
0078:    9B                      wait
0079:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
007B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
007D:    73 04                   jae      0x83    ; Conditional jump
007F:    DD D8                   fstp     st(0)
0081:    EB 02                   jmp      0x85
0083:    DD D9                   fstp     st(1)
0085:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0088:    83 7F AC 00             cmp      dword ptr [edi - 0x54], 0
008C:    75 0E                   jne      0x9c    ; Conditional jump
008E:    D8 C8                   fmul     st(0)
0090:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0094:    D8 C8                   fmul     st(0)
0096:    DE C1                   faddp    st(1)
0098:    D9 FA                   fsqrt    ; sqrt(ST(0))
009A:    EB 17                   jmp      0xb3
009C:    D9 E1                   fabs    ; |ST(0)|
009E:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00A2:    D9 E1                   fabs    ; |ST(0)|
00A4:    D8 D1                   fcom     st(1)
00A6:    9B                      wait
00A7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A9:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00AB:    73 04                   jae      0xb1    ; Conditional jump
00AD:    DD D8                   fstp     st(0)
00AF:    EB 02                   jmp      0xb3
00B1:    DD D9                   fstp     st(1)
00B3:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00B6:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00BA:    75 03                   jne      0xbf    ; Conditional jump
00BC:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00BF:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00C2:    DD 47 94                fld      qword ptr [edi - 0x6c]
00C5:    DB 46 E8                fild     dword ptr [esi - 0x18]
00C8:    DC 4F 9C                fmul     qword ptr [edi - 0x64]
00CB:    DE C1                   faddp    st(1)
00CD:    D9 E1                   fabs    ; |ST(0)|
00CF:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00D5:    83 C4 1C                add      esp, 0x1c
00D8:    C3                      ret
