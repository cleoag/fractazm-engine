; Disassembly: NewFoldCutIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 559 bytes, decoded: 559 bytes (100.0%)
; DEoption: 20

0000:    83 C4 A6                add      esp, -0x5a
0003:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0008:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000D:    66 0F 59 47 E0          mulpd    xmm0, xmmword ptr [edi - 0x20]
0012:    F2 0F 59 4F E0          mulsd    xmm1, qword ptr [edi - 0x20]
0017:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
001C:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
0022:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
0027:    F2 0F 58 4F D0          addsd    xmm1, qword ptr [edi - 0x30]
002C:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0030:    7C 19                   jl       0x4b    ; Conditional jump
0032:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
0037:    F2 0F 59 6F E0          mulsd    xmm5, qword ptr [edi - 0x20]
003C:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0041:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
0046:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
004B:    D9 E8                   fld1    ; Load 1.0
004D:    DD 5C 24 40             fstp     qword ptr [esp + 0x40]
0051:    DD 07                   fld      qword ptr [edi]
0053:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0056:    8B 57 98                mov      edx, dword ptr [edi - 0x68]
0059:    83 E2 1F                and      edx, 0x1f
005C:    83 FA 00                cmp      edx, 0
005F:    75 05                   jne      0x66    ; Conditional jump
0061:    BA 01 00 00 00          mov      edx, 1
0066:    DD 44 24 10             fld      qword ptr [esp + 0x10]
006A:    DD 44 24 08             fld      qword ptr [esp + 8]
006E:    DD 04 24                fld      qword ptr [esp]
0071:    D9 C0                   fld      st(0)
0073:    D8 8F 7C FF FF FF       fmul     dword ptr [edi - 0x84]
0079:    D9 C2                   fld      st(2)
007B:    D8 8F 78 FF FF FF       fmul     dword ptr [edi - 0x88]
0081:    DE C1                   faddp    st(1)
0083:    D9 C3                   fld      st(3)
0085:    D8 8F 74 FF FF FF       fmul     dword ptr [edi - 0x8c]
008B:    DE C1                   faddp    st(1)
008D:    D9 C1                   fld      st(1)
008F:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
0092:    D9 C3                   fld      st(3)
0094:    D8 4F 90                fmul     dword ptr [edi - 0x70]
0097:    DE C1                   faddp    st(1)
0099:    D9 C4                   fld      st(4)
009B:    D8 4F 8C                fmul     dword ptr [edi - 0x74]
009E:    DE C1                   faddp    st(1)
00A0:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00A2:    D8 4F 88                fmul     dword ptr [edi - 0x78]
00A5:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A7:    D8 4F 84                fmul     dword ptr [edi - 0x7c]
00AA:    DE C3                   faddp    st(3)
00AC:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00AE:    D8 4F 80                fmul     dword ptr [edi - 0x80]
00B1:    DE C2                   faddp    st(2)
00B3:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00B6:    DD 54 24 18             fst      qword ptr [esp + 0x18]
00BA:    D9 E1                   fabs    ; |ST(0)|
00BC:    DD 54 24 30             fst      qword ptr [esp + 0x30]
00C0:    DC 67 C0                fsub     qword ptr [edi - 0x40]
00C3:    DD 1C 24                fstp     qword ptr [esp]
00C6:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00C9:    DD 54 24 20             fst      qword ptr [esp + 0x20]
00CD:    D9 E1                   fabs    ; |ST(0)|
00CF:    D9 C0                   fld      st(0)
00D1:    DD 44 24 30             fld      qword ptr [esp + 0x30]
00D5:    D8 D1                   fcom     st(1)
00D7:    9B                      wait
00D8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DA:    80 E4 41                and      ah, 0x41
00DD:    75 04                   jne      0xe3    ; Conditional jump
00DF:    DD D9                   fstp     st(1)
00E1:    EB 02                   jmp      0xe5
00E3:    DD D8                   fstp     st(0)
00E5:    DD 5C 24 30             fstp     qword ptr [esp + 0x30]
00E9:    DC 67 C8                fsub     qword ptr [edi - 0x38]
00EC:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00F0:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00F3:    DD 54 24 28             fst      qword ptr [esp + 0x28]
00F7:    D9 E1                   fabs    ; |ST(0)|
00F9:    D9 C0                   fld      st(0)
00FB:    DD 44 24 30             fld      qword ptr [esp + 0x30]
00FF:    D8 D1                   fcom     st(1)
0101:    9B                      wait
0102:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0104:    80 E4 41                and      ah, 0x41
0107:    75 04                   jne      0x10d    ; Conditional jump
0109:    DD D9                   fstp     st(1)
010B:    EB 02                   jmp      0x10f
010D:    DD D8                   fstp     st(0)
010F:    DD 5C 24 30             fstp     qword ptr [esp + 0x30]
0113:    DC 67 D0                fsub     qword ptr [edi - 0x30]
0116:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
011A:    DD 44 24 10             fld      qword ptr [esp + 0x10]
011E:    DD 44 24 08             fld      qword ptr [esp + 8]
0122:    DD 04 24                fld      qword ptr [esp]
0125:    D8 D1                   fcom     st(1)
0127:    9B                      wait
0128:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
012A:    80 E4 41                and      ah, 0x41
012D:    74 02                   je       0x131    ; Conditional jump
012F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0131:    D8 D2                   fcom     st(2)
0133:    9B                      wait
0134:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0136:    80 E4 41                and      ah, 0x41
0139:    74 02                   je       0x13d    ; Conditional jump
013B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
013D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
013F:    D8 D2                   fcom     st(2)
0141:    9B                      wait
0142:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0144:    80 E4 41                and      ah, 0x41
0147:    74 02                   je       0x14b    ; Conditional jump
0149:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
014B:    DC 67 A4                fsub     qword ptr [edi - 0x5c]
014E:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0152:    DC 67 AC                fsub     qword ptr [edi - 0x54]
0155:    DD 1C 24                fstp     qword ptr [esp]
0158:    DC 67 9C                fsub     qword ptr [edi - 0x64]
015B:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
015F:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0163:    DD 44 24 08             fld      qword ptr [esp + 8]
0167:    DD 04 24                fld      qword ptr [esp]
016A:    D8 D1                   fcom     st(1)
016C:    9B                      wait
016D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
016F:    80 E4 41                and      ah, 0x41
0172:    74 02                   je       0x176    ; Conditional jump
0174:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0176:    D8 D2                   fcom     st(2)
0178:    9B                      wait
0179:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
017B:    80 E4 41                and      ah, 0x41
017E:    74 02                   je       0x182    ; Conditional jump
0180:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0182:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0184:    D8 D2                   fcom     st(2)
0186:    9B                      wait
0187:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0189:    80 E4 41                and      ah, 0x41
018C:    74 02                   je       0x190    ; Conditional jump
018E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0190:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0194:    DD 1C 24                fstp     qword ptr [esp]
0197:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
019B:    DD 44 24 40             fld      qword ptr [esp + 0x40]
019F:    DC 4F F0                fmul     qword ptr [edi - 0x10]
01A2:    D9 E1                   fabs    ; |ST(0)|
01A4:    D9 C0                   fld      st(0)
01A6:    DD 5C 24 40             fstp     qword ptr [esp + 0x40]
01AA:    D9 E8                   fld1    ; Load 1.0
01AC:    DC 6C 24 30             fsubr    qword ptr [esp + 0x30]
01B0:    DD 04 24                fld      qword ptr [esp]
01B3:    D8 4F B8                fmul     dword ptr [edi - 0x48]
01B6:    DD 44 24 08             fld      qword ptr [esp + 8]
01BA:    D8 4F BC                fmul     dword ptr [edi - 0x44]
01BD:    DE C1                   faddp    st(1)
01BF:    DD 47 F0                fld      qword ptr [edi - 0x10]
01C2:    D9 EE                   fldz    ; Load 0.0
01C4:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
01C6:    9B                      wait
01C7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01C9:    80 E4 41                and      ah, 0x41
01CC:    75 02                   jne      0x1d0    ; Conditional jump
01CE:    D9 E0                   fchs    ; -ST(0)
01D0:    D8 D1                   fcom     st(1)
01D2:    9B                      wait
01D3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01D5:    80 E4 41                and      ah, 0x41
01D8:    75 04                   jne      0x1de    ; Conditional jump
01DA:    DD D9                   fstp     st(1)
01DC:    EB 02                   jmp      0x1e0
01DE:    DD D8                   fstp     st(0)
01E0:    DE F1                   fdivrp   st(1)
01E2:    DD 46 E0                fld      qword ptr [esi - 0x20]
01E5:    D8 D1                   fcom     st(1)
01E7:    9B                      wait
01E8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01EA:    80 E4 41                and      ah, 0x41
01ED:    74 04                   je       0x1f3    ; Conditional jump
01EF:    DD D9                   fstp     st(1)
01F1:    EB 02                   jmp      0x1f5
01F3:    DD D8                   fstp     st(0)
01F5:    DD 5E E0                fstp     qword ptr [esi - 0x20]
01F8:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
01FC:    7F 17                   jg       0x215    ; Conditional jump
01FE:    DD 44 24 28             fld      qword ptr [esp + 0x28]
0202:    DD 44 24 20             fld      qword ptr [esp + 0x20]
0206:    DD 44 24 18             fld      qword ptr [esp + 0x18]
020A:    DD 1C 24                fstp     qword ptr [esp]
020D:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0211:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0215:    4A                      dec      edx
0216:    0F 8F 4A FE FF FF       jg       0x66    ; Conditional jump
021C:    DD 46 E0                fld      qword ptr [esi - 0x20]
021F:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0223:    7C 03                   jl       0x228    ; Conditional jump
0225:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0228:    DD 5E E0                fstp     qword ptr [esi - 0x20]
022B:    83 C4 5A                add      esp, 0x5a
022E:    C3                      ret
