; Disassembly: roomclrIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 218 bytes, decoded: 218 bytes (100.0%)
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
004B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
004F:    D9 E4                   ftst
0051:    9B                      wait
0052:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0054:    9E                      sahf
0055:    73 05                   jae      0x5c    ; Conditional jump
0057:    DD 47 94                fld      qword ptr [edi - 0x6c]
005A:    EB 03                   jmp      0x5f
005C:    DD 47 8C                fld      qword ptr [edi - 0x74]
005F:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0065:    D9 E1                   fabs    ; |ST(0)|
0067:    DC 67 9C                fsub     qword ptr [edi - 0x64]
006A:    DD 44 24 08             fld      qword ptr [esp + 8]
006E:    D9 E1                   fabs    ; |ST(0)|
0070:    DC 67 A4                fsub     qword ptr [edi - 0x5c]
0073:    D8 D1                   fcom     st(1)
0075:    9B                      wait
0076:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0078:    9E                      sahf
0079:    72 0D                   jb       0x88    ; Conditional jump
007B:    DD D9                   fstp     st(1)
007D:    DD 47 84                fld      qword ptr [edi - 0x7c]
0080:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0086:    EB 02                   jmp      0x8a
0088:    DD D8                   fstp     st(0)
008A:    DD 04 24                fld      qword ptr [esp]
008D:    D9 E1                   fabs    ; |ST(0)|
008F:    DC 67 AC                fsub     qword ptr [edi - 0x54]
0092:    D8 D1                   fcom     st(1)
0094:    9B                      wait
0095:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0097:    9E                      sahf
0098:    72 0D                   jb       0xa7    ; Conditional jump
009A:    DD D9                   fstp     st(1)
009C:    DD 47 84                fld      qword ptr [edi - 0x7c]
009F:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00A5:    EB 02                   jmp      0xa9
00A7:    DD D8                   fstp     st(0)
00A9:    DD 04 24                fld      qword ptr [esp]
00AC:    D8 C8                   fmul     st(0)
00AE:    DD 44 24 08             fld      qword ptr [esp + 8]
00B2:    D8 C8                   fmul     st(0)
00B4:    DE C1                   faddp    st(1)
00B6:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00BA:    D8 C8                   fmul     st(0)
00BC:    DE C1                   faddp    st(1)
00BE:    D9 FA                   fsqrt    ; sqrt(ST(0))
00C0:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00C3:    DE C1                   faddp    st(1)
00C5:    D9 E1                   fabs    ; |ST(0)|
00C7:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00CA:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00CE:    75 03                   jne      0xd3    ; Conditional jump
00D0:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00D3:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00D6:    83 C4 1C                add      esp, 0x1c
00D9:    C3                      ret
