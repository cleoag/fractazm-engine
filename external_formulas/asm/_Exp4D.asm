; Disassembly: _Exp4D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 120 bytes, decoded: 120 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B D8                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 41 08                fld      qword ptr [ecx + 8]
0010:    D8 C8                   fmul     st(0)
0012:    DD 02                   fld      qword ptr [edx]
0014:    D8 C8                   fmul     st(0)
0016:    DE C1                   faddp    st(1)
0018:    DD 01                   fld      qword ptr [ecx]
001A:    D8 C8                   fmul     st(0)
001C:    DE C1                   faddp    st(1)
001E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0020:    D9 C0                   fld      st(0)
0022:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0024:    D9 D0                   fnop
0026:    DD 03                   fld      qword ptr [ebx]
0028:    D9 EA                   fldl2e    ; Load log2(e)
002A:    DE C9                   fmulp    st(1)
002C:    D9 E8                   fld1    ; Load 1.0
002E:    D9 C1                   fld      st(1)
0030:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
0032:    D9 F0                   f2xm1
0034:    DE C1                   faddp    st(1)
0036:    D9 FD                   fscale
0038:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
003A:    DD D8                   fstp     st(0)
003C:    D9 C0                   fld      st(0)
003E:    D8 CA                   fmul     st(2)
0040:    DD 1B                   fstp     qword ptr [ebx]
0042:    D8 CA                   fmul     st(2)
0044:    D9 C3                   fld      st(3)
0046:    DD 06                   fld      qword ptr [esi]
0048:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
004A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004C:    80 E4 41                and      ah, 0x41
004F:    74 02                   je       0x53    ; Conditional jump
0051:    D8 F3                   fdiv     st(3)
0053:    DD 02                   fld      qword ptr [edx]
0055:    D8 C9                   fmul     st(1)
0057:    DD 1A                   fstp     qword ptr [edx]
0059:    DD 01                   fld      qword ptr [ecx]
005B:    D8 C9                   fmul     st(1)
005D:    DD 19                   fstp     qword ptr [ecx]
005F:    DD 41 08                fld      qword ptr [ecx + 8]
0062:    D8 C9                   fmul     st(1)
0064:    DD 59 08                fstp     qword ptr [ecx + 8]
0067:    DD D8                   fstp     st(0)
0069:    DD D8                   fstp     st(0)
006B:    DD D8                   fstp     st(0)
006D:    DD D8                   fstp     st(0)
006F:    90                      nop
0070:    8B C3                   mov      eax, ebx
0072:    5B                      pop      ebx
0073:    5E                      pop      esi
0074:    5D                      pop      ebp
0075:    C2 08 00                ret      8
