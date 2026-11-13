; Disassembly: boardIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 480 bytes, decoded: 480 bytes (100.0%)
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
00A2:    83 7F 8C 01             cmp      dword ptr [edi - 0x74], 1
00A6:    75 21                   jne      0xc9    ; Conditional jump
00A8:    DD 04 24                fld      qword ptr [esp]
00AB:    D8 C8                   fmul     st(0)
00AD:    DD 44 24 08             fld      qword ptr [esp + 8]
00B1:    D8 C8                   fmul     st(0)
00B3:    DE C1                   faddp    st(1)
00B5:    D9 E8                   fld1    ; Load 1.0
00B7:    DE F1                   fdivrp   st(1)
00B9:    D9 C0                   fld      st(0)
00BB:    DC 0C 24                fmul     qword ptr [esp]
00BE:    DD 1C 24                fstp     qword ptr [esp]
00C1:    DC 4C 24 08             fmul     qword ptr [esp + 8]
00C5:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00C9:    90                      nop
00CA:    83 7F 8C 02             cmp      dword ptr [edi - 0x74], 2
00CE:    75 3C                   jne      0x10c    ; Conditional jump
00D0:    DD 04 24                fld      qword ptr [esp]
00D3:    D8 C8                   fmul     st(0)
00D5:    DD 44 24 08             fld      qword ptr [esp + 8]
00D9:    D9 E8                   fld1    ; Load 1.0
00DB:    DE C1                   faddp    st(1)
00DD:    D8 C8                   fmul     st(0)
00DF:    DE C1                   faddp    st(1)
00E1:    D9 E8                   fld1    ; Load 1.0
00E3:    DE F1                   fdivrp   st(1)
00E5:    DD 04 24                fld      qword ptr [esp]
00E8:    D8 C8                   fmul     st(0)
00EA:    DD 44 24 08             fld      qword ptr [esp + 8]
00EE:    D8 C8                   fmul     st(0)
00F0:    DE C1                   faddp    st(1)
00F2:    D9 E8                   fld1    ; Load 1.0
00F4:    DE E9                   fsubp    st(1)
00F6:    D8 C9                   fmul     st(1)
00F8:    D9 D0                   fnop
00FA:    DD 04 24                fld      qword ptr [esp]
00FD:    D8 C0                   fadd     st(0)
00FF:    D9 E0                   fchs    ; -ST(0)
0101:    D8 CA                   fmul     st(2)
0103:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0107:    DD 1C 24                fstp     qword ptr [esp]
010A:    DD D8                   fstp     st(0)
010C:    90                      nop
010D:    83 7F 8C 03             cmp      dword ptr [edi - 0x74], 3
0111:    75 27                   jne      0x13a    ; Conditional jump
0113:    DD 04 24                fld      qword ptr [esp]
0116:    D8 C8                   fmul     st(0)
0118:    DD 44 24 08             fld      qword ptr [esp + 8]
011C:    D8 C8                   fmul     st(0)
011E:    DE C1                   faddp    st(1)
0120:    D9 E8                   fld1    ; Load 1.0
0122:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0124:    D9 F1                   fyl2x
0126:    D9 E8                   fld1    ; Load 1.0
0128:    DE F1                   fdivrp   st(1)
012A:    D9 C0                   fld      st(0)
012C:    DC 0C 24                fmul     qword ptr [esp]
012F:    DD 1C 24                fstp     qword ptr [esp]
0132:    DC 4C 24 08             fmul     qword ptr [esp + 8]
0136:    DD 5C 24 08             fstp     qword ptr [esp + 8]
013A:    90                      nop
013B:    83 7F 8C 04             cmp      dword ptr [edi - 0x74], 4
013F:    75 21                   jne      0x162    ; Conditional jump
0141:    DD 04 24                fld      qword ptr [esp]
0144:    D8 C8                   fmul     st(0)
0146:    DD 44 24 08             fld      qword ptr [esp + 8]
014A:    D8 C8                   fmul     st(0)
014C:    DE C1                   faddp    st(1)
014E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0150:    90                      nop
0151:    90                      nop
0152:    DD 44 24 08             fld      qword ptr [esp + 8]
0156:    DD 04 24                fld      qword ptr [esp]
0159:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
015B:    DD 5C 24 08             fstp     qword ptr [esp + 8]
015F:    DD 1C 24                fstp     qword ptr [esp]
0162:    DD 04 24                fld      qword ptr [esp]
0165:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0168:    D9 FE                   fsin    ; sin(ST(0))
016A:    DD 44 24 08             fld      qword ptr [esp + 8]
016E:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0171:    D9 FE                   fsin    ; sin(ST(0))
0173:    DE C9                   fmulp    st(1)
0175:    90                      nop
0176:    D9 E4                   ftst
0178:    9B                      wait
0179:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
017B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
017D:    73 05                   jae      0x184    ; Conditional jump
017F:    DD 47 84                fld      qword ptr [edi - 0x7c]
0182:    EB 06                   jmp      0x18a
0184:    DD 87 7C FF FF FF       fld      qword ptr [edi - 0x84]
018A:    DB 46 E8                fild     dword ptr [esi - 0x18]
018D:    DC 8F 74 FF FF FF       fmul     qword ptr [edi - 0x8c]
0193:    DE C1                   faddp    st(1)
0195:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
019B:    DC 0F                   fmul     qword ptr [edi]
019D:    DC 77 B8                fdiv     qword ptr [edi - 0x48]
01A0:    90                      nop
01A1:    DD 44 24 10             fld      qword ptr [esp + 0x10]
01A5:    D9 E0                   fchs    ; -ST(0)
01A7:    DD 54 24 10             fst      qword ptr [esp + 0x10]
01AB:    DC 47 F0                fadd     qword ptr [edi - 0x10]
01AE:    D8 D1                   fcom     st(1)
01B0:    9B                      wait
01B1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01B3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01B5:    73 04                   jae      0x1bb    ; Conditional jump
01B7:    DD D9                   fstp     st(1)
01B9:    EB 02                   jmp      0x1bd
01BB:    DD D8                   fstp     st(0)
01BD:    DD 44 24 10             fld      qword ptr [esp + 0x10]
01C1:    D8 D1                   fcom     st(1)
01C3:    9B                      wait
01C4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01C6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01C8:    73 04                   jae      0x1ce    ; Conditional jump
01CA:    DD D8                   fstp     st(0)
01CC:    EB 02                   jmp      0x1d0
01CE:    DD D9                   fstp     st(1)
01D0:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
01D4:    75 03                   jne      0x1d9    ; Conditional jump
01D6:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
01D9:    DD 5E E0                fstp     qword ptr [esi - 0x20]
01DC:    83 C4 2C                add      esp, 0x2c
01DF:    C3                      ret
