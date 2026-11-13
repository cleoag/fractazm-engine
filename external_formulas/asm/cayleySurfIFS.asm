; Disassembly: cayleySurfIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 616 bytes, decoded: 616 bytes (100.0%)
; DEoption: 20

0000:    83 C4 BA                add      esp, -0x46
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
004D:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0051:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0055:    83 C7 D4                add      edi, -0x2c
0058:    DD 44 24 10             fld      qword ptr [esp + 0x10]
005C:    DD 44 24 08             fld      qword ptr [esp + 8]
0060:    DD 04 24                fld      qword ptr [esp]
0063:    D9 C0                   fld      st(0)
0065:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0068:    D9 C2                   fld      st(2)
006A:    D8 4F C0                fmul     dword ptr [edi - 0x40]
006D:    DE C1                   faddp    st(1)
006F:    D9 C3                   fld      st(3)
0071:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0074:    DE C1                   faddp    st(1)
0076:    D9 C1                   fld      st(1)
0078:    D8 4F DC                fmul     dword ptr [edi - 0x24]
007B:    D9 C3                   fld      st(3)
007D:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0080:    DE C1                   faddp    st(1)
0082:    D9 C4                   fld      st(4)
0084:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0087:    DE C1                   faddp    st(1)
0089:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
008B:    D8 4F D0                fmul     dword ptr [edi - 0x30]
008E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0090:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0093:    DE C3                   faddp    st(3)
0095:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0097:    D8 4F C8                fmul     dword ptr [edi - 0x38]
009A:    DE C2                   faddp    st(2)
009C:    DD 1C 24                fstp     qword ptr [esp]
009F:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00A3:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A7:    83 C7 2C                add      edi, 0x2c
00AA:    DD 04 24                fld      qword ptr [esp]
00AD:    DC 0C 24                fmul     qword ptr [esp]
00B0:    DC 4C 24 08             fmul     qword ptr [esp + 8]
00B4:    D9 E4                   ftst
00B6:    9B                      wait
00B7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B9:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00BB:    73 0A                   jae      0xc7    ; Conditional jump
00BD:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
00C1:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00C5:    EB 08                   jmp      0xcf
00C7:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
00CB:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00CF:    DD 04 24                fld      qword ptr [esp]
00D2:    DC 0C 24                fmul     qword ptr [esp]
00D5:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
00D9:    D9 E4                   ftst
00DB:    9B                      wait
00DC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DE:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00E0:    73 0A                   jae      0xec    ; Conditional jump
00E2:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
00E6:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00EA:    EB 08                   jmp      0xf4
00EC:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
00F0:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00F4:    DD 44 24 08             fld      qword ptr [esp + 8]
00F8:    DC 4C 24 08             fmul     qword ptr [esp + 8]
00FC:    DC 0C 24                fmul     qword ptr [esp]
00FF:    D9 E4                   ftst
0101:    9B                      wait
0102:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0104:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0106:    73 0A                   jae      0x112    ; Conditional jump
0108:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
010C:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0110:    EB 08                   jmp      0x11a
0112:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
0116:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
011A:    DD 44 24 08             fld      qword ptr [esp + 8]
011E:    DC 4C 24 08             fmul     qword ptr [esp + 8]
0122:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
0126:    D9 E4                   ftst
0128:    9B                      wait
0129:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
012B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
012D:    73 0A                   jae      0x139    ; Conditional jump
012F:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
0133:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0137:    EB 08                   jmp      0x141
0139:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
013D:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0141:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0145:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
0149:    DC 4C 24 08             fmul     qword ptr [esp + 8]
014D:    D9 E4                   ftst
014F:    9B                      wait
0150:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0152:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0154:    73 0A                   jae      0x160    ; Conditional jump
0156:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
015A:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
015E:    EB 08                   jmp      0x168
0160:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
0164:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0168:    DD 44 24 10             fld      qword ptr [esp + 0x10]
016C:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
0170:    DC 0C 24                fmul     qword ptr [esp]
0173:    D9 E4                   ftst
0175:    9B                      wait
0176:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0178:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
017A:    73 0A                   jae      0x186    ; Conditional jump
017C:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
0180:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0184:    EB 08                   jmp      0x18e
0186:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
018A:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
018E:    DD 44 24 18             fld      qword ptr [esp + 0x18]
0192:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0195:    D9 E1                   fabs    ; |ST(0)|
0197:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
019B:    DD 44 24 20             fld      qword ptr [esp + 0x20]
019F:    DC 4F B8                fmul     qword ptr [edi - 0x48]
01A2:    D9 E1                   fabs    ; |ST(0)|
01A4:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
01A8:    D9 D0                   fnop
01AA:    DD 04 24                fld      qword ptr [esp]
01AD:    DC 4C 24 08             fmul     qword ptr [esp + 8]
01B1:    D8 C0                   fadd     st(0)
01B3:    D9 E4                   ftst
01B5:    9B                      wait
01B6:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01B8:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01BA:    73 0A                   jae      0x1c6    ; Conditional jump
01BC:    DC 6C 24 18             fsubr    qword ptr [esp + 0x18]
01C0:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
01C4:    EB 08                   jmp      0x1ce
01C6:    DC 44 24 20             fadd     qword ptr [esp + 0x20]
01CA:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
01CE:    DD 04 24                fld      qword ptr [esp]
01D1:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
01D5:    D8 C0                   fadd     st(0)
01D7:    D9 E4                   ftst
01D9:    9B                      wait
01DA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01DC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01DE:    73 0A                   jae      0x1ea    ; Conditional jump
01E0:    DC 6C 24 18             fsubr    qword ptr [esp + 0x18]
01E4:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
01E8:    EB 08                   jmp      0x1f2
01EA:    DC 44 24 20             fadd     qword ptr [esp + 0x20]
01EE:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
01F2:    DD 44 24 08             fld      qword ptr [esp + 8]
01F6:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
01FA:    D8 C0                   fadd     st(0)
01FC:    D9 E4                   ftst
01FE:    9B                      wait
01FF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0201:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0203:    73 0A                   jae      0x20f    ; Conditional jump
0205:    DC 6C 24 18             fsubr    qword ptr [esp + 0x18]
0209:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
020D:    EB 08                   jmp      0x217
020F:    DC 44 24 20             fadd     qword ptr [esp + 0x20]
0213:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0217:    D9 D0                   fnop
0219:    DD 44 24 18             fld      qword ptr [esp + 0x18]
021D:    D9 FA                   fsqrt    ; sqrt(ST(0))
021F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0221:    DD 44 24 20             fld      qword ptr [esp + 0x20]
0225:    D9 FA                   fsqrt    ; sqrt(ST(0))
0227:    D9 FA                   fsqrt    ; sqrt(ST(0))
0229:    D9 E0                   fchs    ; -ST(0)
022B:    DE C1                   faddp    st(1)
022D:    DC 0F                   fmul     qword ptr [edi]
022F:    DD 04 24                fld      qword ptr [esp]
0232:    D8 C8                   fmul     st(0)
0234:    DD 44 24 08             fld      qword ptr [esp + 8]
0238:    D8 C8                   fmul     st(0)
023A:    DD 44 24 10             fld      qword ptr [esp + 0x10]
023E:    D8 C8                   fmul     st(0)
0240:    DE C1                   faddp    st(1)
0242:    DE C1                   faddp    st(1)
0244:    D9 FA                   fsqrt    ; sqrt(ST(0))
0246:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0249:    D8 D1                   fcom     st(1)
024B:    9B                      wait
024C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
024E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0250:    73 04                   jae      0x256    ; Conditional jump
0252:    DD D8                   fstp     st(0)
0254:    EB 02                   jmp      0x258
0256:    DD D9                   fstp     st(1)
0258:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
025C:    75 03                   jne      0x261    ; Conditional jump
025E:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0261:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0264:    83 C4 46                add      esp, 0x46
0267:    C3                      ret
