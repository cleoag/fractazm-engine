; Disassembly: Kalisets1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 72 bytes, decoded: 72 bytes (100.0%)

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
0017:    D9 C0                   fld      st(0)
0019:    D8 C8                   fmul     st(0)
001B:    D9 C2                   fld      st(2)
001D:    D8 C8                   fmul     st(0)
001F:    DE C1                   faddp    st(1)
0021:    D9 C3                   fld      st(3)
0023:    D8 C8                   fmul     st(0)
0025:    DE C1                   faddp    st(1)
0027:    DC 47 E8                fadd     qword ptr [edi - 0x18]
002A:    DC 7F F0                fdivr    qword ptr [edi - 0x10]
002D:    DC CA                   fmul     st(2), st(0)
002F:    DC CB                   fmul     st(3), st(0)
0031:    DE C9                   fmulp    st(1)
0033:    DC 46 18                fadd     qword ptr [esi + 0x18]
0036:    DD 18                   fstp     qword ptr [eax]
0038:    DC 46 20                fadd     qword ptr [esi + 0x20]
003B:    DD 1A                   fstp     qword ptr [edx]
003D:    DC 46 28                fadd     qword ptr [esi + 0x28]
0040:    DD 19                   fstp     qword ptr [ecx]
0042:    5F                      pop      edi
0043:    5E                      pop      esi
0044:    5D                      pop      ebp
0045:    C2 08 00                ret      8
