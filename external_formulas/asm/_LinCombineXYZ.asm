; Disassembly: _LinCombineXYZ.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 82 bytes, decoded: 82 bytes (100.0%)
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
0012:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0015:    D9 C2                   fld      st(2)
0017:    DC 4E E8                fmul     qword ptr [esi - 0x18]
001A:    DE C1                   faddp    st(1)
001C:    D9 C3                   fld      st(3)
001E:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0021:    DE C1                   faddp    st(1)
0023:    DD 18                   fstp     qword ptr [eax]
0025:    D9 C0                   fld      st(0)
0027:    DC 4E D8                fmul     qword ptr [esi - 0x28]
002A:    D9 C2                   fld      st(2)
002C:    DC 4E D0                fmul     qword ptr [esi - 0x30]
002F:    DE C1                   faddp    st(1)
0031:    D9 C3                   fld      st(3)
0033:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0036:    DE C1                   faddp    st(1)
0038:    DD 1A                   fstp     qword ptr [edx]
003A:    DC 4E C0                fmul     qword ptr [esi - 0x40]
003D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
003F:    DC 4E B8                fmul     qword ptr [esi - 0x48]
0042:    DE C1                   faddp    st(1)
0044:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0046:    DC 4E B0                fmul     qword ptr [esi - 0x50]
0049:    DE C1                   faddp    st(1)
004B:    DD 19                   fstp     qword ptr [ecx]
004D:    5E                      pop      esi
004E:    5D                      pop      ebp
004F:    C2 08 00                ret      8
