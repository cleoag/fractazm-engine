; Disassembly: Beth322.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 126 bytes, decoded: 126 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    57                      push     edi
0004:    56                      push     esi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 01                   fld      qword ptr [ecx]
000D:    D8 C8                   fmul     st(0)
000F:    D9 C0                   fld      st(0)
0011:    D8 C8                   fmul     st(0)
0013:    DD 02                   fld      qword ptr [edx]
0015:    D8 C8                   fmul     st(0)
0017:    D9 C0                   fld      st(0)
0019:    D8 C8                   fmul     st(0)
001B:    DD 00                   fld      qword ptr [eax]
001D:    D8 C8                   fmul     st(0)
001F:    D9 C0                   fld      st(0)
0021:    D8 C8                   fmul     st(0)
0023:    D9 C5                   fld      st(5)
0025:    D8 C4                   fadd     st(4)
0027:    D8 CA                   fmul     st(2)
0029:    D8 C0                   fadd     st(0)
002B:    D9 C5                   fld      st(5)
002D:    D8 C4                   fadd     st(4)
002F:    DE E9                   fsubp    st(1)
0031:    DC 0F                   fmul     qword ptr [edi]
0033:    D9 C1                   fld      st(1)
0035:    D8 C0                   fadd     st(0)
0037:    DE E1                   fsubrp   st(1)
0039:    DC 08                   fmul     qword ptr [eax]
003B:    DC 46 18                fadd     qword ptr [esi + 0x18]
003E:    DD 18                   fstp     qword ptr [eax]
0040:    D9 C5                   fld      st(5)
0042:    D8 C2                   fadd     st(2)
0044:    D8 CC                   fmul     st(4)
0046:    D8 C0                   fadd     st(0)
0048:    D9 C5                   fld      st(5)
004A:    D8 C2                   fadd     st(2)
004C:    DE E9                   fsubp    st(1)
004E:    DC 0F                   fmul     qword ptr [edi]
0050:    D9 C3                   fld      st(3)
0052:    D8 C0                   fadd     st(0)
0054:    DE E1                   fsubrp   st(1)
0056:    DC 0A                   fmul     qword ptr [edx]
0058:    DC 46 20                fadd     qword ptr [esi + 0x20]
005B:    DD 1A                   fstp     qword ptr [edx]
005D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
005F:    DE C1                   faddp    st(1)
0061:    D8 C0                   fadd     st(0)
0063:    DE CC                   fmulp    st(4)
0065:    DE C1                   faddp    st(1)
0067:    DE EA                   fsubp    st(2)
0069:    D8 C0                   fadd     st(0)
006B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006D:    DC 0F                   fmul     qword ptr [edi]
006F:    DE E9                   fsubp    st(1)
0071:    DC 09                   fmul     qword ptr [ecx]
0073:    DC 46 28                fadd     qword ptr [esi + 0x28]
0076:    DD 19                   fstp     qword ptr [ecx]
0078:    5E                      pop      esi
0079:    5F                      pop      edi
007A:    5D                      pop      ebp
007B:    C2 08 00                ret      8
