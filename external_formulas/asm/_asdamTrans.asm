; Disassembly: _asdamTrans.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 127 bytes, decoded: 127 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 00                   fld      qword ptr [eax]
000C:    D8 C8                   fmul     st(0)
000E:    DD 02                   fld      qword ptr [edx]
0010:    D8 C8                   fmul     st(0)
0012:    DD 01                   fld      qword ptr [ecx]
0014:    D8 C8                   fmul     st(0)
0016:    DE C1                   faddp    st(1)
0018:    DE C1                   faddp    st(1)
001A:    D9 FA                   fsqrt    ; sqrt(ST(0))
001C:    DD 46 CC                fld      qword ptr [esi - 0x34]
001F:    DE F1                   fdivrp   st(1)
0021:    DD 01                   fld      qword ptr [ecx]
0023:    D8 C9                   fmul     st(1)
0025:    DD 02                   fld      qword ptr [edx]
0027:    D8 CA                   fmul     st(2)
0029:    DD 00                   fld      qword ptr [eax]
002B:    D8 CB                   fmul     st(3)
002D:    90                      nop
002E:    90                      nop
002F:    90                      nop
0030:    90                      nop
0031:    90                      nop
0032:    90                      nop
0033:    D9 C0                   fld      st(0)
0035:    D8 4E DC                fmul     dword ptr [esi - 0x24]
0038:    D9 C2                   fld      st(2)
003A:    D8 4E D8                fmul     dword ptr [esi - 0x28]
003D:    DE C1                   faddp    st(1)
003F:    D9 C3                   fld      st(3)
0041:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
0044:    DE C1                   faddp    st(1)
0046:    D9 C1                   fld      st(1)
0048:    D8 4E F4                fmul     dword ptr [esi - 0xc]
004B:    D9 C3                   fld      st(3)
004D:    D8 4E F0                fmul     dword ptr [esi - 0x10]
0050:    DE C1                   faddp    st(1)
0052:    D9 C4                   fld      st(4)
0054:    D8 4E EC                fmul     dword ptr [esi - 0x14]
0057:    DE C1                   faddp    st(1)
0059:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
005B:    D8 4E E8                fmul     dword ptr [esi - 0x18]
005E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0060:    D8 4E E4                fmul     dword ptr [esi - 0x1c]
0063:    DE C3                   faddp    st(3)
0065:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0067:    D8 4E E0                fmul     dword ptr [esi - 0x20]
006A:    DE C2                   faddp    st(2)
006C:    DC 00                   fadd     qword ptr [eax]
006E:    DD 18                   fstp     qword ptr [eax]
0070:    DC 02                   fadd     qword ptr [edx]
0072:    DD 1A                   fstp     qword ptr [edx]
0074:    DC 01                   fadd     qword ptr [ecx]
0076:    DD 19                   fstp     qword ptr [ecx]
0078:    DD D8                   fstp     st(0)
007A:    5E                      pop      esi
007B:    5D                      pop      ebp
007C:    C2 08 00                ret      8
