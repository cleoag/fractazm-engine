; Disassembly: KaliDucks.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 85 bytes, decoded: 85 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 01                   fld      qword ptr [ecx]
000D:    D9 E1                   fabs    ; |ST(0)|
000F:    DD 02                   fld      qword ptr [edx]
0011:    D9 E1                   fabs    ; |ST(0)|
0013:    DD 00                   fld      qword ptr [eax]
0015:    D9 E1                   fabs    ; |ST(0)|
0017:    DD 46 38                fld      qword ptr [esi + 0x38]
001A:    DD 47 E8                fld      qword ptr [edi - 0x18]
001D:    D8 D1                   fcom     st(1)
001F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0021:    80 E4 41                and      ah, 0x41
0024:    74 07                   je       0x2d    ; Conditional jump
0026:    DD D8                   fstp     st(0)
0028:    DC 7F F0                fdivr    qword ptr [edi - 0x10]
002B:    EB 0A                   jmp      0x37
002D:    D8 C8                   fmul     st(0)
002F:    D9 E8                   fld1    ; Load 1.0
0031:    DE F1                   fdivrp   st(1)
0033:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0035:    DD D8                   fstp     st(0)
0037:    DC CA                   fmul     st(2), st(0)
0039:    DC CB                   fmul     st(3), st(0)
003B:    DE C9                   fmulp    st(1)
003D:    8D 42 F8                lea      eax, [edx - 8]
0040:    DC 46 18                fadd     qword ptr [esi + 0x18]
0043:    DD 18                   fstp     qword ptr [eax]
0045:    DC 46 20                fadd     qword ptr [esi + 0x20]
0048:    DD 1A                   fstp     qword ptr [edx]
004A:    DC 46 28                fadd     qword ptr [esi + 0x28]
004D:    DD 19                   fstp     qword ptr [ecx]
004F:    5F                      pop      edi
0050:    5E                      pop      esi
0051:    5D                      pop      ebp
0052:    C2 08 00                ret      8
