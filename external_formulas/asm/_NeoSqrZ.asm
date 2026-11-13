; Disassembly: _NeoSqrZ.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 74 bytes, decoded: 74 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B D8                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 01                   fld      qword ptr [ecx]
000F:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0012:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0015:    D9 C0                   fld      st(0)
0017:    D9 EE                   fldz    ; Load 0.0
0019:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
001B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
001D:    80 E4 41                and      ah, 0x41
0020:    7F 0D                   jg       0x2f    ; Conditional jump
0022:    D9 C0                   fld      st(0)
0024:    D9 E0                   fchs    ; -ST(0)
0026:    DD 46 E8                fld      qword ptr [esi - 0x18]
0029:    DE C1                   faddp    st(1)
002B:    DE C9                   fmulp    st(1)
002D:    EB 0B                   jmp      0x3a
002F:    D9 C0                   fld      st(0)
0031:    DD 46 E8                fld      qword ptr [esi - 0x18]
0034:    D9 E0                   fchs    ; -ST(0)
0036:    DE C1                   faddp    st(1)
0038:    DE C9                   fmulp    st(1)
003A:    DC 76 D8                fdiv     qword ptr [esi - 0x28]
003D:    DC 66 F0                fsub     qword ptr [esi - 0x10]
0040:    DD 19                   fstp     qword ptr [ecx]
0042:    8B C3                   mov      eax, ebx
0044:    5B                      pop      ebx
0045:    5E                      pop      esi
0046:    5D                      pop      ebp
0047:    C2 08 00                ret      8
