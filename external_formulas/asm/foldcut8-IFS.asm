; Disassembly: foldcut8-IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 249 bytes, decoded: 249 bytes (100.0%)
; DEoption: 20

0000:    83 C4 E4                add      esp, -0x1c
0003:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0008:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000D:    66 0F 59 47 E0          mulpd    xmm0, xmmword ptr [edi - 0x20]
0012:    F2 0F 59 4F E0          mulsd    xmm1, qword ptr [edi - 0x20]
0017:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
001C:    F2 0F 58 4F D0          addsd    xmm1, qword ptr [edi - 0x30]
0021:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
0025:    74 19                   je       0x40    ; Conditional jump
0027:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002C:    F2 0F 59 6F E0          mulsd    xmm5, qword ptr [edi - 0x20]
0031:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0036:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
003B:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0040:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0045:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
004B:    DD 04 24                fld      qword ptr [esp]
004E:    D9 E1                   fabs    ; |ST(0)|
0050:    DD 44 24 08             fld      qword ptr [esp + 8]
0054:    D9 E1                   fabs    ; |ST(0)|
0056:    DD 44 24 10             fld      qword ptr [esp + 0x10]
005A:    D9 E1                   fabs    ; |ST(0)|
005C:    D8 D1                   fcom     st(1)
005E:    9B                      wait
005F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0061:    80 E4 41                and      ah, 0x41
0064:    74 02                   je       0x68    ; Conditional jump
0066:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0068:    D8 D2                   fcom     st(2)
006A:    9B                      wait
006B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006D:    80 E4 41                and      ah, 0x41
0070:    74 02                   je       0x74    ; Conditional jump
0072:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0074:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0076:    D8 D2                   fcom     st(2)
0078:    9B                      wait
0079:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
007B:    80 E4 41                and      ah, 0x41
007E:    74 02                   je       0x82    ; Conditional jump
0080:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0082:    DD 47 A4                fld      qword ptr [edi - 0x5c]
0085:    DC CB                   fmul     st(3), st(0)
0087:    DD D8                   fstp     st(0)
0089:    DD 47 AC                fld      qword ptr [edi - 0x54]
008C:    DC CA                   fmul     st(2), st(0)
008E:    DD D8                   fstp     st(0)
0090:    DD 47 9C                fld      qword ptr [edi - 0x64]
0093:    DC C9                   fmul     st(1), st(0)
0095:    DD D8                   fstp     st(0)
0097:    DE C1                   faddp    st(1)
0099:    DE C1                   faddp    st(1)
009B:    DC 67 F0                fsub     qword ptr [edi - 0x10]
009E:    DD 04 24                fld      qword ptr [esp]
00A1:    D8 C8                   fmul     st(0)
00A3:    DD 44 24 08             fld      qword ptr [esp + 8]
00A7:    D8 C8                   fmul     st(0)
00A9:    DE C1                   faddp    st(1)
00AB:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00AF:    D8 C8                   fmul     st(0)
00B1:    DE C1                   faddp    st(1)
00B3:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B5:    DD 47 B4                fld      qword ptr [edi - 0x4c]
00B8:    D9 E1                   fabs    ; |ST(0)|
00BA:    DE E9                   fsubp    st(1)
00BC:    8B 57 B8                mov      edx, dword ptr [edi - 0x48]
00BF:    81 E2 00 00 00 80       and      edx, 0x80000000
00C5:    75 12                   jne      0xd9    ; Conditional jump
00C7:    D8 D1                   fcom     st(1)
00C9:    9B                      wait
00CA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CC:    80 E4 41                and      ah, 0x41
00CF:    74 04                   je       0xd5    ; Conditional jump
00D1:    DD D9                   fstp     st(1)
00D3:    EB 14                   jmp      0xe9
00D5:    DD D8                   fstp     st(0)
00D7:    EB 10                   jmp      0xe9
00D9:    D8 D1                   fcom     st(1)
00DB:    9B                      wait
00DC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DE:    80 E4 41                and      ah, 0x41
00E1:    74 04                   je       0xe7    ; Conditional jump
00E3:    DD D8                   fstp     st(0)
00E5:    EB 02                   jmp      0xe9
00E7:    DD D9                   fstp     st(1)
00E9:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
00ED:    75 03                   jne      0xf2    ; Conditional jump
00EF:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00F2:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00F5:    83 C4 1C                add      esp, 0x1c
00F8:    C3                      ret
