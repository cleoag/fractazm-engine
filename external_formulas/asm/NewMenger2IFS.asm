; Disassembly: NewMenger2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 622 bytes, decoded: 622 bytes (100.0%)
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
00F2:    DD 47 B8                fld      qword ptr [edi - 0x48]
00F5:    DD 44 24 08             fld      qword ptr [esp + 8]
00F9:    D8 C9                   fmul     st(1)
00FB:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
00FF:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
0101:    9B                      wait
0102:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0104:    9B                      wait
0105:    9E                      sahf
0106:    7A F7                   jp       0xff    ; Conditional jump
0108:    DD D9                   fstp     st(1)
010A:    DC A7 6C FF FF FF       fsub     qword ptr [edi - 0x94]
0110:    DD 47 B8                fld      qword ptr [edi - 0x48]
0113:    DD 04 24                fld      qword ptr [esp]
0116:    D8 C9                   fmul     st(1)
0118:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
011C:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
011E:    9B                      wait
011F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0121:    9B                      wait
0122:    9E                      sahf
0123:    7A F7                   jp       0x11c    ; Conditional jump
0125:    DD D9                   fstp     st(1)
0127:    DC A7 6C FF FF FF       fsub     qword ptr [edi - 0x94]
012D:    D9 C0                   fld      st(0)
012F:    D8 8F 7C FF FF FF       fmul     dword ptr [edi - 0x84]
0135:    D9 C2                   fld      st(2)
0137:    D8 8F 78 FF FF FF       fmul     dword ptr [edi - 0x88]
013D:    DE C1                   faddp    st(1)
013F:    D9 C3                   fld      st(3)
0141:    D8 8F 74 FF FF FF       fmul     dword ptr [edi - 0x8c]
0147:    DE C1                   faddp    st(1)
0149:    D9 C1                   fld      st(1)
014B:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
014E:    D9 C3                   fld      st(3)
0150:    D8 4F 90                fmul     dword ptr [edi - 0x70]
0153:    DE C1                   faddp    st(1)
0155:    D9 C4                   fld      st(4)
0157:    D8 4F 8C                fmul     dword ptr [edi - 0x74]
015A:    DE C1                   faddp    st(1)
015C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
015E:    D8 4F 88                fmul     dword ptr [edi - 0x78]
0161:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0163:    D8 4F 84                fmul     dword ptr [edi - 0x7c]
0166:    DE C3                   faddp    st(3)
0168:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
016A:    D8 4F 80                fmul     dword ptr [edi - 0x80]
016D:    DE C2                   faddp    st(2)
016F:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0173:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0177:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
017B:    DD 44 24 18             fld      qword ptr [esp + 0x18]
017F:    D8 C8                   fmul     st(0)
0181:    DD 44 24 20             fld      qword ptr [esp + 0x20]
0185:    D8 C8                   fmul     st(0)
0187:    DE C1                   faddp    st(1)
0189:    D9 FA                   fsqrt    ; sqrt(ST(0))
018B:    DD 47 9C                fld      qword ptr [edi - 0x64]
018E:    DC 4C 24 28             fmul     qword ptr [esp + 0x28]
0192:    DC 4C 24 28             fmul     qword ptr [esp + 0x28]
0196:    DE E9                   fsubp    st(1)
0198:    DD 44 24 28             fld      qword ptr [esp + 0x28]
019C:    D9 E1                   fabs    ; |ST(0)|
019E:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
01A1:    DE E9                   fsubp    st(1)
01A3:    DC 67 AC                fsub     qword ptr [edi - 0x54]
01A6:    DD 44 24 18             fld      qword ptr [esp + 0x18]
01AA:    D8 C8                   fmul     st(0)
01AC:    DD 44 24 28             fld      qword ptr [esp + 0x28]
01B0:    D8 C8                   fmul     st(0)
01B2:    DE C1                   faddp    st(1)
01B4:    D9 FA                   fsqrt    ; sqrt(ST(0))
01B6:    DD 47 9C                fld      qword ptr [edi - 0x64]
01B9:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
01BD:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
01C1:    DE E9                   fsubp    st(1)
01C3:    DD 44 24 20             fld      qword ptr [esp + 0x20]
01C7:    D9 E1                   fabs    ; |ST(0)|
01C9:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
01CC:    DE E9                   fsubp    st(1)
01CE:    DC 67 AC                fsub     qword ptr [edi - 0x54]
01D1:    DD 44 24 28             fld      qword ptr [esp + 0x28]
01D5:    D8 C8                   fmul     st(0)
01D7:    DD 44 24 20             fld      qword ptr [esp + 0x20]
01DB:    D8 C8                   fmul     st(0)
01DD:    DE C1                   faddp    st(1)
01DF:    D9 FA                   fsqrt    ; sqrt(ST(0))
01E1:    DD 47 9C                fld      qword ptr [edi - 0x64]
01E4:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
01E8:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
01EC:    DE E9                   fsubp    st(1)
01EE:    DD 44 24 18             fld      qword ptr [esp + 0x18]
01F2:    D9 E1                   fabs    ; |ST(0)|
01F4:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
01F7:    DE E9                   fsubp    st(1)
01F9:    DC 67 AC                fsub     qword ptr [edi - 0x54]
01FC:    D8 D1                   fcom     st(1)
01FE:    9B                      wait
01FF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0201:    80 E4 41                and      ah, 0x41
0204:    74 04                   je       0x20a    ; Conditional jump
0206:    DD D9                   fstp     st(1)
0208:    EB 02                   jmp      0x20c
020A:    DD D8                   fstp     st(0)
020C:    D8 D1                   fcom     st(1)
020E:    9B                      wait
020F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0211:    80 E4 41                and      ah, 0x41
0214:    74 04                   je       0x21a    ; Conditional jump
0216:    DD D9                   fstp     st(1)
0218:    EB 02                   jmp      0x21c
021A:    DD D8                   fstp     st(0)
021C:    D9 E8                   fld1    ; Load 1.0
021E:    DD 47 9C                fld      qword ptr [edi - 0x64]
0221:    D9 E1                   fabs    ; |ST(0)|
0223:    DE C1                   faddp    st(1)
0225:    DD 47 A4                fld      qword ptr [edi - 0x5c]
0228:    D9 E1                   fabs    ; |ST(0)|
022A:    DE C1                   faddp    st(1)
022C:    DE F9                   fdivp    st(1)
022E:    DC 74 24 30             fdiv     qword ptr [esp + 0x30]
0232:    D9 E0                   fchs    ; -ST(0)
0234:    DD 54 24 38             fst      qword ptr [esp + 0x38]
0238:    DD 44 24 40             fld      qword ptr [esp + 0x40]
023C:    D8 D1                   fcom     st(1)
023E:    9B                      wait
023F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0241:    80 E4 41                and      ah, 0x41
0244:    75 04                   jne      0x24a    ; Conditional jump
0246:    DD D9                   fstp     st(1)
0248:    EB 02                   jmp      0x24c
024A:    DD D8                   fstp     st(0)
024C:    DD 47 F0                fld      qword ptr [edi - 0x10]
024F:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
0253:    DD 5C 24 30             fstp     qword ptr [esp + 0x30]
0257:    4A                      dec      edx
0258:    0F 8F 72 FE FF FF       jg       0xd0    ; Conditional jump
025E:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0262:    75 03                   jne      0x267    ; Conditional jump
0264:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0267:    DD 5E E0                fstp     qword ptr [esi - 0x20]
026A:    83 C4 5A                add      esp, 0x5a
026D:    C3                      ret
