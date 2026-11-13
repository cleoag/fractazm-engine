; Disassembly: hextgridIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 483 bytes, decoded: 483 bytes (100.0%)
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
004D:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0051:    83 C7 D4                add      edi, -0x2c
0054:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0058:    DD 44 24 08             fld      qword ptr [esp + 8]
005C:    DD 04 24                fld      qword ptr [esp]
005F:    D9 C0                   fld      st(0)
0061:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0064:    D9 C2                   fld      st(2)
0066:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0069:    DE C1                   faddp    st(1)
006B:    D9 C3                   fld      st(3)
006D:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0070:    DE C1                   faddp    st(1)
0072:    D9 C1                   fld      st(1)
0074:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0077:    D9 C3                   fld      st(3)
0079:    D8 4F D8                fmul     dword ptr [edi - 0x28]
007C:    DE C1                   faddp    st(1)
007E:    D9 C4                   fld      st(4)
0080:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0083:    DE C1                   faddp    st(1)
0085:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0087:    D8 4F D0                fmul     dword ptr [edi - 0x30]
008A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008C:    D8 4F CC                fmul     dword ptr [edi - 0x34]
008F:    DE C3                   faddp    st(3)
0091:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0093:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0096:    DE C2                   faddp    st(2)
0098:    DD 1C 24                fstp     qword ptr [esp]
009B:    DD 5C 24 08             fstp     qword ptr [esp + 8]
009F:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A3:    83 C7 2C                add      edi, 0x2c
00A6:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00AA:    D8 C8                   fmul     st(0)
00AC:    DC 0F                   fmul     qword ptr [edi]
00AE:    DC 47 F0                fadd     qword ptr [edi - 0x10]
00B1:    D9 E1                   fabs    ; |ST(0)|
00B3:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00B7:    DD 44 24 08             fld      qword ptr [esp + 8]
00BB:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00BE:    D9 FF                   fcos    ; cos(ST(0))
00C0:    90                      nop
00C1:    D9 E4                   ftst
00C3:    9B                      wait
00C4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00C8:    73 0A                   jae      0xd4    ; Conditional jump
00CA:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
00CE:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00D2:    EB 08                   jmp      0xdc
00D4:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
00D8:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00DC:    DD 04 24                fld      qword ptr [esp]
00DF:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00E2:    DD 44 24 08             fld      qword ptr [esp + 8]
00E6:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00E9:    DE E9                   fsubp    st(1)
00EB:    D9 FF                   fcos    ; cos(ST(0))
00ED:    90                      nop
00EE:    D9 E4                   ftst
00F0:    9B                      wait
00F1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00F5:    73 0A                   jae      0x101    ; Conditional jump
00F7:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
00FB:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
00FF:    EB 08                   jmp      0x109
0101:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
0105:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0109:    DD 04 24                fld      qword ptr [esp]
010C:    DC 4F 20                fmul     qword ptr [edi + 0x20]
010F:    DD 44 24 08             fld      qword ptr [esp + 8]
0113:    DC 4F 18                fmul     qword ptr [edi + 0x18]
0116:    DE C1                   faddp    st(1)
0118:    D9 FF                   fcos    ; cos(ST(0))
011A:    90                      nop
011B:    D9 E4                   ftst
011D:    9B                      wait
011E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0120:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0122:    73 0A                   jae      0x12e    ; Conditional jump
0124:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
0128:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
012C:    EB 08                   jmp      0x136
012E:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
0132:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0136:    DD 04 24                fld      qword ptr [esp]
0139:    DC 4F 28                fmul     qword ptr [edi + 0x28]
013C:    D9 FF                   fcos    ; cos(ST(0))
013E:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0141:    90                      nop
0142:    D9 E4                   ftst
0144:    9B                      wait
0145:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0147:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0149:    73 0A                   jae      0x155    ; Conditional jump
014B:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
014F:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0153:    EB 08                   jmp      0x15d
0155:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
0159:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
015D:    DD 04 24                fld      qword ptr [esp]
0160:    DC 4F 20                fmul     qword ptr [edi + 0x20]
0163:    DD 44 24 08             fld      qword ptr [esp + 8]
0167:    DC 4F 08                fmul     qword ptr [edi + 8]
016A:    DE E1                   fsubrp   st(1)
016C:    D9 FF                   fcos    ; cos(ST(0))
016E:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0171:    90                      nop
0172:    D9 E4                   ftst
0174:    9B                      wait
0175:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0177:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0179:    73 0A                   jae      0x185    ; Conditional jump
017B:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
017F:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0183:    EB 08                   jmp      0x18d
0185:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
0189:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
018D:    DD 04 24                fld      qword ptr [esp]
0190:    DC 4F 20                fmul     qword ptr [edi + 0x20]
0193:    DD 44 24 08             fld      qword ptr [esp + 8]
0197:    DC 4F 08                fmul     qword ptr [edi + 8]
019A:    DE C1                   faddp    st(1)
019C:    D9 FF                   fcos    ; cos(ST(0))
019E:    DC 4F B8                fmul     qword ptr [edi - 0x48]
01A1:    90                      nop
01A2:    D9 E4                   ftst
01A4:    9B                      wait
01A5:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01A7:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01A9:    73 0A                   jae      0x1b5    ; Conditional jump
01AB:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
01AF:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
01B3:    EB 08                   jmp      0x1bd
01B5:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
01B9:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
01BD:    DD 44 24 18             fld      qword ptr [esp + 0x18]
01C1:    D9 FA                   fsqrt    ; sqrt(ST(0))
01C3:    90                      nop
01C4:    90                      nop
01C5:    DD 44 24 20             fld      qword ptr [esp + 0x20]
01C9:    D9 FA                   fsqrt    ; sqrt(ST(0))
01CB:    90                      nop
01CC:    90                      nop
01CD:    D9 E0                   fchs    ; -ST(0)
01CF:    DE C1                   faddp    st(1)
01D1:    90                      nop
01D2:    90                      nop
01D3:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
01D7:    75 03                   jne      0x1dc    ; Conditional jump
01D9:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
01DC:    DD 5E E0                fstp     qword ptr [esi - 0x20]
01DF:    83 C4 46                add      esp, 0x46
01E2:    C3                      ret
