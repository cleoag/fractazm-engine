; Disassembly: helistairsIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 600 bytes, decoded: 600 bytes (100.0%)
; DEoption: 20

0000:    83 C4 B0                add      esp, -0x50
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
004B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
004F:    DD 44 24 08             fld      qword ptr [esp + 8]
0053:    DD 04 24                fld      qword ptr [esp]
0056:    D9 C0                   fld      st(0)
0058:    D8 4F 98                fmul     dword ptr [edi - 0x68]
005B:    D9 C2                   fld      st(2)
005D:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
0060:    DE C1                   faddp    st(1)
0062:    D9 C3                   fld      st(3)
0064:    D8 4F 90                fmul     dword ptr [edi - 0x70]
0067:    DE C1                   faddp    st(1)
0069:    D9 C1                   fld      st(1)
006B:    D8 4F B0                fmul     dword ptr [edi - 0x50]
006E:    D9 C3                   fld      st(3)
0070:    D8 4F AC                fmul     dword ptr [edi - 0x54]
0073:    DE C1                   faddp    st(1)
0075:    D9 C4                   fld      st(4)
0077:    D8 4F A8                fmul     dword ptr [edi - 0x58]
007A:    DE C1                   faddp    st(1)
007C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007E:    D8 4F A4                fmul     dword ptr [edi - 0x5c]
0081:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0083:    D8 4F A0                fmul     dword ptr [edi - 0x60]
0086:    DE C3                   faddp    st(3)
0088:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008A:    D8 4F 9C                fmul     dword ptr [edi - 0x64]
008D:    DE C2                   faddp    st(2)
008F:    DD 1C 24                fstp     qword ptr [esp]
0092:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0096:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
009A:    DD 87 74 FF FF FF       fld      qword ptr [edi - 0x8c]
00A0:    D9 E4                   ftst
00A2:    9B                      wait
00A3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A5:    80 E4 41                and      ah, 0x41
00A8:    75 11                   jne      0xbb    ; Conditional jump
00AA:    DD 04 24                fld      qword ptr [esp]
00AD:    D8 C8                   fmul     st(0)
00AF:    DD 44 24 08             fld      qword ptr [esp + 8]
00B3:    D8 C8                   fmul     st(0)
00B5:    DE C1                   faddp    st(1)
00B7:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B9:    EB 1B                   jmp      0xd6
00BB:    DD 04 24                fld      qword ptr [esp]
00BE:    D9 E1                   fabs    ; |ST(0)|
00C0:    DD 44 24 08             fld      qword ptr [esp + 8]
00C4:    D9 E1                   fabs    ; |ST(0)|
00C6:    D8 D1                   fcom     st(1)
00C8:    9B                      wait
00C9:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CB:    80 E4 41                and      ah, 0x41
00CE:    75 04                   jne      0xd4    ; Conditional jump
00D0:    DD D9                   fstp     st(1)
00D2:    EB 02                   jmp      0xd6
00D4:    DD D8                   fstp     st(0)
00D6:    DD D9                   fstp     st(1)
00D8:    DD 44 24 08             fld      qword ptr [esp + 8]
00DC:    DD 04 24                fld      qword ptr [esp]
00DF:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00E1:    DC 4F 08                fmul     qword ptr [edi + 8]
00E4:    DA 4F 8C                fimul    dword ptr [edi - 0x74]
00E7:    DB 54 24 18             fist     dword ptr [esp + 0x18]
00EB:    DB 44 24 18             fild     dword ptr [esp + 0x18]
00EF:    DE E9                   fsubp    st(1)
00F1:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00F4:    DA 77 8C                fidiv    dword ptr [edi - 0x74]
00F7:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00FB:    DD 1C 24                fstp     qword ptr [esp]
00FE:    DD 47 84                fld      qword ptr [edi - 0x7c]
0101:    D9 E1                   fabs    ; |ST(0)|
0103:    D9 E0                   fchs    ; -ST(0)
0105:    D9 C0                   fld      st(0)
0107:    DC 4F 18                fmul     qword ptr [edi + 0x18]
010A:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
010E:    DA 4F 8C                fimul    dword ptr [edi - 0x74]
0111:    D9 E1                   fabs    ; |ST(0)|
0113:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0117:    DD 44 24 10             fld      qword ptr [esp + 0x10]
011B:    DB 44 24 18             fild     dword ptr [esp + 0x18]
011F:    DD 47 84                fld      qword ptr [edi - 0x7c]
0122:    D9 E1                   fabs    ; |ST(0)|
0124:    D9 E0                   fchs    ; -ST(0)
0126:    DE C9                   fmulp    st(1)
0128:    DE C1                   faddp    st(1)
012A:    D9 C0                   fld      st(0)
012C:    DC 74 24 20             fdiv     qword ptr [esp + 0x20]
0130:    D9 FC                   frndint    ; round(ST(0))
0132:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
0136:    DE E9                   fsubp    st(1)
0138:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
013C:    8B 47 88                mov      eax, dword ptr [edi - 0x78]
013F:    25 00 00 00 80          and      eax, 0x80000000
0144:    74 4C                   je       0x192    ; Conditional jump
0146:    DD 44 24 08             fld      qword ptr [esp + 8]
014A:    DC 64 24 28             fsub     qword ptr [esp + 0x28]
014E:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0152:    D9 E0                   fchs    ; -ST(0)
0154:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0156:    9B                      wait
0157:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0159:    80 E4 41                and      ah, 0x41
015C:    75 0A                   jne      0x168    ; Conditional jump
015E:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0162:    DC 64 24 28             fsub     qword ptr [esp + 0x28]
0166:    EB 28                   jmp      0x190
0168:    DD 44 24 08             fld      qword ptr [esp + 8]
016C:    DC 44 24 28             fadd     qword ptr [esp + 0x28]
0170:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0174:    D9 E0                   fchs    ; -ST(0)
0176:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0178:    9B                      wait
0179:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
017B:    80 E4 41                and      ah, 0x41
017E:    74 0A                   je       0x18a    ; Conditional jump
0180:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0184:    DC 44 24 28             fadd     qword ptr [esp + 0x28]
0188:    EB 06                   jmp      0x190
018A:    DD 44 24 08             fld      qword ptr [esp + 8]
018E:    D9 E0                   fchs    ; -ST(0)
0190:    EB 76                   jmp      0x208
0192:    8B 44 24 14             mov      eax, dword ptr [esp + 0x14]
0196:    25 00 00 00 80          and      eax, 0x80000000
019B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
019F:    D9 E1                   fabs    ; |ST(0)|
01A1:    DC 44 24 28             fadd     qword ptr [esp + 0x28]
01A5:    D9 E1                   fabs    ; |ST(0)|
01A7:    DD 44 24 08             fld      qword ptr [esp + 8]
01AB:    85 C0                   test     eax, eax
01AD:    75 02                   jne      0x1b1    ; Conditional jump
01AF:    D9 E0                   fchs    ; -ST(0)
01B1:    D8 D1                   fcom     st(1)
01B3:    9B                      wait
01B4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01B6:    80 E4 41                and      ah, 0x41
01B9:    75 04                   jne      0x1bf    ; Conditional jump
01BB:    DD D9                   fstp     st(1)
01BD:    EB 02                   jmp      0x1c1
01BF:    DD D8                   fstp     st(0)
01C1:    DC A7 7C FF FF FF       fsub     qword ptr [edi - 0x84]
01C7:    DD 04 24                fld      qword ptr [esp]
01CA:    DD 87 74 FF FF FF       fld      qword ptr [edi - 0x8c]
01D0:    D9 E1                   fabs    ; |ST(0)|
01D2:    DE E9                   fsubp    st(1)
01D4:    DD 56 E0                fst      qword ptr [esi - 0x20]
01D7:    DC A7 6C FF FF FF       fsub     qword ptr [edi - 0x94]
01DD:    D8 D1                   fcom     st(1)
01DF:    9B                      wait
01E0:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01E2:    80 E4 41                and      ah, 0x41
01E5:    75 04                   jne      0x1eb    ; Conditional jump
01E7:    DD D9                   fstp     st(1)
01E9:    EB 02                   jmp      0x1ed
01EB:    DD D8                   fstp     st(0)
01ED:    DD 46 E0                fld      qword ptr [esi - 0x20]
01F0:    DC 87 6C FF FF FF       fadd     qword ptr [edi - 0x94]
01F6:    D8 D1                   fcom     st(1)
01F8:    9B                      wait
01F9:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01FB:    80 E4 41                and      ah, 0x41
01FE:    74 04                   je       0x204    ; Conditional jump
0200:    DD D9                   fstp     st(1)
0202:    EB 02                   jmp      0x206
0204:    DD D8                   fstp     st(0)
0206:    EB 2D                   jmp      0x235
0208:    D9 E1                   fabs    ; |ST(0)|
020A:    DC A7 7C FF FF FF       fsub     qword ptr [edi - 0x84]
0210:    DD 04 24                fld      qword ptr [esp]
0213:    DD 87 74 FF FF FF       fld      qword ptr [edi - 0x8c]
0219:    D9 E1                   fabs    ; |ST(0)|
021B:    DE E9                   fsubp    st(1)
021D:    D9 E1                   fabs    ; |ST(0)|
021F:    DC A7 6C FF FF FF       fsub     qword ptr [edi - 0x94]
0225:    D8 D1                   fcom     st(1)
0227:    9B                      wait
0228:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
022A:    80 E4 41                and      ah, 0x41
022D:    75 04                   jne      0x233    ; Conditional jump
022F:    DD D9                   fstp     st(1)
0231:    EB 02                   jmp      0x235
0233:    DD D8                   fstp     st(0)
0235:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
0239:    75 03                   jne      0x23e    ; Conditional jump
023B:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
023E:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0241:    DD 47 F0                fld      qword ptr [edi - 0x10]
0244:    DB 46 E8                fild     dword ptr [esi - 0x18]
0247:    DC 4F B4                fmul     qword ptr [edi - 0x4c]
024A:    DE C1                   faddp    st(1)
024C:    D9 E1                   fabs    ; |ST(0)|
024E:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0254:    83 C4 50                add      esp, 0x50
0257:    C3                      ret
