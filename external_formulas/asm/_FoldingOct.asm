; Disassembly: _FoldingOct.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 71 bytes, decoded: 71 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    89 C7                   mov      edi, eax
000A:    DD 01                   fld      qword ptr [ecx]
000C:    D9 E1                   fabs    ; |ST(0)|
000E:    DD 02                   fld      qword ptr [edx]
0010:    D9 E1                   fabs    ; |ST(0)|
0012:    DD 07                   fld      qword ptr [edi]
0014:    D9 E1                   fabs    ; |ST(0)|
0016:    D8 D1                   fcom     st(1)
0018:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
001A:    80 E4 41                and      ah, 0x41
001D:    74 02                   je       0x21    ; Conditional jump
001F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0021:    D8 D2                   fcom     st(2)
0023:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0025:    80 E4 41                and      ah, 0x41
0028:    74 02                   je       0x2c    ; Conditional jump
002A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
002C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002E:    D8 D2                   fcom     st(2)
0030:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0032:    80 E4 41                and      ah, 0x41
0035:    74 02                   je       0x39    ; Conditional jump
0037:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0039:    DD 1A                   fstp     qword ptr [edx]
003B:    DD 1F                   fstp     qword ptr [edi]
003D:    DD 19                   fstp     qword ptr [ecx]
003F:    89 F8                   mov      eax, edi
0041:    5F                      pop      edi
0042:    5E                      pop      esi
0043:    5D                      pop      ebp
0044:    C2 08 00                ret      8
