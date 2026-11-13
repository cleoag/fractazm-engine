; Disassembly: _SphereInvC4d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 122 bytes, decoded: 122 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    DD 46 18                fld      qword ptr [esi + 0x18]
000A:    DD 46 20                fld      qword ptr [esi + 0x20]
000D:    DD 46 28                fld      qword ptr [esi + 0x28]
0010:    DD 46 C8                fld      qword ptr [esi - 0x38]
0013:    D9 C3                   fld      st(3)
0015:    D8 C8                   fmul     st(0)
0017:    D9 C3                   fld      st(3)
0019:    D8 C8                   fmul     st(0)
001B:    D9 C3                   fld      st(3)
001D:    D8 C8                   fmul     st(0)
001F:    D9 C3                   fld      st(3)
0021:    D8 C8                   fmul     st(0)
0023:    DE C1                   faddp    st(1)
0025:    DE C1                   faddp    st(1)
0027:    DE C1                   faddp    st(1)
0029:    D9 E8                   fld1    ; Load 1.0
002B:    DE F1                   fdivrp   st(1)
002D:    DC CC                   fmul     st(4), st(0)
002F:    DC CB                   fmul     st(3), st(0)
0031:    DC CA                   fmul     st(2), st(0)
0033:    DE C9                   fmulp    st(1)
0035:    DD 5E C8                fstp     qword ptr [esi - 0x38]
0038:    DD 5E 28                fstp     qword ptr [esi + 0x28]
003B:    DD 5E 20                fstp     qword ptr [esi + 0x20]
003E:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0041:    DD 00                   fld      qword ptr [eax]
0043:    DD 02                   fld      qword ptr [edx]
0045:    DD 01                   fld      qword ptr [ecx]
0047:    DD 41 08                fld      qword ptr [ecx + 8]
004A:    D9 C3                   fld      st(3)
004C:    D8 C8                   fmul     st(0)
004E:    D9 C3                   fld      st(3)
0050:    D8 C8                   fmul     st(0)
0052:    D9 C3                   fld      st(3)
0054:    D8 C8                   fmul     st(0)
0056:    D9 C3                   fld      st(3)
0058:    D8 C8                   fmul     st(0)
005A:    DE C1                   faddp    st(1)
005C:    DE C1                   faddp    st(1)
005E:    DE C1                   faddp    st(1)
0060:    D9 E8                   fld1    ; Load 1.0
0062:    DE F1                   fdivrp   st(1)
0064:    DC CC                   fmul     st(4), st(0)
0066:    DC CB                   fmul     st(3), st(0)
0068:    DC CA                   fmul     st(2), st(0)
006A:    DE C9                   fmulp    st(1)
006C:    DD 59 08                fstp     qword ptr [ecx + 8]
006F:    DD 19                   fstp     qword ptr [ecx]
0071:    DD 1A                   fstp     qword ptr [edx]
0073:    DD 18                   fstp     qword ptr [eax]
0075:    5E                      pop      esi
0076:    5D                      pop      ebp
0077:    C2 08 00                ret      8
