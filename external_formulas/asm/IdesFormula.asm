; Disassembly: IdesFormula.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 83 bytes, decoded: 83 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
0008:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
000B:    DD 01                   fld      qword ptr [ecx]
000D:    DD 02                   fld      qword ptr [edx]
000F:    DD 00                   fld      qword ptr [eax]
0011:    D9 C0                   fld      st(0)
0013:    D8 CA                   fmul     st(2)
0015:    D8 CB                   fmul     st(3)
0017:    DC 4E E8                fmul     qword ptr [esi - 0x18]
001A:    DC 47 20                fadd     qword ptr [edi + 0x20]
001D:    DD 1A                   fstp     qword ptr [edx]
001F:    D8 C8                   fmul     st(0)
0021:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0023:    D8 C8                   fmul     st(0)
0025:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0027:    D8 C8                   fmul     st(0)
0029:    D9 C0                   fld      st(0)
002B:    D8 C3                   fadd     st(3)
002D:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0030:    D9 C2                   fld      st(2)
0032:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0035:    DE E1                   fsubrp   st(1)
0037:    DC 47 18                fadd     qword ptr [edi + 0x18]
003A:    DD 18                   fstp     qword ptr [eax]
003C:    DC 4E E0                fmul     qword ptr [esi - 0x20]
003F:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0041:    DE C1                   faddp    st(1)
0043:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0046:    DE E9                   fsubp    st(1)
0048:    DC 47 28                fadd     qword ptr [edi + 0x28]
004B:    DD 19                   fstp     qword ptr [ecx]
004D:    5F                      pop      edi
004E:    5E                      pop      esi
004F:    5D                      pop      ebp
0050:    C2 08 00                ret      8
