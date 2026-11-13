; Disassembly: _Rotate4d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 135 bytes, decoded: 135 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 41 08                fld      qword ptr [ecx + 8]
000D:    DD 01                   fld      qword ptr [ecx]
000F:    DD 02                   fld      qword ptr [edx]
0011:    DD 00                   fld      qword ptr [eax]
0013:    D9 C0                   fld      st(0)
0015:    D8 4E F4                fmul     dword ptr [esi - 0xc]
0018:    D9 C2                   fld      st(2)
001A:    D8 4E F0                fmul     dword ptr [esi - 0x10]
001D:    DE C1                   faddp    st(1)
001F:    D9 C3                   fld      st(3)
0021:    D8 4E EC                fmul     dword ptr [esi - 0x14]
0024:    DE C1                   faddp    st(1)
0026:    D9 C4                   fld      st(4)
0028:    D8 4E E8                fmul     dword ptr [esi - 0x18]
002B:    DE C1                   faddp    st(1)
002D:    DD 18                   fstp     qword ptr [eax]
002F:    D9 C0                   fld      st(0)
0031:    D8 4E E4                fmul     dword ptr [esi - 0x1c]
0034:    D9 C2                   fld      st(2)
0036:    D8 4E E0                fmul     dword ptr [esi - 0x20]
0039:    DE C1                   faddp    st(1)
003B:    D9 C3                   fld      st(3)
003D:    D8 4E DC                fmul     dword ptr [esi - 0x24]
0040:    DE C1                   faddp    st(1)
0042:    D9 C4                   fld      st(4)
0044:    D8 4E D8                fmul     dword ptr [esi - 0x28]
0047:    DE C1                   faddp    st(1)
0049:    DD 1A                   fstp     qword ptr [edx]
004B:    D9 C0                   fld      st(0)
004D:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
0050:    D9 C2                   fld      st(2)
0052:    D8 4E D0                fmul     dword ptr [esi - 0x30]
0055:    DE C1                   faddp    st(1)
0057:    D9 C3                   fld      st(3)
0059:    D8 4E CC                fmul     dword ptr [esi - 0x34]
005C:    DE C1                   faddp    st(1)
005E:    D9 C4                   fld      st(4)
0060:    D8 4E C8                fmul     dword ptr [esi - 0x38]
0063:    DE C1                   faddp    st(1)
0065:    DD 19                   fstp     qword ptr [ecx]
0067:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
006A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006C:    D8 4E C0                fmul     dword ptr [esi - 0x40]
006F:    DE C1                   faddp    st(1)
0071:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0073:    D8 4E BC                fmul     dword ptr [esi - 0x44]
0076:    DE C1                   faddp    st(1)
0078:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
007A:    D8 4E B8                fmul     dword ptr [esi - 0x48]
007D:    DE C1                   faddp    st(1)
007F:    DD 59 08                fstp     qword ptr [ecx + 8]
0082:    5E                      pop      esi
0083:    5D                      pop      ebp
0084:    C2 08 00                ret      8
