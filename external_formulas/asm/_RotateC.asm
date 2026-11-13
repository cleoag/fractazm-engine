; Disassembly: _RotateC.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 92 bytes, decoded: 92 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 46 28                fld      qword ptr [esi + 0x28]
000E:    DD 46 20                fld      qword ptr [esi + 0x20]
0011:    DD 46 18                fld      qword ptr [esi + 0x18]
0014:    D9 C0                   fld      st(0)
0016:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0019:    D9 C2                   fld      st(2)
001B:    D8 4F D8                fmul     dword ptr [edi - 0x28]
001E:    DE C1                   faddp    st(1)
0020:    D9 C3                   fld      st(3)
0022:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0025:    DE C1                   faddp    st(1)
0027:    D9 C1                   fld      st(1)
0029:    D8 4F F4                fmul     dword ptr [edi - 0xc]
002C:    D9 C3                   fld      st(3)
002E:    D8 4F F0                fmul     dword ptr [edi - 0x10]
0031:    DE C1                   faddp    st(1)
0033:    D9 C4                   fld      st(4)
0035:    D8 4F EC                fmul     dword ptr [edi - 0x14]
0038:    DE C1                   faddp    st(1)
003A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003C:    D8 4F E8                fmul     dword ptr [edi - 0x18]
003F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0041:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
0044:    DE C3                   faddp    st(3)
0046:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0048:    D8 4F E0                fmul     dword ptr [edi - 0x20]
004B:    DE C2                   faddp    st(2)
004D:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0050:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0053:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0056:    5F                      pop      edi
0057:    5E                      pop      esi
0058:    5D                      pop      ebp
0059:    C2 08 00                ret      8
