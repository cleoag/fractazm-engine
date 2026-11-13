; Disassembly: _FoldingInfy.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 296 bytes, decoded: 296 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    50                      push     eax
0005:    56                      push     esi
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B D8                   mov      ebx, eax
000B:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000E:    DD 46 D8                fld      qword ptr [esi - 0x28]
0011:    D9 E4                   ftst
0013:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0015:    D9 D0                   fnop
0017:    9E                      sahf
0018:    DD D8                   fstp     st(0)
001A:    74 36                   je       0x52    ; Conditional jump
001C:    DD 46 D8                fld      qword ptr [esi - 0x28]
001F:    D8 C0                   fadd     st(0)
0021:    D8 C0                   fadd     st(0)
0023:    DD 41 08                fld      qword ptr [ecx + 8]
0026:    D9 C0                   fld      st(0)
0028:    D8 F2                   fdiv     st(2)
002A:    90                      nop
002B:    D9 FC                   frndint    ; round(ST(0))
002D:    D8 CA                   fmul     st(2)
002F:    90                      nop
0030:    DE E9                   fsubp    st(1)
0032:    DD D9                   fstp     st(1)
0034:    D9 E0                   fchs    ; -ST(0)
0036:    D9 C0                   fld      st(0)
0038:    D9 E0                   fchs    ; -ST(0)
003A:    DC 46 D8                fadd     qword ptr [esi - 0x28]
003D:    D9 E1                   fabs    ; |ST(0)|
003F:    D9 C1                   fld      st(1)
0041:    D9 E0                   fchs    ; -ST(0)
0043:    DC 66 D8                fsub     qword ptr [esi - 0x28]
0046:    D9 E1                   fabs    ; |ST(0)|
0048:    D9 E0                   fchs    ; -ST(0)
004A:    DE C1                   faddp    st(1)
004C:    DE C1                   faddp    st(1)
004E:    DD 59 08                fstp     qword ptr [ecx + 8]
0051:    90                      nop
0052:    DD 46 E0                fld      qword ptr [esi - 0x20]
0055:    D9 E4                   ftst
0057:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0059:    D9 D0                   fnop
005B:    9E                      sahf
005C:    DD D8                   fstp     st(0)
005E:    74 36                   je       0x96    ; Conditional jump
0060:    DD 46 E0                fld      qword ptr [esi - 0x20]
0063:    D8 C0                   fadd     st(0)
0065:    D8 C0                   fadd     st(0)
0067:    DD 01                   fld      qword ptr [ecx]
0069:    90                      nop
006A:    D9 C0                   fld      st(0)
006C:    D8 F2                   fdiv     st(2)
006E:    90                      nop
006F:    D9 FC                   frndint    ; round(ST(0))
0071:    D8 CA                   fmul     st(2)
0073:    90                      nop
0074:    DE E9                   fsubp    st(1)
0076:    DD D9                   fstp     st(1)
0078:    D9 E0                   fchs    ; -ST(0)
007A:    D9 C0                   fld      st(0)
007C:    D9 E0                   fchs    ; -ST(0)
007E:    DC 46 E0                fadd     qword ptr [esi - 0x20]
0081:    D9 E1                   fabs    ; |ST(0)|
0083:    D9 C1                   fld      st(1)
0085:    D9 E0                   fchs    ; -ST(0)
0087:    DC 66 E0                fsub     qword ptr [esi - 0x20]
008A:    D9 E1                   fabs    ; |ST(0)|
008C:    D9 E0                   fchs    ; -ST(0)
008E:    DE C1                   faddp    st(1)
0090:    DE C1                   faddp    st(1)
0092:    DD 19                   fstp     qword ptr [ecx]
0094:    90                      nop
0095:    90                      nop
0096:    DD 46 E8                fld      qword ptr [esi - 0x18]
0099:    D9 E4                   ftst
009B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009D:    D9 D0                   fnop
009F:    9E                      sahf
00A0:    DD D8                   fstp     st(0)
00A2:    74 36                   je       0xda    ; Conditional jump
00A4:    DD 46 E8                fld      qword ptr [esi - 0x18]
00A7:    D8 C0                   fadd     st(0)
00A9:    D8 C0                   fadd     st(0)
00AB:    DD 02                   fld      qword ptr [edx]
00AD:    90                      nop
00AE:    D9 C0                   fld      st(0)
00B0:    D8 F2                   fdiv     st(2)
00B2:    90                      nop
00B3:    D9 FC                   frndint    ; round(ST(0))
00B5:    D8 CA                   fmul     st(2)
00B7:    90                      nop
00B8:    DE E9                   fsubp    st(1)
00BA:    DD D9                   fstp     st(1)
00BC:    D9 E0                   fchs    ; -ST(0)
00BE:    D9 C0                   fld      st(0)
00C0:    D9 E0                   fchs    ; -ST(0)
00C2:    DC 46 E8                fadd     qword ptr [esi - 0x18]
00C5:    D9 E1                   fabs    ; |ST(0)|
00C7:    D9 C1                   fld      st(1)
00C9:    D9 E0                   fchs    ; -ST(0)
00CB:    DC 66 E8                fsub     qword ptr [esi - 0x18]
00CE:    D9 E1                   fabs    ; |ST(0)|
00D0:    D9 E0                   fchs    ; -ST(0)
00D2:    DE C1                   faddp    st(1)
00D4:    DE C1                   faddp    st(1)
00D6:    DD 1A                   fstp     qword ptr [edx]
00D8:    90                      nop
00D9:    90                      nop
00DA:    DD 46 F0                fld      qword ptr [esi - 0x10]
00DD:    D9 E4                   ftst
00DF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E1:    D9 D0                   fnop
00E3:    9E                      sahf
00E4:    DD D8                   fstp     st(0)
00E6:    74 36                   je       0x11e    ; Conditional jump
00E8:    DD 46 F0                fld      qword ptr [esi - 0x10]
00EB:    D8 C0                   fadd     st(0)
00ED:    D8 C0                   fadd     st(0)
00EF:    DD 03                   fld      qword ptr [ebx]
00F1:    90                      nop
00F2:    D9 C0                   fld      st(0)
00F4:    D8 F2                   fdiv     st(2)
00F6:    90                      nop
00F7:    D9 FC                   frndint    ; round(ST(0))
00F9:    D8 CA                   fmul     st(2)
00FB:    90                      nop
00FC:    DE E9                   fsubp    st(1)
00FE:    DD D9                   fstp     st(1)
0100:    D9 E0                   fchs    ; -ST(0)
0102:    D9 C0                   fld      st(0)
0104:    D9 E0                   fchs    ; -ST(0)
0106:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0109:    D9 E1                   fabs    ; |ST(0)|
010B:    D9 C1                   fld      st(1)
010D:    D9 E0                   fchs    ; -ST(0)
010F:    DC 66 F0                fsub     qword ptr [esi - 0x10]
0112:    D9 E1                   fabs    ; |ST(0)|
0114:    D9 E0                   fchs    ; -ST(0)
0116:    DE C1                   faddp    st(1)
0118:    DE C1                   faddp    st(1)
011A:    DD 1B                   fstp     qword ptr [ebx]
011C:    90                      nop
011D:    90                      nop
011E:    90                      nop
011F:    8B C3                   mov      eax, ebx
0121:    5E                      pop      esi
0122:    58                      pop      eax
0123:    5B                      pop      ebx
0124:    5D                      pop      ebp
0125:    C2 08 00                ret      8
