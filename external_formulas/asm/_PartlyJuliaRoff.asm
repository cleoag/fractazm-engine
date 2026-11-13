; Disassembly: _PartlyJuliaRoff.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 133 bytes, decoded: 133 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    50                      push     eax
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    DD 06                   fld      qword ptr [esi]
000E:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0011:    DD 46 08                fld      qword ptr [esi + 8]
0014:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0017:    DD 46 10                fld      qword ptr [esi + 0x10]
001A:    DC 67 E0                fsub     qword ptr [edi - 0x20]
001D:    D9 C2                   fld      st(2)
001F:    D8 C8                   fmul     st(0)
0021:    D9 C2                   fld      st(2)
0023:    D8 C8                   fmul     st(0)
0025:    DE C1                   faddp    st(1)
0027:    D9 C1                   fld      st(1)
0029:    D8 C8                   fmul     st(0)
002B:    DE C1                   faddp    st(1)
002D:    D9 FA                   fsqrt    ; sqrt(ST(0))
002F:    DC 57 D8                fcom     qword ptr [edi - 0x28]
0032:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0034:    80 E4 41                and      ah, 0x41
0037:    75 41                   jne      0x7a    ; Conditional jump
0039:    D9 C0                   fld      st(0)
003B:    DC 67 D8                fsub     qword ptr [edi - 0x28]
003E:    D9 C1                   fld      st(1)
0040:    D9 E1                   fabs    ; |ST(0)|
0042:    DC 1F                   fcomp    qword ptr [edi]
0044:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0046:    80 E4 41                and      ah, 0x41
0049:    74 13                   je       0x5e    ; Conditional jump
004B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
004D:    D9 EE                   fldz    ; Load 0.0
004F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0051:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0053:    DD 07                   fld      qword ptr [edi]
0055:    80 E4 41                and      ah, 0x41
0058:    75 02                   jne      0x5c    ; Conditional jump
005A:    D9 E0                   fchs    ; -ST(0)
005C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
005E:    DE F1                   fdivrp   st(1)
0060:    DC CB                   fmul     st(3), st(0)
0062:    DC CA                   fmul     st(2), st(0)
0064:    DE C9                   fmulp    st(1)
0066:    DC 46 28                fadd     qword ptr [esi + 0x28]
0069:    DD 5E 28                fstp     qword ptr [esi + 0x28]
006C:    DC 46 20                fadd     qword ptr [esi + 0x20]
006F:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0072:    DC 46 18                fadd     qword ptr [esi + 0x18]
0075:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0078:    EB 04                   jmp      0x7e
007A:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
007C:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
007E:    58                      pop      eax
007F:    5F                      pop      edi
0080:    5E                      pop      esi
0081:    5D                      pop      ebp
0082:    C2 08 00                ret      8
