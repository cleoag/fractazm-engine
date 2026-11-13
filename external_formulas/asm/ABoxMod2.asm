; Disassembly: ABoxMod2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 202 bytes, decoded: 202 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 47 D8                fld      qword ptr [edi - 0x28]
0011:    DD 03                   fld      qword ptr [ebx]
0013:    D9 C0                   fld      st(0)
0015:    D8 E2                   fsub     st(2)
0017:    D9 E1                   fabs    ; |ST(0)|
0019:    D8 C1                   fadd     st(1)
001B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
001D:    D8 C2                   fadd     st(2)
001F:    D9 E1                   fabs    ; |ST(0)|
0021:    DE E1                   fsubrp   st(1)
0023:    DD 02                   fld      qword ptr [edx]
0025:    D9 C0                   fld      st(0)
0027:    D8 E3                   fsub     st(3)
0029:    D9 E1                   fabs    ; |ST(0)|
002B:    D8 C1                   fadd     st(1)
002D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002F:    D8 C3                   fadd     st(3)
0031:    D9 E1                   fabs    ; |ST(0)|
0033:    DE E1                   fsubrp   st(1)
0035:    DD 47 D0                fld      qword ptr [edi - 0x30]
0038:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
003A:    DD D8                   fstp     st(0)
003C:    DD 01                   fld      qword ptr [ecx]
003E:    D9 C0                   fld      st(0)
0040:    D8 E4                   fsub     st(4)
0042:    D9 E1                   fabs    ; |ST(0)|
0044:    D8 C1                   fadd     st(1)
0046:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0048:    D8 C4                   fadd     st(4)
004A:    D9 E1                   fabs    ; |ST(0)|
004C:    DE E1                   fsubrp   st(1)
004E:    D9 C0                   fld      st(0)
0050:    D9 E1                   fabs    ; |ST(0)|
0052:    DC 57 C8                fcom     qword ptr [edi - 0x38]
0055:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0057:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0059:    76 15                   jbe      0x70    ; Conditional jump
005B:    DC 67 C8                fsub     qword ptr [edi - 0x38]
005E:    D8 C8                   fmul     st(0)
0060:    D9 C2                   fld      st(2)
0062:    D8 CB                   fmul     st(3)
0064:    DE C1                   faddp    st(1)
0066:    D9 C3                   fld      st(3)
0068:    D8 CC                   fmul     st(4)
006A:    DE C1                   faddp    st(1)
006C:    EB 0E                   jmp      0x7c
006E:    90                      nop
006F:    90                      nop
0070:    DD D8                   fstp     st(0)
0072:    D9 C1                   fld      st(1)
0074:    D8 CA                   fmul     st(2)
0076:    D9 C3                   fld      st(3)
0078:    D8 CC                   fmul     st(4)
007A:    DE C1                   faddp    st(1)
007C:    DC 57 E0                fcom     qword ptr [edi - 0x20]
007F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0081:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0083:    73 07                   jae      0x8c    ; Conditional jump
0085:    DD D8                   fstp     st(0)
0087:    DD 47 E8                fld      qword ptr [edi - 0x18]
008A:    EB 16                   jmp      0xa2
008C:    D9 E8                   fld1    ; Load 1.0
008E:    D8 D1                   fcom     st(1)
0090:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0092:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0094:    72 07                   jb       0x9d    ; Conditional jump
0096:    DD D8                   fstp     st(0)
0098:    DC 7F F0                fdivr    qword ptr [edi - 0x10]
009B:    EB 05                   jmp      0xa2
009D:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
009F:    DD 47 F0                fld      qword ptr [edi - 0x10]
00A2:    DD 41 08                fld      qword ptr [ecx + 8]
00A5:    D8 C9                   fmul     st(1)
00A7:    DD 59 08                fstp     qword ptr [ecx + 8]
00AA:    DC CB                   fmul     st(3), st(0)
00AC:    DC CA                   fmul     st(2), st(0)
00AE:    DE C9                   fmulp    st(1)
00B0:    DC 46 28                fadd     qword ptr [esi + 0x28]
00B3:    DD 19                   fstp     qword ptr [ecx]
00B5:    DC 46 20                fadd     qword ptr [esi + 0x20]
00B8:    DD 1A                   fstp     qword ptr [edx]
00BA:    DC 46 18                fadd     qword ptr [esi + 0x18]
00BD:    DD 1B                   fstp     qword ptr [ebx]
00BF:    DD D8                   fstp     st(0)
00C1:    89 D8                   mov      eax, ebx
00C3:    5B                      pop      ebx
00C4:    5F                      pop      edi
00C5:    5E                      pop      esi
00C6:    5D                      pop      ebp
00C7:    C2 08 00                ret      8
