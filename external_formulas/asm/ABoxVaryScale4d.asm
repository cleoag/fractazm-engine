; Disassembly: ABoxVaryScale4d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 295 bytes, decoded: 295 bytes (100.0%)
; DEoption: 5

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
000E:    81 C7 80 00 00 00       add      edi, 0x80
0014:    83 7F 50 00             cmp      dword ptr [edi + 0x50], 0
0018:    7F 09                   jg       0x23    ; Conditional jump
001A:    FF 47 50                inc      dword ptr [edi + 0x50]
001D:    DD 46 F0                fld      qword ptr [esi - 0x10]
0020:    DD 5F 48                fstp     qword ptr [edi + 0x48]
0023:    DD 47 48                fld      qword ptr [edi + 0x48]
0026:    D9 E8                   fld1    ; Load 1.0
0028:    D9 C1                   fld      st(1)
002A:    D9 E1                   fabs    ; |ST(0)|
002C:    DE E1                   fsubrp   st(1)
002E:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0031:    DE C1                   faddp    st(1)
0033:    DD 5F 48                fstp     qword ptr [edi + 0x48]
0036:    DD 46 D8                fld      qword ptr [esi - 0x28]
0039:    DD 03                   fld      qword ptr [ebx]
003B:    D9 C0                   fld      st(0)
003D:    D8 E2                   fsub     st(2)
003F:    D9 E1                   fabs    ; |ST(0)|
0041:    D8 C1                   fadd     st(1)
0043:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0045:    D8 C2                   fadd     st(2)
0047:    D9 E1                   fabs    ; |ST(0)|
0049:    DE E1                   fsubrp   st(1)
004B:    DD 02                   fld      qword ptr [edx]
004D:    D9 C0                   fld      st(0)
004F:    D8 E3                   fsub     st(3)
0051:    D9 E1                   fabs    ; |ST(0)|
0053:    D8 C1                   fadd     st(1)
0055:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0057:    D8 C3                   fadd     st(3)
0059:    D9 E1                   fabs    ; |ST(0)|
005B:    DE E1                   fsubrp   st(1)
005D:    DD 01                   fld      qword ptr [ecx]
005F:    D9 C0                   fld      st(0)
0061:    D8 E4                   fsub     st(4)
0063:    D9 E1                   fabs    ; |ST(0)|
0065:    D8 C1                   fadd     st(1)
0067:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0069:    D8 C4                   fadd     st(4)
006B:    D9 E1                   fabs    ; |ST(0)|
006D:    DE E1                   fsubrp   st(1)
006F:    DD 41 08                fld      qword ptr [ecx + 8]
0072:    D9 C0                   fld      st(0)
0074:    D8 E5                   fsub     st(5)
0076:    D9 E1                   fabs    ; |ST(0)|
0078:    D8 C1                   fadd     st(1)
007A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
007C:    D8 C5                   fadd     st(5)
007E:    D9 E1                   fabs    ; |ST(0)|
0080:    DE E1                   fsubrp   st(1)
0082:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
0084:    DD D8                   fstp     st(0)
0086:    D9 C0                   fld      st(0)
0088:    D8 C9                   fmul     st(1)
008A:    D9 C2                   fld      st(2)
008C:    D8 CB                   fmul     st(3)
008E:    DE C1                   faddp    st(1)
0090:    D9 C3                   fld      st(3)
0092:    D8 CC                   fmul     st(4)
0094:    DE C1                   faddp    st(1)
0096:    D9 C4                   fld      st(4)
0098:    D8 CD                   fmul     st(5)
009A:    DE C1                   faddp    st(1)
009C:    81 7E CC 00 00 80 3F    cmp      dword ptr [esi - 0x34], 0x3f800000
00A3:    74 25                   je       0xca    ; Conditional jump
00A5:    D9 46 CC                fld      dword ptr [esi - 0x34]
00A8:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00AA:    D9 ED                   fldln2    ; Load ln(2)
00AC:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00AE:    D9 F1                   fyl2x
00B0:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00B2:    DE C9                   fmulp    st(1)
00B4:    D9 EA                   fldl2e    ; Load log2(e)
00B6:    DE C9                   fmulp    st(1)
00B8:    D9 C0                   fld      st(0)
00BA:    D9 FC                   frndint    ; round(ST(0))
00BC:    DC E9                   fsub     st(1), st(0)
00BE:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00C0:    D9 F0                   f2xm1
00C2:    D9 E8                   fld1    ; Load 1.0
00C4:    DE C1                   faddp    st(1)
00C6:    D9 FD                   fscale
00C8:    DD D9                   fstp     st(1)
00CA:    DC 56 E0                fcom     qword ptr [esi - 0x20]
00CD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CF:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D1:    73 0A                   jae      0xdd    ; Conditional jump
00D3:    DD D8                   fstp     st(0)
00D5:    DD 47 48                fld      qword ptr [edi + 0x48]
00D8:    DC 76 E0                fdiv     qword ptr [esi - 0x20]
00DB:    EB 16                   jmp      0xf3
00DD:    D9 E8                   fld1    ; Load 1.0
00DF:    D8 D1                   fcom     st(1)
00E1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00E5:    72 07                   jb       0xee    ; Conditional jump
00E7:    DD D8                   fstp     st(0)
00E9:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
00EC:    EB 05                   jmp      0xf3
00EE:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00F0:    DD 47 48                fld      qword ptr [edi + 0x48]
00F3:    DD 47 68                fld      qword ptr [edi + 0x68]
00F6:    D8 C9                   fmul     st(1)
00F8:    DD 5F 68                fstp     qword ptr [edi + 0x68]
00FB:    DC CC                   fmul     st(4), st(0)
00FD:    DC CB                   fmul     st(3), st(0)
00FF:    DC CA                   fmul     st(2), st(0)
0101:    DE C9                   fmulp    st(1)
0103:    DC 47 A8                fadd     qword ptr [edi - 0x58]
0106:    DD 19                   fstp     qword ptr [ecx]
0108:    DC 47 A0                fadd     qword ptr [edi - 0x60]
010B:    DD 1A                   fstp     qword ptr [edx]
010D:    DC 47 98                fadd     qword ptr [edi - 0x68]
0110:    DD 1B                   fstp     qword ptr [ebx]
0112:    DC 46 C4                fadd     qword ptr [esi - 0x3c]
0115:    DC 87 48 FF FF FF       fadd     qword ptr [edi - 0xb8]
011B:    DD 59 08                fstp     qword ptr [ecx + 8]
011E:    89 D8                   mov      eax, ebx
0120:    5B                      pop      ebx
0121:    5F                      pop      edi
0122:    5E                      pop      esi
0123:    5D                      pop      ebp
0124:    C2 08 00                ret      8
