; Disassembly: _HopSqrtY.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 58 bytes, decoded: 58 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B D8                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 02                   fld      qword ptr [edx]
000F:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0012:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0015:    D9 C0                   fld      st(0)
0017:    D9 E1                   fabs    ; |ST(0)|
0019:    D9 FA                   fsqrt    ; sqrt(ST(0))
001B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
001D:    D9 EE                   fldz    ; Load 0.0
001F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0021:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0023:    80 E4 41                and      ah, 0x41
0026:    7F 02                   jg       0x2a    ; Conditional jump
0028:    D9 E0                   fchs    ; -ST(0)
002A:    DC 76 D8                fdiv     qword ptr [esi - 0x28]
002D:    DC 46 E8                fadd     qword ptr [esi - 0x18]
0030:    DD 1A                   fstp     qword ptr [edx]
0032:    8B C3                   mov      eax, ebx
0034:    5B                      pop      ebx
0035:    5E                      pop      esi
0036:    5D                      pop      ebp
0037:    C2 08 00                ret      8
