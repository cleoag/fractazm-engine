; Disassembly: Benesi3Pow2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 101 bytes, decoded: 101 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    50                      push     eax
0004:    56                      push     esi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    DD 01                   fld      qword ptr [ecx]
000A:    DD 02                   fld      qword ptr [edx]
000C:    DD 00                   fld      qword ptr [eax]
000E:    D9 C2                   fld      st(2)
0010:    D8 C8                   fmul     st(0)
0012:    D9 C2                   fld      st(2)
0014:    D8 C8                   fmul     st(0)
0016:    D9 C2                   fld      st(2)
0018:    D8 C8                   fmul     st(0)
001A:    D9 C2                   fld      st(2)
001C:    D8 C2                   fadd     st(2)
001E:    D9 C0                   fld      st(0)
0020:    D9 FA                   fsqrt    ; sqrt(ST(0))
0022:    D8 D5                   fcom     st(5)
0024:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0026:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0028:    DE E1                   fsubrp   st(1)
002A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
002C:    73 09                   jae      0x37    ; Conditional jump
002E:    8A 46 1F                mov      al, byte ptr [esi + 0x1f]
0031:    D0 E0                   shl      al, 1
0033:    72 02                   jb       0x37    ; Conditional jump
0035:    D9 E0                   fchs    ; -ST(0)
0037:    DC 46 18                fadd     qword ptr [esi + 0x18]
003A:    DD 5A F8                fstp     qword ptr [edx - 8]
003D:    D9 E8                   fld1    ; Load 1.0
003F:    D8 C0                   fadd     st(0)
0041:    D9 E0                   fchs    ; -ST(0)
0043:    DE F1                   fdivrp   st(1)
0045:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0047:    D9 E1                   fabs    ; |ST(0)|
0049:    DE CB                   fmulp    st(3)
004B:    DE E1                   fsubrp   st(1)
004D:    D8 C9                   fmul     st(1)
004F:    DC 46 20                fadd     qword ptr [esi + 0x20]
0052:    DD 1A                   fstp     qword ptr [edx]
0054:    DE C9                   fmulp    st(1)
0056:    DE C9                   fmulp    st(1)
0058:    D8 C0                   fadd     st(0)
005A:    DC 46 28                fadd     qword ptr [esi + 0x28]
005D:    DD 19                   fstp     qword ptr [ecx]
005F:    5E                      pop      esi
0060:    58                      pop      eax
0061:    5D                      pop      ebp
0062:    C2 08 00                ret      8
