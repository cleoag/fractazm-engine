; Disassembly: hextgrid2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 288 bytes, decoded: 288 bytes (100.0%)
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
004B:    D9 EE                   fldz    ; Load 0.0
004D:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0051:    83 C7 D4                add      edi, -0x2c
0054:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0058:    DD 44 24 08             fld      qword ptr [esp + 8]
005C:    DD 04 24                fld      qword ptr [esp]
005F:    D9 C0                   fld      st(0)
0061:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0064:    D9 C2                   fld      st(2)
0066:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0069:    DE C1                   faddp    st(1)
006B:    D9 C3                   fld      st(3)
006D:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0070:    DE C1                   faddp    st(1)
0072:    D9 C1                   fld      st(1)
0074:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0077:    D9 C3                   fld      st(3)
0079:    D8 4F D8                fmul     dword ptr [edi - 0x28]
007C:    DE C1                   faddp    st(1)
007E:    D9 C4                   fld      st(4)
0080:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0083:    DE C1                   faddp    st(1)
0085:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0087:    D8 4F D0                fmul     dword ptr [edi - 0x30]
008A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008C:    D8 4F CC                fmul     dword ptr [edi - 0x34]
008F:    DE C3                   faddp    st(3)
0091:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0093:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0096:    DE C2                   faddp    st(2)
0098:    DD 1C 24                fstp     qword ptr [esp]
009B:    DD 5C 24 08             fstp     qword ptr [esp + 8]
009F:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A3:    83 C7 2C                add      edi, 0x2c
00A6:    DD 44 24 08             fld      qword ptr [esp + 8]
00AA:    DC 4F 08                fmul     qword ptr [edi + 8]
00AD:    D9 FC                   frndint    ; round(ST(0))
00AF:    DC 0F                   fmul     qword ptr [edi]
00B1:    DC 44 24 08             fadd     qword ptr [esp + 8]
00B5:    D9 E1                   fabs    ; |ST(0)|
00B7:    DD 54 24 18             fst      qword ptr [esp + 0x18]
00BB:    DC 4F 08                fmul     qword ptr [edi + 8]
00BE:    DD 04 24                fld      qword ptr [esp]
00C1:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00C4:    D9 FC                   frndint    ; round(ST(0))
00C6:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00C9:    DD 04 24                fld      qword ptr [esp]
00CC:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00CF:    DE C1                   faddp    st(1)
00D1:    D9 E1                   fabs    ; |ST(0)|
00D3:    DE C1                   faddp    st(1)
00D5:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00D9:    D8 D1                   fcom     st(1)
00DB:    9B                      wait
00DC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DE:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00E0:    73 04                   jae      0xe6    ; Conditional jump
00E2:    DD D8                   fstp     st(0)
00E4:    EB 02                   jmp      0xe8
00E6:    DD D9                   fstp     st(1)
00E8:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00EB:    90                      nop
00EC:    90                      nop
00ED:    90                      nop
00EE:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00F2:    D8 D1                   fcom     st(1)
00F4:    9B                      wait
00F5:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F7:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00F9:    76 04                   jbe      0xff    ; Conditional jump
00FB:    DD D8                   fstp     st(0)
00FD:    EB 02                   jmp      0x101
00FF:    DD D9                   fstp     st(1)
0101:    D8 C8                   fmul     st(0)
0103:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0107:    D8 C8                   fmul     st(0)
0109:    DE C1                   faddp    st(1)
010B:    D9 FA                   fsqrt    ; sqrt(ST(0))
010D:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0110:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0114:    75 03                   jne      0x119    ; Conditional jump
0116:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0119:    DD 5E E0                fstp     qword ptr [esi - 0x20]
011C:    83 C4 32                add      esp, 0x32
011F:    C3                      ret
