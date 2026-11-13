; Disassembly: MsltoeSym2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 110 bytes, decoded: 110 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    57                      push     edi
0004:    53                      push     ebx
0005:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
0008:    89 C3                   mov      ebx, eax
000A:    DD 00                   fld      qword ptr [eax]
000C:    DD 01                   fld      qword ptr [ecx]
000E:    DD 02                   fld      qword ptr [edx]
0010:    D9 C1                   fld      st(1)
0012:    D9 E1                   fabs    ; |ST(0)|
0014:    D9 C1                   fld      st(1)
0016:    D9 E1                   fabs    ; |ST(0)|
0018:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
001A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
001C:    80 E4 41                and      ah, 0x41
001F:    74 02                   je       0x23    ; Conditional jump
0021:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0023:    D8 D1                   fcom     st(1)
0025:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0027:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0029:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
002B:    72 02                   jb       0x2f    ; Conditional jump
002D:    D9 E0                   fchs    ; -ST(0)
002F:    D9 C1                   fld      st(1)
0031:    D8 C8                   fmul     st(0)
0033:    DC 77 38                fdiv     qword ptr [edi + 0x38]
0036:    D9 E8                   fld1    ; Load 1.0
0038:    DE E1                   fsubrp   st(1)
003A:    D9 C1                   fld      st(1)
003C:    D8 C8                   fmul     st(0)
003E:    D9 C4                   fld      st(4)
0040:    D8 C8                   fmul     st(0)
0042:    D9 C1                   fld      st(1)
0044:    D8 E1                   fsub     st(1)
0046:    D8 CB                   fmul     st(3)
0048:    DC 47 18                fadd     qword ptr [edi + 0x18]
004B:    DD 1B                   fstp     qword ptr [ebx]
004D:    DE C1                   faddp    st(1)
004F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0051:    DE CB                   fmulp    st(3)
0053:    DE C9                   fmulp    st(1)
0055:    DE CA                   fmulp    st(2)
0057:    D8 C0                   fadd     st(0)
0059:    DD 19                   fstp     qword ptr [ecx]
005B:    8B 5F 30                mov      ebx, dword ptr [edi + 0x30]
005E:    D8 C0                   fadd     st(0)
0060:    DC 4B F0                fmul     qword ptr [ebx - 0x10]
0063:    DC 47 20                fadd     qword ptr [edi + 0x20]
0066:    DD 1A                   fstp     qword ptr [edx]
0068:    5B                      pop      ebx
0069:    5F                      pop      edi
006A:    5D                      pop      ebp
006B:    C2 08 00                ret      8
