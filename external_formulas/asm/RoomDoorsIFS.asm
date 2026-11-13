; Disassembly: RoomDoorsIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 635 bytes, decoded: 635 bytes (100.0%)
; DEoption: 20

0000:    51                      push     ecx
0001:    83 C4 C4                add      esp, -0x3c
0004:    66 0F 10 46 90          movupd   xmm0, xmmword ptr [esi - 0x70]
0009:    F2 0F 10 4E 88          movsd    xmm1, qword ptr [esi - 0x78]
000E:    66 0F 59 47 D0          mulpd    xmm0, xmmword ptr [edi - 0x30]
0013:    F2 0F 59 4F D0          mulsd    xmm1, qword ptr [edi - 0x30]
0018:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
001C:    7C 0F                   jl       0x2d    ; Conditional jump
001E:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
0023:    F2 0F 59 6F D0          mulsd    xmm5, qword ptr [edi - 0x30]
0028:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
002D:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0032:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
0038:    DD 44 24 10             fld      qword ptr [esp + 0x10]
003C:    DD 04 24                fld      qword ptr [esp]
003F:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0041:    DC 6F 18                fsubr    qword ptr [edi + 0x18]
0044:    DC 4F 08                fmul     qword ptr [edi + 8]
0047:    DA 4F B4                fimul    dword ptr [edi - 0x4c]
004A:    DB 5C 24 28             fistp    dword ptr [esp + 0x28]
004E:    DB 44 24 28             fild     dword ptr [esp + 0x28]
0052:    DA 77 B4                fidiv    dword ptr [edi - 0x4c]
0055:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0058:    D9 FB                   fsincos    ; ST(0)=cos, push sin
005A:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
005E:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0062:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0066:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
006A:    DD 04 24                fld      qword ptr [esp]
006D:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
0071:    D9 E0                   fchs    ; -ST(0)
0073:    DE C1                   faddp    st(1)
0075:    DC 47 B8                fadd     qword ptr [edi - 0x48]
0078:    DD 44 24 10             fld      qword ptr [esp + 0x10]
007C:    DC 4C 24 20             fmul     qword ptr [esp + 0x20]
0080:    DD 04 24                fld      qword ptr [esp]
0083:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
0087:    DE C1                   faddp    st(1)
0089:    DC 47 C0                fadd     qword ptr [edi - 0x40]
008C:    DD 1C 24                fstp     qword ptr [esp]
008F:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0093:    DD 44 24 08             fld      qword ptr [esp + 8]
0097:    DC 47 C8                fadd     qword ptr [edi - 0x38]
009A:    DD 5C 24 08             fstp     qword ptr [esp + 8]
009E:    66 0F 10 04 24          movupd   xmm0, xmmword ptr [esp]
00A3:    F2 0F 10 4C 24 10       movsd    xmm1, qword ptr [esp + 0x10]
00A9:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00AD:    7C 0A                   jl       0xb9    ; Conditional jump
00AF:    66 0F 11 46 90          movupd   xmmword ptr [esi - 0x70], xmm0
00B4:    F2 0F 11 4E 88          movsd    qword ptr [esi - 0x78], xmm1
00B9:    DD 44 24 08             fld      qword ptr [esp + 8]
00BD:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
00C0:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00C4:    DC 4F 9C                fmul     qword ptr [edi - 0x64]
00C7:    DE C1                   faddp    st(1)
00C9:    DD 44 24 08             fld      qword ptr [esp + 8]
00CD:    DC 4F 9C                fmul     qword ptr [edi - 0x64]
00D0:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00D4:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
00D7:    D9 E0                   fchs    ; -ST(0)
00D9:    DE C1                   faddp    st(1)
00DB:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00DF:    DD 54 24 20             fst      qword ptr [esp + 0x20]
00E3:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00E7:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00EB:    D9 E1                   fabs    ; |ST(0)|
00ED:    DD 47 F0                fld      qword ptr [edi - 0x10]
00F0:    D9 E1                   fabs    ; |ST(0)|
00F2:    DE E9                   fsubp    st(1)
00F4:    D9 E0                   fchs    ; -ST(0)
00F6:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
00FB:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
0101:    8B 44 24 28             mov      eax, dword ptr [esp + 0x28]
0105:    05 00 01 00 00          add      eax, 0x100
010A:    31 D2                   xor      edx, edx
010C:    8B 4F 88                mov      ecx, dword ptr [edi - 0x78]
010F:    83 F9 00                cmp      ecx, 0
0112:    7E 0B                   jle      0x11f    ; Conditional jump
0114:    F7 F1                   div      ecx
0116:    83 FA 01                cmp      edx, 1
0119:    0F 85 93 00 00 00       jne      0x1b2    ; Conditional jump
011F:    DD 04 24                fld      qword ptr [esp]
0122:    D9 E1                   fabs    ; |ST(0)|
0124:    DC 4F 8C                fmul     qword ptr [edi - 0x74]
0127:    DD 44 24 08             fld      qword ptr [esp + 8]
012B:    DC 4F 94                fmul     qword ptr [edi - 0x6c]
012E:    D9 E0                   fchs    ; -ST(0)
0130:    DE C1                   faddp    st(1)
0132:    DC 67 AC                fsub     qword ptr [edi - 0x54]
0135:    DD 44 24 08             fld      qword ptr [esp + 8]
0139:    DD 47 E0                fld      qword ptr [edi - 0x20]
013C:    D9 E0                   fchs    ; -ST(0)
013E:    DE E1                   fsubrp   st(1)
0140:    D9 C0                   fld      st(0)
0142:    DD 47 80                fld      qword ptr [edi - 0x80]
0145:    D9 EE                   fldz    ; Load 0.0
0147:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0149:    9B                      wait
014A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
014C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
014E:    77 0F                   ja       0x15f    ; Conditional jump
0150:    DD 47 80                fld      qword ptr [edi - 0x80]
0153:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0157:    D9 EE                   fldz    ; Load 0.0
0159:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
015D:    EB 11                   jmp      0x170
015F:    DD 47 AC                fld      qword ptr [edi - 0x54]
0162:    DC 67 80                fsub     qword ptr [edi - 0x80]
0165:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0169:    DD 47 80                fld      qword ptr [edi - 0x80]
016C:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
0170:    D8 C8                   fmul     st(0)
0172:    DD 04 24                fld      qword ptr [esp]
0175:    D9 E1                   fabs    ; |ST(0)|
0177:    DC 64 24 28             fsub     qword ptr [esp + 0x28]
017B:    D8 C8                   fmul     st(0)
017D:    DE C1                   faddp    st(1)
017F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0181:    DC 64 24 18             fsub     qword ptr [esp + 0x18]
0185:    D8 D1                   fcom     st(1)
0187:    9B                      wait
0188:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
018A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
018C:    72 04                   jb       0x192    ; Conditional jump
018E:    DD D8                   fstp     st(0)
0190:    EB 02                   jmp      0x194
0192:    DD D9                   fstp     st(1)
0194:    D8 D1                   fcom     st(1)
0196:    9B                      wait
0197:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0199:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
019B:    73 04                   jae      0x1a1    ; Conditional jump
019D:    DD D8                   fstp     st(0)
019F:    EB 02                   jmp      0x1a3
01A1:    DD D9                   fstp     st(1)
01A3:    D8 D1                   fcom     st(1)
01A5:    9B                      wait
01A6:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01A8:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01AA:    73 04                   jae      0x1b0    ; Conditional jump
01AC:    DD D9                   fstp     st(1)
01AE:    EB 02                   jmp      0x1b2
01B0:    DD D8                   fstp     st(0)
01B2:    D9 E0                   fchs    ; -ST(0)
01B4:    31 C9                   xor      ecx, ecx
01B6:    8B 57 F4                mov      edx, dword ptr [edi - 0xc]
01B9:    81 E2 00 00 00 80       and      edx, 0x80000000
01BF:    74 6B                   je       0x22c    ; Conditional jump
01C1:    DD 44 24 08             fld      qword ptr [esp + 8]
01C5:    DC 47 E8                fadd     qword ptr [edi - 0x18]
01C8:    D9 E1                   fabs    ; |ST(0)|
01CA:    DC 47 F0                fadd     qword ptr [edi - 0x10]
01CD:    DD 44 24 20             fld      qword ptr [esp + 0x20]
01D1:    DC 47 F0                fadd     qword ptr [edi - 0x10]
01D4:    D8 D1                   fcom     st(1)
01D6:    9B                      wait
01D7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01D9:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01DB:    72 04                   jb       0x1e1    ; Conditional jump
01DD:    DD D9                   fstp     st(1)
01DF:    EB 02                   jmp      0x1e3
01E1:    DD D8                   fstp     st(0)
01E3:    D9 C0                   fld      st(0)
01E5:    DD 47 20                fld      qword ptr [edi + 0x20]
01E8:    DC 8E 78 FF FF FF       fmul     qword ptr [esi - 0x88]
01EE:    DE C1                   faddp    st(1)
01F0:    D9 EE                   fldz    ; Load 0.0
01F2:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
01F4:    9B                      wait
01F5:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01F7:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01F9:    72 22                   jb       0x21d    ; Conditional jump
01FB:    DD 87 68 FF FF FF       fld      qword ptr [edi - 0x98]
0201:    D9 EE                   fldz    ; Load 0.0
0203:    D8 D9                   fcomp    st(1)
0205:    9B                      wait
0206:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0208:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
020A:    72 06                   jb       0x212    ; Conditional jump
020C:    31 C9                   xor      ecx, ecx
020E:    DD D8                   fstp     st(0)
0210:    EB 0B                   jmp      0x21d
0212:    B9 01 00 00 00          mov      ecx, 1
0217:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
021D:    D8 D1                   fcom     st(1)
021F:    9B                      wait
0220:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0222:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0224:    73 04                   jae      0x22a    ; Conditional jump
0226:    DD D9                   fstp     st(1)
0228:    EB 02                   jmp      0x22c
022A:    DD D8                   fstp     st(0)
022C:    DD 44 24 08             fld      qword ptr [esp + 8]
0230:    DD 47 E8                fld      qword ptr [edi - 0x18]
0233:    DC 0F                   fmul     qword ptr [edi]
0235:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0239:    DE C1                   faddp    st(1)
023B:    D9 E1                   fabs    ; |ST(0)|
023D:    DC 64 24 18             fsub     qword ptr [esp + 0x18]
0241:    D8 D1                   fcom     st(1)
0243:    9B                      wait
0244:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0246:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0248:    73 04                   jae      0x24e    ; Conditional jump
024A:    DD D8                   fstp     st(0)
024C:    EB 02                   jmp      0x250
024E:    DD D9                   fstp     st(1)
0250:    83 F9 00                cmp      ecx, 0
0253:    75 15                   jne      0x26a    ; Conditional jump
0255:    DB 46 E8                fild     dword ptr [esi - 0x18]
0258:    DC 8F 78 FF FF FF       fmul     qword ptr [edi - 0x88]
025E:    DC 87 70 FF FF FF       fadd     qword ptr [edi - 0x90]
0264:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
026A:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
026E:    7D 03                   jge      0x273    ; Conditional jump
0270:    DC 77 D0                fdiv     qword ptr [edi - 0x30]
0273:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0276:    83 C4 3C                add      esp, 0x3c
0279:    59                      pop      ecx
027A:    C3                      ret
