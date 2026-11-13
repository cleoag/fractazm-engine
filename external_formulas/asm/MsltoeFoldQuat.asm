; Disassembly: MsltoeFoldQuat.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 118 bytes, decoded: 118 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    89 C7                   mov      edi, eax
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DD 02                   fld      qword ptr [edx]
000E:    DD 00                   fld      qword ptr [eax]
0010:    D9 C2                   fld      st(2)
0012:    D8 CB                   fmul     st(3)
0014:    D9 C2                   fld      st(2)
0016:    D8 CB                   fmul     st(3)
0018:    D9 C2                   fld      st(2)
001A:    D8 CB                   fmul     st(3)
001C:    D8 D2                   fcom     st(2)
001E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0020:    80 E4 41                and      ah, 0x41
0023:    74 08                   je       0x2d    ; Conditional jump
0025:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0027:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0029:    D9 CD                   fxch     st(5)    ; Exchange ST(0) and st(5)
002B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
002D:    D8 D1                   fcom     st(1)
002F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0031:    80 E4 41                and      ah, 0x41
0034:    74 08                   je       0x3e    ; Conditional jump
0036:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0038:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
003A:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
003C:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
003E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0040:    D8 D2                   fcom     st(2)
0042:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0044:    80 E4 41                and      ah, 0x41
0047:    74 08                   je       0x51    ; Conditional jump
0049:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
004B:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
004D:    D9 CD                   fxch     st(5)    ; Exchange ST(0) and st(5)
004F:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
0051:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0053:    DE E1                   fsubrp   st(1)
0055:    DE E1                   fsubrp   st(1)
0057:    DC 46 18                fadd     qword ptr [esi + 0x18]
005A:    DD 1F                   fstp     qword ptr [edi]
005C:    DC C9                   fmul     st(1), st(0)
005E:    DE CA                   fmulp    st(2)
0060:    D8 C0                   fadd     st(0)
0062:    DC 46 20                fadd     qword ptr [esi + 0x20]
0065:    DD 1A                   fstp     qword ptr [edx]
0067:    D8 C0                   fadd     st(0)
0069:    DC 46 28                fadd     qword ptr [esi + 0x28]
006C:    DD 19                   fstp     qword ptr [ecx]
006E:    89 F8                   mov      eax, edi
0070:    5F                      pop      edi
0071:    5E                      pop      esi
0072:    5D                      pop      ebp
0073:    C2 08 00                ret      8
