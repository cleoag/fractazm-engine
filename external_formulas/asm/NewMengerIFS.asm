; Disassembly: NewMengerIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 572 bytes, decoded: 572 bytes (100.0%)
; DEoption: 20

0000:    83 C4 A6                add      esp, -0x5a
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
004B:    8B 57 98                mov      edx, dword ptr [edi - 0x68]
004E:    83 E2 1F                and      edx, 0x1f
0051:    83 FA 00                cmp      edx, 0
0054:    75 05                   jne      0x5b    ; Conditional jump
0056:    BA 01 00 00 00          mov      edx, 1
005B:    DD 04 24                fld      qword ptr [esp]
005E:    DC 0F                   fmul     qword ptr [edi]
0060:    DC 07                   fadd     qword ptr [edi]
0062:    DD 1C 24                fstp     qword ptr [esp]
0065:    DD 44 24 08             fld      qword ptr [esp + 8]
0069:    DC 0F                   fmul     qword ptr [edi]
006B:    DC 07                   fadd     qword ptr [edi]
006D:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0071:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0075:    DC 0F                   fmul     qword ptr [edi]
0077:    DC 07                   fadd     qword ptr [edi]
0079:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
007D:    DD 04 24                fld      qword ptr [esp]
0080:    DC 27                   fsub     qword ptr [edi]
0082:    D9 E1                   fabs    ; |ST(0)|
0084:    DC 27                   fsub     qword ptr [edi]
0086:    DD 54 24 18             fst      qword ptr [esp + 0x18]
008A:    DD 44 24 08             fld      qword ptr [esp + 8]
008E:    DC 27                   fsub     qword ptr [edi]
0090:    D9 E1                   fabs    ; |ST(0)|
0092:    DC 27                   fsub     qword ptr [edi]
0094:    DD 54 24 20             fst      qword ptr [esp + 0x20]
0098:    DD 44 24 10             fld      qword ptr [esp + 0x10]
009C:    DC 27                   fsub     qword ptr [edi]
009E:    D9 E1                   fabs    ; |ST(0)|
00A0:    DC 27                   fsub     qword ptr [edi]
00A2:    DD 54 24 28             fst      qword ptr [esp + 0x28]
00A6:    D9 E8                   fld1    ; Load 1.0
00A8:    DD 5C 24 30             fstp     qword ptr [esp + 0x30]
00AC:    D8 D1                   fcom     st(1)
00AE:    9B                      wait
00AF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B1:    80 E4 41                and      ah, 0x41
00B4:    75 04                   jne      0xba    ; Conditional jump
00B6:    DD D9                   fstp     st(1)
00B8:    EB 02                   jmp      0xbc
00BA:    DD D8                   fstp     st(0)
00BC:    D8 D1                   fcom     st(1)
00BE:    9B                      wait
00BF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C1:    80 E4 41                and      ah, 0x41
00C4:    75 04                   jne      0xca    ; Conditional jump
00C6:    DD D9                   fstp     st(1)
00C8:    EB 02                   jmp      0xcc
00CA:    DD D8                   fstp     st(0)
00CC:    DD 54 24 38             fst      qword ptr [esp + 0x38]
00D0:    DD 5C 24 40             fstp     qword ptr [esp + 0x40]
00D4:    DD 47 B8                fld      qword ptr [edi - 0x48]
00D7:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00DB:    D8 C9                   fmul     st(1)
00DD:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
00E1:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
00E3:    9B                      wait
00E4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E6:    9B                      wait
00E7:    9E                      sahf
00E8:    7A F7                   jp       0xe1    ; Conditional jump
00EA:    DD D9                   fstp     st(1)
00EC:    DC A7 6C FF FF FF       fsub     qword ptr [edi - 0x94]
00F2:    D9 E1                   fabs    ; |ST(0)|
00F4:    D9 E0                   fchs    ; -ST(0)
00F6:    DC 07                   fadd     qword ptr [edi]
00F8:    DC 47 9C                fadd     qword ptr [edi - 0x64]
00FB:    DD 47 B8                fld      qword ptr [edi - 0x48]
00FE:    DD 44 24 08             fld      qword ptr [esp + 8]
0102:    D8 C9                   fmul     st(1)
0104:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
0108:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
010A:    9B                      wait
010B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
010D:    9B                      wait
010E:    9E                      sahf
010F:    7A F7                   jp       0x108    ; Conditional jump
0111:    DD D9                   fstp     st(1)
0113:    DC A7 6C FF FF FF       fsub     qword ptr [edi - 0x94]
0119:    D9 E1                   fabs    ; |ST(0)|
011B:    D9 E0                   fchs    ; -ST(0)
011D:    DC 07                   fadd     qword ptr [edi]
011F:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
0122:    DD 47 B8                fld      qword ptr [edi - 0x48]
0125:    DD 04 24                fld      qword ptr [esp]
0128:    D8 C9                   fmul     st(1)
012A:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
012E:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
0130:    9B                      wait
0131:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0133:    9B                      wait
0134:    9E                      sahf
0135:    7A F7                   jp       0x12e    ; Conditional jump
0137:    DD D9                   fstp     st(1)
0139:    DC A7 6C FF FF FF       fsub     qword ptr [edi - 0x94]
013F:    D9 E1                   fabs    ; |ST(0)|
0141:    D9 E0                   fchs    ; -ST(0)
0143:    DC 07                   fadd     qword ptr [edi]
0145:    DC 47 AC                fadd     qword ptr [edi - 0x54]
0148:    D9 C0                   fld      st(0)
014A:    D8 8F 7C FF FF FF       fmul     dword ptr [edi - 0x84]
0150:    D9 C2                   fld      st(2)
0152:    D8 8F 78 FF FF FF       fmul     dword ptr [edi - 0x88]
0158:    DE C1                   faddp    st(1)
015A:    D9 C3                   fld      st(3)
015C:    D8 8F 74 FF FF FF       fmul     dword ptr [edi - 0x8c]
0162:    DE C1                   faddp    st(1)
0164:    D9 C1                   fld      st(1)
0166:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
0169:    D9 C3                   fld      st(3)
016B:    D8 4F 90                fmul     dword ptr [edi - 0x70]
016E:    DE C1                   faddp    st(1)
0170:    D9 C4                   fld      st(4)
0172:    D8 4F 8C                fmul     dword ptr [edi - 0x74]
0175:    DE C1                   faddp    st(1)
0177:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0179:    D8 4F 88                fmul     dword ptr [edi - 0x78]
017C:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
017E:    D8 4F 84                fmul     dword ptr [edi - 0x7c]
0181:    DE C3                   faddp    st(3)
0183:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0185:    D8 4F 80                fmul     dword ptr [edi - 0x80]
0188:    DE C2                   faddp    st(2)
018A:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
018E:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0192:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
0196:    DD 44 24 18             fld      qword ptr [esp + 0x18]
019A:    DD 44 24 20             fld      qword ptr [esp + 0x20]
019E:    D8 D1                   fcom     st(1)
01A0:    9B                      wait
01A1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01A3:    80 E4 41                and      ah, 0x41
01A6:    75 04                   jne      0x1ac    ; Conditional jump
01A8:    DD D9                   fstp     st(1)
01AA:    EB 02                   jmp      0x1ae
01AC:    DD D8                   fstp     st(0)
01AE:    DD 44 24 18             fld      qword ptr [esp + 0x18]
01B2:    DD 44 24 28             fld      qword ptr [esp + 0x28]
01B6:    D8 D1                   fcom     st(1)
01B8:    9B                      wait
01B9:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01BB:    80 E4 41                and      ah, 0x41
01BE:    75 04                   jne      0x1c4    ; Conditional jump
01C0:    DD D9                   fstp     st(1)
01C2:    EB 02                   jmp      0x1c6
01C4:    DD D8                   fstp     st(0)
01C6:    DD 44 24 20             fld      qword ptr [esp + 0x20]
01CA:    DD 44 24 28             fld      qword ptr [esp + 0x28]
01CE:    D8 D1                   fcom     st(1)
01D0:    9B                      wait
01D1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01D3:    80 E4 41                and      ah, 0x41
01D6:    75 04                   jne      0x1dc    ; Conditional jump
01D8:    DD D9                   fstp     st(1)
01DA:    EB 02                   jmp      0x1de
01DC:    DD D8                   fstp     st(0)
01DE:    D8 D1                   fcom     st(1)
01E0:    9B                      wait
01E1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01E3:    80 E4 41                and      ah, 0x41
01E6:    74 04                   je       0x1ec    ; Conditional jump
01E8:    DD D9                   fstp     st(1)
01EA:    EB 02                   jmp      0x1ee
01EC:    DD D8                   fstp     st(0)
01EE:    D8 D1                   fcom     st(1)
01F0:    9B                      wait
01F1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01F3:    80 E4 41                and      ah, 0x41
01F6:    74 04                   je       0x1fc    ; Conditional jump
01F8:    DD D9                   fstp     st(1)
01FA:    EB 02                   jmp      0x1fe
01FC:    DD D8                   fstp     st(0)
01FE:    DC 74 24 30             fdiv     qword ptr [esp + 0x30]
0202:    DD 54 24 38             fst      qword ptr [esp + 0x38]
0206:    DD 44 24 40             fld      qword ptr [esp + 0x40]
020A:    D8 D1                   fcom     st(1)
020C:    9B                      wait
020D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
020F:    80 E4 41                and      ah, 0x41
0212:    75 04                   jne      0x218    ; Conditional jump
0214:    DD D9                   fstp     st(1)
0216:    EB 02                   jmp      0x21a
0218:    DD D8                   fstp     st(0)
021A:    DD 47 F0                fld      qword ptr [edi - 0x10]
021D:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
0221:    DD 5C 24 30             fstp     qword ptr [esp + 0x30]
0225:    4A                      dec      edx
0226:    0F 8F A4 FE FF FF       jg       0xd0    ; Conditional jump
022C:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0230:    75 03                   jne      0x235    ; Conditional jump
0232:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0235:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0238:    83 C4 5A                add      esp, 0x5a
023B:    C3                      ret
