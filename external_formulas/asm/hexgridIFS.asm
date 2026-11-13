; Disassembly: hexgridIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 475 bytes, decoded: 475 bytes (100.0%)
; DEoption: 20

0000:    83 C4 CE                add      esp, -0x32
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
004B:    83 C7 D4                add      edi, -0x2c
004E:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0052:    DD 44 24 08             fld      qword ptr [esp + 8]
0056:    DD 04 24                fld      qword ptr [esp]
0059:    D9 C0                   fld      st(0)
005B:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
005E:    D9 C2                   fld      st(2)
0060:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0063:    DE C1                   faddp    st(1)
0065:    D9 C3                   fld      st(3)
0067:    D8 4F BC                fmul     dword ptr [edi - 0x44]
006A:    DE C1                   faddp    st(1)
006C:    D9 C1                   fld      st(1)
006E:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0071:    D9 C3                   fld      st(3)
0073:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0076:    DE C1                   faddp    st(1)
0078:    D9 C4                   fld      st(4)
007A:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
007D:    DE C1                   faddp    st(1)
007F:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0081:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0084:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0086:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0089:    DE C3                   faddp    st(3)
008B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008D:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0090:    DE C2                   faddp    st(2)
0092:    DD 1C 24                fstp     qword ptr [esp]
0095:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0099:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
009D:    83 C7 2C                add      edi, 0x2c
00A0:    DD 04 24                fld      qword ptr [esp]
00A3:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00A6:    D9 E1                   fabs    ; |ST(0)|
00A8:    DD 1C 24                fstp     qword ptr [esp]
00AB:    DD 04 24                fld      qword ptr [esp]
00AE:    DC 77 B8                fdiv     qword ptr [edi - 0x48]
00B1:    DC 0F                   fmul     qword ptr [edi]
00B3:    DC 27                   fsub     qword ptr [edi]
00B5:    D9 FC                   frndint    ; round(ST(0))
00B7:    D8 C0                   fadd     st(0)
00B9:    D9 E8                   fld1    ; Load 1.0
00BB:    DE C1                   faddp    st(1)
00BD:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00C0:    D9 E0                   fchs    ; -ST(0)
00C2:    DC 04 24                fadd     qword ptr [esp]
00C5:    D9 E1                   fabs    ; |ST(0)|
00C7:    DD 1C 24                fstp     qword ptr [esp]
00CA:    DD 47 B8                fld      qword ptr [edi - 0x48]
00CD:    DC 4F 08                fmul     qword ptr [edi + 8]
00D0:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00D4:    DD 44 24 08             fld      qword ptr [esp + 8]
00D8:    DC 64 24 18             fsub     qword ptr [esp + 0x18]
00DC:    D9 E1                   fabs    ; |ST(0)|
00DE:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00E2:    DD 44 24 08             fld      qword ptr [esp + 8]
00E6:    DC 74 24 18             fdiv     qword ptr [esp + 0x18]
00EA:    DC 0F                   fmul     qword ptr [edi]
00EC:    DC 27                   fsub     qword ptr [edi]
00EE:    D9 FC                   frndint    ; round(ST(0))
00F0:    D8 C0                   fadd     st(0)
00F2:    D9 E8                   fld1    ; Load 1.0
00F4:    DE C1                   faddp    st(1)
00F6:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00FA:    D9 E0                   fchs    ; -ST(0)
00FC:    DC 44 24 08             fadd     qword ptr [esp + 8]
0100:    D9 E1                   fabs    ; |ST(0)|
0102:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0106:    DD 04 24                fld      qword ptr [esp]
0109:    DC 4F 10                fmul     qword ptr [edi + 0x10]
010C:    DD 44 24 08             fld      qword ptr [esp + 8]
0110:    DC 4F 18                fmul     qword ptr [edi + 0x18]
0113:    DE C1                   faddp    st(1)
0115:    DC 47 20                fadd     qword ptr [edi + 0x20]
0118:    D9 C0                   fld      st(0)
011A:    D9 E1                   fabs    ; |ST(0)|
011C:    D9 E0                   fchs    ; -ST(0)
011E:    DE C1                   faddp    st(1)
0120:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0124:    D9 C0                   fld      st(0)
0126:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0129:    D9 E0                   fchs    ; -ST(0)
012B:    DC 04 24                fadd     qword ptr [esp]
012E:    D9 E1                   fabs    ; |ST(0)|
0130:    DD 1C 24                fstp     qword ptr [esp]
0133:    DC 4F 18                fmul     qword ptr [edi + 0x18]
0136:    D9 E0                   fchs    ; -ST(0)
0138:    DC 44 24 08             fadd     qword ptr [esp + 8]
013C:    D9 E1                   fabs    ; |ST(0)|
013E:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0142:    DD 04 24                fld      qword ptr [esp]
0145:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0148:    DD 44 24 08             fld      qword ptr [esp + 8]
014C:    DC 4F 18                fmul     qword ptr [edi + 0x18]
014F:    DE C1                   faddp    st(1)
0151:    DC 47 20                fadd     qword ptr [edi + 0x20]
0154:    D9 C0                   fld      st(0)
0156:    D9 E1                   fabs    ; |ST(0)|
0158:    D9 E0                   fchs    ; -ST(0)
015A:    DE C1                   faddp    st(1)
015C:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0160:    D9 C0                   fld      st(0)
0162:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0165:    D9 E0                   fchs    ; -ST(0)
0167:    DC 04 24                fadd     qword ptr [esp]
016A:    90                      nop
016B:    90                      nop
016C:    DD 1C 24                fstp     qword ptr [esp]
016F:    DC 4F 18                fmul     qword ptr [edi + 0x18]
0172:    D9 E0                   fchs    ; -ST(0)
0174:    DC 44 24 08             fadd     qword ptr [esp + 8]
0178:    D9 E1                   fabs    ; |ST(0)|
017A:    DD 5C 24 08             fstp     qword ptr [esp + 8]
017E:    DD 04 24                fld      qword ptr [esp]
0181:    D8 4F 8C                fmul     dword ptr [edi - 0x74]
0184:    DD 44 24 08             fld      qword ptr [esp + 8]
0188:    D8 4F 88                fmul     dword ptr [edi - 0x78]
018B:    DE C1                   faddp    st(1)
018D:    DC 47 80                fadd     qword ptr [edi - 0x80]
0190:    D9 E1                   fabs    ; |ST(0)|
0192:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0196:    D9 E1                   fabs    ; |ST(0)|
0198:    DC A7 78 FF FF FF       fsub     qword ptr [edi - 0x88]
019E:    D8 D1                   fcom     st(1)
01A0:    9B                      wait
01A1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01A3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01A5:    76 04                   jbe      0x1ab    ; Conditional jump
01A7:    DD D9                   fstp     st(1)
01A9:    EB 02                   jmp      0x1ad
01AB:    DD D8                   fstp     st(0)
01AD:    90                      nop
01AE:    90                      nop
01AF:    90                      nop
01B0:    90                      nop
01B1:    90                      nop
01B2:    90                      nop
01B3:    90                      nop
01B4:    90                      nop
01B5:    90                      nop
01B6:    90                      nop
01B7:    90                      nop
01B8:    90                      nop
01B9:    90                      nop
01BA:    90                      nop
01BB:    90                      nop
01BC:    90                      nop
01BD:    90                      nop
01BE:    90                      nop
01BF:    90                      nop
01C0:    90                      nop
01C1:    90                      nop
01C2:    90                      nop
01C3:    90                      nop
01C4:    90                      nop
01C5:    90                      nop
01C6:    90                      nop
01C7:    90                      nop
01C8:    DC 67 F0                fsub     qword ptr [edi - 0x10]
01CB:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
01CF:    75 03                   jne      0x1d4    ; Conditional jump
01D1:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
01D4:    DD 5E E0                fstp     qword ptr [esi - 0x20]
01D7:    83 C4 32                add      esp, 0x32
01DA:    C3                      ret
