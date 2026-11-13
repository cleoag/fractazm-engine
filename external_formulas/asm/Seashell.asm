; Disassembly: Seashell.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 395 bytes, decoded: 395 bytes (100.0%)
; DEoption: 20

0000:    83 C4 CC                add      esp, -0x34
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
0058:    D8 4F 90                fmul     dword ptr [edi - 0x70]
005B:    D9 C2                   fld      st(2)
005D:    D8 4F 8C                fmul     dword ptr [edi - 0x74]
0060:    DE C1                   faddp    st(1)
0062:    D9 C3                   fld      st(3)
0064:    D8 4F 88                fmul     dword ptr [edi - 0x78]
0067:    DE C1                   faddp    st(1)
0069:    D9 C1                   fld      st(1)
006B:    D8 4F A8                fmul     dword ptr [edi - 0x58]
006E:    D9 C3                   fld      st(3)
0070:    D8 4F A4                fmul     dword ptr [edi - 0x5c]
0073:    DE C1                   faddp    st(1)
0075:    D9 C4                   fld      st(4)
0077:    D8 4F A0                fmul     dword ptr [edi - 0x60]
007A:    DE C1                   faddp    st(1)
007C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007E:    D8 4F 9C                fmul     dword ptr [edi - 0x64]
0081:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0083:    D8 4F 98                fmul     dword ptr [edi - 0x68]
0086:    DE C3                   faddp    st(3)
0088:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008A:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
008D:    DE C2                   faddp    st(2)
008F:    DD 1C 24                fstp     qword ptr [esp]
0092:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0096:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
009A:    DD 44 24 10             fld      qword ptr [esp + 0x10]
009E:    DC 27                   fsub     qword ptr [edi]
00A0:    D9 E1                   fabs    ; |ST(0)|
00A2:    DC 27                   fsub     qword ptr [edi]
00A4:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00A7:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00AB:    D9 E8                   fld1    ; Load 1.0
00AD:    DE E9                   fsubp    st(1)
00AF:    DD 87 60 FF FF FF       fld      qword ptr [edi - 0xa0]
00B5:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00B7:    D9 E1                   fabs    ; |ST(0)|
00B9:    D9 F1                   fyl2x
00BB:    D9 E8                   fld1    ; Load 1.0
00BD:    D9 C1                   fld      st(1)
00BF:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
00C1:    D9 F0                   f2xm1
00C3:    DE C1                   faddp    st(1)
00C5:    D9 FD                   fscale
00C7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00C9:    DD D8                   fstp     st(0)
00CB:    D9 E0                   fchs    ; -ST(0)
00CD:    DD 54 24 10             fst      qword ptr [esp + 0x10]
00D1:    DD 87 68 FF FF FF       fld      qword ptr [edi - 0x98]
00D7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D9:    D9 E1                   fabs    ; |ST(0)|
00DB:    D9 F1                   fyl2x
00DD:    D9 E8                   fld1    ; Load 1.0
00DF:    D9 C1                   fld      st(1)
00E1:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
00E3:    D9 F0                   f2xm1
00E5:    DE C1                   faddp    st(1)
00E7:    D9 FD                   fscale
00E9:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00EB:    DD D8                   fstp     st(0)
00ED:    DC 4F 80                fmul     qword ptr [edi - 0x80]
00F0:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00F2:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00F6:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00FA:    DD 04 24                fld      qword ptr [esp]
00FD:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0101:    DD 44 24 08             fld      qword ptr [esp + 8]
0105:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
0109:    D9 E0                   fchs    ; -ST(0)
010B:    DE C1                   faddp    st(1)
010D:    D8 C8                   fmul     st(0)
010F:    DD 04 24                fld      qword ptr [esp]
0112:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
0116:    DD 44 24 08             fld      qword ptr [esp + 8]
011A:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
011E:    DE C1                   faddp    st(1)
0120:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0124:    DC 8F 78 FF FF FF       fmul     qword ptr [edi - 0x88]
012A:    DE C1                   faddp    st(1)
012C:    D8 C8                   fmul     st(0)
012E:    DE C1                   faddp    st(1)
0130:    D9 FA                   fsqrt    ; sqrt(ST(0))
0132:    DD 47 80                fld      qword ptr [edi - 0x80]
0135:    D9 E1                   fabs    ; |ST(0)|
0137:    D9 E8                   fld1    ; Load 1.0
0139:    D8 D9                   fcomp    st(1)
013B:    9B                      wait
013C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
013E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0140:    72 04                   jb       0x146    ; Conditional jump
0142:    DD D8                   fstp     st(0)
0144:    EB 04                   jmp      0x14a
0146:    D9 FA                   fsqrt    ; sqrt(ST(0))
0148:    DE F9                   fdivp    st(1)
014A:    DD 44 24 10             fld      qword ptr [esp + 0x10]
014E:    DC 8F 70 FF FF FF       fmul     qword ptr [edi - 0x90]
0154:    DE C1                   faddp    st(1)
0156:    DD 46 E0                fld      qword ptr [esi - 0x20]
0159:    D8 D1                   fcom     st(1)
015B:    9B                      wait
015C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
015E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0160:    73 04                   jae      0x166    ; Conditional jump
0162:    DD D8                   fstp     st(0)
0164:    EB 02                   jmp      0x168
0166:    DD D9                   fstp     st(1)
0168:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
016C:    75 03                   jne      0x171    ; Conditional jump
016E:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0171:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0174:    DD 47 F0                fld      qword ptr [edi - 0x10]
0177:    DB 46 E8                fild     dword ptr [esi - 0x18]
017A:    DC 4F B4                fmul     qword ptr [edi - 0x4c]
017D:    DE C1                   faddp    st(1)
017F:    D9 E1                   fabs    ; |ST(0)|
0181:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0187:    83 C4 34                add      esp, 0x34
018A:    C3                      ret
