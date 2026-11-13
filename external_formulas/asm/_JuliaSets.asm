; Disassembly: _JuliaSets.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 89 bytes, decoded: 89 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 47 F0                fld      qword ptr [edi - 0x10]
000E:    D9 E8                   fld1    ; Load 1.0
0010:    D8 F1                   fdiv     st(1)
0012:    DD 46 08                fld      qword ptr [esi + 8]
0015:    D8 C9                   fmul     st(1)
0017:    DD 06                   fld      qword ptr [esi]
0019:    D8 CA                   fmul     st(2)
001B:    DD 46 10                fld      qword ptr [esi + 0x10]
001E:    DE CB                   fmulp    st(3)
0020:    D9 C0                   fld      st(0)
0022:    D9 FC                   frndint    ; round(ST(0))
0024:    D9 C2                   fld      st(2)
0026:    D9 FC                   frndint    ; round(ST(0))
0028:    D9 C4                   fld      st(4)
002A:    D9 FC                   frndint    ; round(ST(0))
002C:    DC ED                   fsub     st(5), st(0)
002E:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0031:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0034:    DC EB                   fsub     st(3), st(0)
0036:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0039:    DD 5E 20                fstp     qword ptr [esi + 0x20]
003C:    DC E9                   fsub     st(1), st(0)
003E:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0041:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0044:    D8 CB                   fmul     st(3)
0046:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0049:    D8 CA                   fmul     st(2)
004B:    DD 5E E8                fstp     qword ptr [esi - 0x18]
004E:    DE C9                   fmulp    st(1)
0050:    DD 5E F0                fstp     qword ptr [esi - 0x10]
0053:    5F                      pop      edi
0054:    5E                      pop      esi
0055:    5D                      pop      ebp
0056:    C2 08 00                ret      8
