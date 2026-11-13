; Disassembly: _SphereInvC.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 94 bytes, decoded: 94 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    DD 46 18                fld      qword ptr [esi + 0x18]
000A:    DD 46 20                fld      qword ptr [esi + 0x20]
000D:    DD 46 28                fld      qword ptr [esi + 0x28]
0010:    D9 C2                   fld      st(2)
0012:    D8 C8                   fmul     st(0)
0014:    D9 C2                   fld      st(2)
0016:    D8 C8                   fmul     st(0)
0018:    D9 C2                   fld      st(2)
001A:    D8 C8                   fmul     st(0)
001C:    DE C1                   faddp    st(1)
001E:    DE C1                   faddp    st(1)
0020:    D9 E8                   fld1    ; Load 1.0
0022:    DE F1                   fdivrp   st(1)
0024:    DC CB                   fmul     st(3), st(0)
0026:    DC CA                   fmul     st(2), st(0)
0028:    DE C9                   fmulp    st(1)
002A:    DD 5E 28                fstp     qword ptr [esi + 0x28]
002D:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0030:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0033:    DD 00                   fld      qword ptr [eax]
0035:    DD 02                   fld      qword ptr [edx]
0037:    DD 01                   fld      qword ptr [ecx]
0039:    D9 C2                   fld      st(2)
003B:    D8 C8                   fmul     st(0)
003D:    D9 C2                   fld      st(2)
003F:    D8 C8                   fmul     st(0)
0041:    D9 C2                   fld      st(2)
0043:    D8 C8                   fmul     st(0)
0045:    DE C1                   faddp    st(1)
0047:    DE C1                   faddp    st(1)
0049:    D9 E8                   fld1    ; Load 1.0
004B:    DE F1                   fdivrp   st(1)
004D:    DC CB                   fmul     st(3), st(0)
004F:    DC CA                   fmul     st(2), st(0)
0051:    DE C9                   fmulp    st(1)
0053:    DD 19                   fstp     qword ptr [ecx]
0055:    DD 1A                   fstp     qword ptr [edx]
0057:    DD 18                   fstp     qword ptr [eax]
0059:    5E                      pop      esi
005A:    5D                      pop      ebp
005B:    C2 08 00                ret      8
