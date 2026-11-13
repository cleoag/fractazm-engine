; Disassembly: loxodromeIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 315 bytes, decoded: 315 bytes (100.0%)
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
004B:    BA 00 00 00 00          mov      edx, 0
0050:    89 54 24 1C             mov      dword ptr [esp + 0x1c], edx
0054:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0058:    D9 E1                   fabs    ; |ST(0)|
005A:    DD 47 20                fld      qword ptr [edi + 0x20]
005D:    D8 D9                   fcomp    st(1)
005F:    9B                      wait
0060:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0062:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0064:    73 07                   jae      0x6d    ; Conditional jump
0066:    DD D8                   fstp     st(0)
0068:    DD 47 18                fld      qword ptr [edi + 0x18]
006B:    EB 73                   jmp      0xe0
006D:    D9 E0                   fchs    ; -ST(0)
006F:    D9 E8                   fld1    ; Load 1.0
0071:    DE C1                   faddp    st(1)
0073:    DD 47 B8                fld      qword ptr [edi - 0x48]
0076:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0078:    D9 F1                   fyl2x
007A:    8B 54 24 14             mov      edx, dword ptr [esp + 0x14]
007E:    81 E2 00 00 00 80       and      edx, 0x80000000
0084:    74 02                   je       0x88    ; Conditional jump
0086:    D9 E0                   fchs    ; -ST(0)
0088:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
008B:    D9 FB                   fsincos    ; ST(0)=cos, push sin
008D:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0091:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0095:    DD 04 24                fld      qword ptr [esp]
0098:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
009C:    DD 44 24 08             fld      qword ptr [esp + 8]
00A0:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00A4:    D9 E0                   fchs    ; -ST(0)
00A6:    DE C1                   faddp    st(1)
00A8:    DD 04 24                fld      qword ptr [esp]
00AB:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00AF:    DD 44 24 08             fld      qword ptr [esp + 8]
00B3:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00B7:    DE C1                   faddp    st(1)
00B9:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00BB:    DC 0F                   fmul     qword ptr [edi]
00BD:    DC 77 AC                fdiv     qword ptr [edi - 0x54]
00C0:    D9 C0                   fld      st(0)
00C2:    DB 5C 24 1C             fistp    dword ptr [esp + 0x1c]
00C6:    DB 44 24 1C             fild     dword ptr [esp + 0x1c]
00CA:    DE E9                   fsubp    st(1)
00CC:    DC 4F AC                fmul     qword ptr [edi - 0x54]
00CF:    D9 E1                   fabs    ; |ST(0)|
00D1:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
00D4:    D9 E8                   fld1    ; Load 1.0
00D6:    DE E9                   fsubp    st(1)
00D8:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00DB:    DC 4F 08                fmul     qword ptr [edi + 8]
00DE:    D9 E0                   fchs    ; -ST(0)
00E0:    DD 04 24                fld      qword ptr [esp]
00E3:    D8 C8                   fmul     st(0)
00E5:    DD 44 24 08             fld      qword ptr [esp + 8]
00E9:    D8 C8                   fmul     st(0)
00EB:    DE C1                   faddp    st(1)
00ED:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00F1:    D8 C8                   fmul     st(0)
00F3:    DE C1                   faddp    st(1)
00F5:    D9 FA                   fsqrt    ; sqrt(ST(0))
00F7:    DC 67 28                fsub     qword ptr [edi + 0x28]
00FA:    DC 47 9C                fadd     qword ptr [edi - 0x64]
00FD:    D9 E1                   fabs    ; |ST(0)|
00FF:    DC 67 9C                fsub     qword ptr [edi - 0x64]
0102:    D8 D1                   fcom     st(1)
0104:    9B                      wait
0105:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0107:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0109:    73 04                   jae      0x10f    ; Conditional jump
010B:    DD D8                   fstp     st(0)
010D:    EB 02                   jmp      0x111
010F:    DD D9                   fstp     st(1)
0111:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0115:    75 03                   jne      0x11a    ; Conditional jump
0117:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
011A:    DD 5E E0                fstp     qword ptr [esi - 0x20]
011D:    DB 46 E8                fild     dword ptr [esi - 0x18]
0120:    DC 4F 94                fmul     qword ptr [edi - 0x6c]
0123:    DC 47 8C                fadd     qword ptr [edi - 0x74]
0126:    DB 44 24 1C             fild     dword ptr [esp + 0x1c]
012A:    DC 4F 84                fmul     qword ptr [edi - 0x7c]
012D:    DE C1                   faddp    st(1)
012F:    D9 E1                   fabs    ; |ST(0)|
0131:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0137:    83 C4 28                add      esp, 0x28
013A:    C3                      ret
