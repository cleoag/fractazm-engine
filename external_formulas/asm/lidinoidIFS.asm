; Disassembly: lidinoidIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 390 bytes, decoded: 390 bytes (100.0%)
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
00A1:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00A5:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00A8:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00AA:    DD 44 24 08             fld      qword ptr [esp + 8]
00AE:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00B1:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00B3:    DD 04 24                fld      qword ptr [esp]
00B6:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00B9:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00BB:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00BF:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00C3:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
00C7:    DD 5C 24 30             fstp     qword ptr [esp + 0x30]
00CB:    DD 5C 24 38             fstp     qword ptr [esp + 0x38]
00CF:    DD 5C 24 40             fstp     qword ptr [esp + 0x40]
00D3:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00D7:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00D9:    DD 44 24 08             fld      qword ptr [esp + 8]
00DD:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00DF:    DD 04 24                fld      qword ptr [esp]
00E2:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00E4:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
00E8:    DE CB                   fmulp    st(3)
00EA:    DC 4C 24 40             fmul     qword ptr [esp + 0x40]
00EE:    DE CB                   fmulp    st(3)
00F0:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
00F4:    DE CB                   fmulp    st(3)
00F6:    D9 C0                   fld      st(0)
00F8:    D8 E2                   fsub     st(2)
00FA:    D8 E3                   fsub     st(3)
00FC:    DC 4F 88                fmul     qword ptr [edi - 0x78]
00FF:    D9 E1                   fabs    ; |ST(0)|
0101:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0107:    DE C1                   faddp    st(1)
0109:    DE C1                   faddp    st(1)
010B:    DD 44 24 18             fld      qword ptr [esp + 0x18]
010F:    DC 4C 24 28             fmul     qword ptr [esp + 0x28]
0113:    DD 44 24 28             fld      qword ptr [esp + 0x28]
0117:    DC 4C 24 38             fmul     qword ptr [esp + 0x38]
011B:    DD 44 24 38             fld      qword ptr [esp + 0x38]
011F:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0123:    DE C1                   faddp    st(1)
0125:    DE C1                   faddp    st(1)
0127:    D9 E0                   fchs    ; -ST(0)
0129:    DE C1                   faddp    st(1)
012B:    DC 0F                   fmul     qword ptr [edi]
012D:    DC 47 08                fadd     qword ptr [edi + 8]
0130:    90                      nop
0131:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0135:    D9 E1                   fabs    ; |ST(0)|
0137:    DD 44 24 08             fld      qword ptr [esp + 8]
013B:    D9 E1                   fabs    ; |ST(0)|
013D:    D8 D1                   fcom     st(1)
013F:    9B                      wait
0140:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0142:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0144:    73 04                   jae      0x14a    ; Conditional jump
0146:    DD D8                   fstp     st(0)
0148:    EB 02                   jmp      0x14c
014A:    DD D9                   fstp     st(1)
014C:    DD 04 24                fld      qword ptr [esp]
014F:    90                      nop
0150:    D9 E1                   fabs    ; |ST(0)|
0152:    D8 D1                   fcom     st(1)
0154:    9B                      wait
0155:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0157:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0159:    73 04                   jae      0x15f    ; Conditional jump
015B:    DD D8                   fstp     st(0)
015D:    EB 02                   jmp      0x161
015F:    DD D9                   fstp     st(1)
0161:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0164:    D8 D1                   fcom     st(1)
0166:    9B                      wait
0167:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0169:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
016B:    73 04                   jae      0x171    ; Conditional jump
016D:    DD D8                   fstp     st(0)
016F:    EB 02                   jmp      0x173
0171:    DD D9                   fstp     st(1)
0173:    DC 77 B8                fdiv     qword ptr [edi - 0x48]
0176:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
017A:    75 03                   jne      0x17f    ; Conditional jump
017C:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
017F:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0182:    83 C4 50                add      esp, 0x50
0185:    C3                      ret
