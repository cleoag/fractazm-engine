; Disassembly: _RotateBack.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 84 bytes, decoded: 84 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DD 02                   fld      qword ptr [edx]
000E:    DD 00                   fld      qword ptr [eax]
0010:    D9 C0                   fld      st(0)
0012:    D8 4E EC                fmul     dword ptr [esi - 0x14]
0015:    D9 C2                   fld      st(2)
0017:    D8 4E E0                fmul     dword ptr [esi - 0x20]
001A:    DE C1                   faddp    st(1)
001C:    D9 C3                   fld      st(3)
001E:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
0021:    DE C1                   faddp    st(1)
0023:    D9 C1                   fld      st(1)
0025:    D8 4E F4                fmul     dword ptr [esi - 0xc]
0028:    D9 C3                   fld      st(3)
002A:    D8 4E E8                fmul     dword ptr [esi - 0x18]
002D:    DE C1                   faddp    st(1)
002F:    D9 C4                   fld      st(4)
0031:    D8 4E DC                fmul     dword ptr [esi - 0x24]
0034:    DE C1                   faddp    st(1)
0036:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0038:    D8 4E F0                fmul     dword ptr [esi - 0x10]
003B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
003D:    D8 4E E4                fmul     dword ptr [esi - 0x1c]
0040:    DE C3                   faddp    st(3)
0042:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0044:    D8 4E D8                fmul     dword ptr [esi - 0x28]
0047:    DE C2                   faddp    st(2)
0049:    DD 18                   fstp     qword ptr [eax]
004B:    DD 1A                   fstp     qword ptr [edx]
004D:    DD 19                   fstp     qword ptr [ecx]
004F:    5E                      pop      esi
0050:    5D                      pop      ebp
0051:    C2 08 00                ret      8
