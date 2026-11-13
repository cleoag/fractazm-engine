; Disassembly: _SphereFolding2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 60 bytes, decoded: 60 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 46 38                fld      qword ptr [esi + 0x38]
000E:    DC 57 F0                fcom     qword ptr [edi - 0x10]
0011:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0013:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0015:    73 1D                   jae      0x34    ; Conditional jump
0017:    D9 FA                   fsqrt    ; sqrt(ST(0))
0019:    D9 C0                   fld      st(0)
001B:    DC 6F E8                fsubr    qword ptr [edi - 0x18]
001E:    DE F1                   fdivrp   st(1)
0020:    DD 01                   fld      qword ptr [ecx]
0022:    DD 02                   fld      qword ptr [edx]
0024:    DD 42 F8                fld      qword ptr [edx - 8]
0027:    D8 CB                   fmul     st(3)
0029:    DD 5A F8                fstp     qword ptr [edx - 8]
002C:    D8 CA                   fmul     st(2)
002E:    DD 1A                   fstp     qword ptr [edx]
0030:    DE C9                   fmulp    st(1)
0032:    DD 11                   fst      qword ptr [ecx]
0034:    DD D8                   fstp     st(0)
0036:    5F                      pop      edi
0037:    5E                      pop      esi
0038:    5D                      pop      ebp
0039:    C2 08 00                ret      8
