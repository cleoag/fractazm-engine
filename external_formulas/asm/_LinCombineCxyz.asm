; Disassembly: _LinCombineCxyz.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 90 bytes, decoded: 90 bytes (100.0%)
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
0016:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0019:    D9 C2                   fld      st(2)
001B:    DC 4F E8                fmul     qword ptr [edi - 0x18]
001E:    DE C1                   faddp    st(1)
0020:    D9 C3                   fld      st(3)
0022:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0025:    DE C1                   faddp    st(1)
0027:    DD 5E 18                fstp     qword ptr [esi + 0x18]
002A:    D9 C0                   fld      st(0)
002C:    DC 4F D8                fmul     qword ptr [edi - 0x28]
002F:    D9 C2                   fld      st(2)
0031:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0034:    DE C1                   faddp    st(1)
0036:    D9 C3                   fld      st(3)
0038:    DC 4F C8                fmul     qword ptr [edi - 0x38]
003B:    DE C1                   faddp    st(1)
003D:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0040:    DC 4F C0                fmul     qword ptr [edi - 0x40]
0043:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0045:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0048:    DE C1                   faddp    st(1)
004A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
004C:    DC 4F B0                fmul     qword ptr [edi - 0x50]
004F:    DE C1                   faddp    st(1)
0051:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0054:    5F                      pop      edi
0055:    5E                      pop      esi
0056:    5D                      pop      ebp
0057:    C2 08 00                ret      8
