; Disassembly: greekIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 282 bytes, decoded: 282 bytes (100.0%)
; DEoption: 20

0000:    83 C4 B0                add      esp, -0x50
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
004B:    83 C7 D4                add      edi, -0x2c
004E:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0052:    DD 44 24 08             fld      qword ptr [esp + 8]
0056:    DD 04 24                fld      qword ptr [esp]
0059:    D9 C0                   fld      st(0)
005B:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
005E:    D9 C2                   fld      st(2)
0060:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0063:    DE C1                   faddp    st(1)
0065:    D9 C3                   fld      st(3)
0067:    D8 4F BC                fmul     dword ptr [edi - 0x44]
006A:    DE C1                   faddp    st(1)
006C:    D9 C1                   fld      st(1)
006E:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0071:    D9 C3                   fld      st(3)
0073:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0076:    DE C1                   faddp    st(1)
0078:    D9 C4                   fld      st(4)
007A:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
007D:    DE C1                   faddp    st(1)
007F:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0081:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0084:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0086:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0089:    DE C3                   faddp    st(3)
008B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008D:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0090:    DE C2                   faddp    st(2)
0092:    DD 1C 24                fstp     qword ptr [esp]
0095:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0099:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
009D:    83 C7 2C                add      edi, 0x2c
00A0:    8A 54 24 07             mov      dl, byte ptr [esp + 7]
00A4:    DD 04 24                fld      qword ptr [esp]
00A7:    90                      nop
00A8:    D9 E1                   fabs    ; |ST(0)|
00AA:    DD 44 24 08             fld      qword ptr [esp + 8]
00AE:    83 7F B8 00             cmp      dword ptr [edi - 0x48], 0
00B2:    75 03                   jne      0xb7    ; Conditional jump
00B4:    80 F2 80                xor      dl, 0x80
00B7:    80 E2 80                and      dl, 0x80
00BA:    75 04                   jne      0xc0    ; Conditional jump
00BC:    DC 07                   fadd     qword ptr [edi]
00BE:    EB 02                   jmp      0xc2
00C0:    DC 27                   fsub     qword ptr [edi]
00C2:    D9 E1                   fabs    ; |ST(0)|
00C4:    D8 D1                   fcom     st(1)
00C6:    9B                      wait
00C7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C9:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00CB:    72 04                   jb       0xd1    ; Conditional jump
00CD:    DD D9                   fstp     st(1)
00CF:    EB 02                   jmp      0xd3
00D1:    DD D8                   fstp     st(0)
00D3:    DC 27                   fsub     qword ptr [edi]
00D5:    D9 C0                   fld      st(0)
00D7:    D9 FC                   frndint    ; round(ST(0))
00D9:    DE E9                   fsubp    st(1)
00DB:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
00DF:    74 1A                   je       0xfb    ; Conditional jump
00E1:    D9 E1                   fabs    ; |ST(0)|
00E3:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00E7:    D9 E1                   fabs    ; |ST(0)|
00E9:    D8 D1                   fcom     st(1)
00EB:    9B                      wait
00EC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00EE:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00F0:    72 04                   jb       0xf6    ; Conditional jump
00F2:    DD D9                   fstp     st(1)
00F4:    EB 02                   jmp      0xf8
00F6:    DD D8                   fstp     st(0)
00F8:    EB 0D                   jmp      0x107
00FA:    90                      nop
00FB:    D8 C8                   fmul     st(0)
00FD:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0101:    D8 C8                   fmul     st(0)
0103:    DE C1                   faddp    st(1)
0105:    D9 FA                   fsqrt    ; sqrt(ST(0))
0107:    DC 67 F0                fsub     qword ptr [edi - 0x10]
010A:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
010E:    75 03                   jne      0x113    ; Conditional jump
0110:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0113:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0116:    83 C4 50                add      esp, 0x50
0119:    C3                      ret
