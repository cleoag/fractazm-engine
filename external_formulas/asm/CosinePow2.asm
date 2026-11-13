; Disassembly: CosinePow2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 77 bytes, decoded: 77 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    DD 02                   fld      qword ptr [edx]
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000D:    DC 0A                   fmul     qword ptr [edx]
000F:    DD 00                   fld      qword ptr [eax]
0011:    DC 08                   fmul     qword ptr [eax]
0013:    D9 C0                   fld      st(0)
0015:    D8 C2                   fadd     st(2)
0017:    DD 01                   fld      qword ptr [ecx]
0019:    D9 C0                   fld      st(0)
001B:    D8 C8                   fmul     st(0)
001D:    D8 E2                   fsub     st(2)
001F:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0022:    DC 46 28                fadd     qword ptr [esi + 0x28]
0025:    DD 19                   fstp     qword ptr [ecx]
0027:    D8 C0                   fadd     st(0)
0029:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002B:    D9 FA                   fsqrt    ; sqrt(ST(0))
002D:    DE F9                   fdivp    st(1)
002F:    D9 C0                   fld      st(0)
0031:    DC 08                   fmul     qword ptr [eax]
0033:    DC 0A                   fmul     qword ptr [edx]
0035:    D8 C0                   fadd     st(0)
0037:    DC 46 20                fadd     qword ptr [esi + 0x20]
003A:    DD 1A                   fstp     qword ptr [edx]
003C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
003E:    DE E2                   fsubrp   st(2)
0040:    DE C9                   fmulp    st(1)
0042:    DC 46 18                fadd     qword ptr [esi + 0x18]
0045:    DD 18                   fstp     qword ptr [eax]
0047:    5F                      pop      edi
0048:    5E                      pop      esi
0049:    5D                      pop      ebp
004A:    C2 08 00                ret      8
