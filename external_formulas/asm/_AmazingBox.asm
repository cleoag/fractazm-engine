; Disassembly: _AmazingBox.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 153 bytes, decoded: 153 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    89 C3                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    D9 E8                   fld1    ; Load 1.0
000F:    DD 03                   fld      qword ptr [ebx]
0011:    D9 C0                   fld      st(0)
0013:    D8 E2                   fsub     st(2)
0015:    D9 E1                   fabs    ; |ST(0)|
0017:    D8 C1                   fadd     st(1)
0019:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
001B:    D8 C2                   fadd     st(2)
001D:    D9 E1                   fabs    ; |ST(0)|
001F:    DE E1                   fsubrp   st(1)
0021:    DD 02                   fld      qword ptr [edx]
0023:    D9 C0                   fld      st(0)
0025:    D8 E3                   fsub     st(3)
0027:    D9 E1                   fabs    ; |ST(0)|
0029:    D8 C1                   fadd     st(1)
002B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002D:    D8 C3                   fadd     st(3)
002F:    D9 E1                   fabs    ; |ST(0)|
0031:    DE E1                   fsubrp   st(1)
0033:    DD 01                   fld      qword ptr [ecx]
0035:    D9 C0                   fld      st(0)
0037:    D8 E4                   fsub     st(4)
0039:    D9 E1                   fabs    ; |ST(0)|
003B:    D8 C1                   fadd     st(1)
003D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
003F:    D8 C4                   fadd     st(4)
0041:    D9 E1                   fabs    ; |ST(0)|
0043:    DE E1                   fsubrp   st(1)
0045:    D9 C0                   fld      st(0)
0047:    D8 C9                   fmul     st(1)
0049:    D9 C2                   fld      st(2)
004B:    D8 CB                   fmul     st(3)
004D:    DE C1                   faddp    st(1)
004F:    D9 C3                   fld      st(3)
0051:    D8 CC                   fmul     st(4)
0053:    DE C1                   faddp    st(1)
0055:    DC 56 E0                fcom     qword ptr [esi - 0x20]
0058:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005C:    73 07                   jae      0x65    ; Conditional jump
005E:    DD D8                   fstp     st(0)
0060:    DD 46 E8                fld      qword ptr [esi - 0x18]
0063:    EB 16                   jmp      0x7b
0065:    D9 E8                   fld1    ; Load 1.0
0067:    D8 D1                   fcom     st(1)
0069:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006D:    72 07                   jb       0x76    ; Conditional jump
006F:    DD D8                   fstp     st(0)
0071:    DC 7E F0                fdivr    qword ptr [esi - 0x10]
0074:    EB 05                   jmp      0x7b
0076:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0078:    DD 46 F0                fld      qword ptr [esi - 0x10]
007B:    DD 41 08                fld      qword ptr [ecx + 8]
007E:    D8 C9                   fmul     st(1)
0080:    DD 59 08                fstp     qword ptr [ecx + 8]
0083:    DC CB                   fmul     st(3), st(0)
0085:    DC CA                   fmul     st(2), st(0)
0087:    DE C9                   fmulp    st(1)
0089:    DD 19                   fstp     qword ptr [ecx]
008B:    DD 1A                   fstp     qword ptr [edx]
008D:    DD 1B                   fstp     qword ptr [ebx]
008F:    DD D8                   fstp     st(0)
0091:    89 D8                   mov      eax, ebx
0093:    5B                      pop      ebx
0094:    5E                      pop      esi
0095:    5D                      pop      ebp
0096:    C2 08 00                ret      8
