; Disassembly: schwartzIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 367 bytes, decoded: 367 bytes (100.0%)
; DEoption: 20

0000:    83 C4 C4                add      esp, -0x3c
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
00A2:    DD 04 24                fld      qword ptr [esp]
00A5:    D8 C8                   fmul     st(0)
00A7:    DD 44 24 08             fld      qword ptr [esp + 8]
00AB:    D8 C8                   fmul     st(0)
00AD:    DE C1                   faddp    st(1)
00AF:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00B3:    D8 C8                   fmul     st(0)
00B5:    DE C1                   faddp    st(1)
00B7:    DD 47 F0                fld      qword ptr [edi - 0x10]
00BA:    D9 E1                   fabs    ; |ST(0)|
00BC:    DE C1                   faddp    st(1)
00BE:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00C2:    D9 EE                   fldz    ; Load 0.0
00C4:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00C8:    90                      nop
00C9:    DD 04 24                fld      qword ptr [esp]
00CC:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00CF:    D9 FF                   fcos    ; cos(ST(0))
00D1:    D9 E4                   ftst
00D3:    9B                      wait
00D4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00D6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D8:    73 0A                   jae      0xe4    ; Conditional jump
00DA:    DC 6C 24 18             fsubr    qword ptr [esp + 0x18]
00DE:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00E2:    EB 08                   jmp      0xec
00E4:    DC 44 24 20             fadd     qword ptr [esp + 0x20]
00E8:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00EC:    DD 44 24 08             fld      qword ptr [esp + 8]
00F0:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00F3:    D9 FF                   fcos    ; cos(ST(0))
00F5:    D9 E4                   ftst
00F7:    9B                      wait
00F8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00FA:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00FC:    73 0A                   jae      0x108    ; Conditional jump
00FE:    DC 6C 24 18             fsubr    qword ptr [esp + 0x18]
0102:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0106:    EB 08                   jmp      0x110
0108:    DC 44 24 20             fadd     qword ptr [esp + 0x20]
010C:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0110:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0114:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0117:    D9 FF                   fcos    ; cos(ST(0))
0119:    D9 E4                   ftst
011B:    9B                      wait
011C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
011E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0120:    73 0A                   jae      0x12c    ; Conditional jump
0122:    DC 6C 24 18             fsubr    qword ptr [esp + 0x18]
0126:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
012A:    EB 08                   jmp      0x134
012C:    DC 44 24 20             fadd     qword ptr [esp + 0x20]
0130:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0134:    90                      nop
0135:    DD 44 24 20             fld      qword ptr [esp + 0x20]
0139:    D9 FA                   fsqrt    ; sqrt(ST(0))
013B:    DD 44 24 18             fld      qword ptr [esp + 0x18]
013F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0141:    DE E1                   fsubrp   st(1)
0143:    DD 47 B8                fld      qword ptr [edi - 0x48]
0146:    D9 E1                   fabs    ; |ST(0)|
0148:    D9 E8                   fld1    ; Load 1.0
014A:    DE E9                   fsubp    st(1)
014C:    D9 C0                   fld      st(0)
014E:    D9 E1                   fabs    ; |ST(0)|
0150:    DE C1                   faddp    st(1)
0152:    D9 E8                   fld1    ; Load 1.0
0154:    DE C1                   faddp    st(1)
0156:    DC 0F                   fmul     qword ptr [edi]
0158:    90                      nop
0159:    90                      nop
015A:    D9 FA                   fsqrt    ; sqrt(ST(0))
015C:    DE F9                   fdivp    st(1)
015E:    90                      nop
015F:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0163:    75 03                   jne      0x168    ; Conditional jump
0165:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0168:    DD 5E E0                fstp     qword ptr [esi - 0x20]
016B:    83 C4 3C                add      esp, 0x3c
016E:    C3                      ret
