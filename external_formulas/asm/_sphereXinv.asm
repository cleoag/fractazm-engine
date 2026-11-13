; Disassembly: _sphereXinv.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 96 bytes, decoded: 96 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DD 02                   fld      qword ptr [edx]
000E:    DD 00                   fld      qword ptr [eax]
0010:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0013:    DD 18                   fstp     qword ptr [eax]
0015:    DC 46 E8                fadd     qword ptr [esi - 0x18]
0018:    DD 1A                   fstp     qword ptr [edx]
001A:    DC 46 E0                fadd     qword ptr [esi - 0x20]
001D:    DD 19                   fstp     qword ptr [ecx]
001F:    DD 00                   fld      qword ptr [eax]
0021:    D8 C8                   fmul     st(0)
0023:    DD 02                   fld      qword ptr [edx]
0025:    D8 C8                   fmul     st(0)
0027:    DE C1                   faddp    st(1)
0029:    DD 01                   fld      qword ptr [ecx]
002B:    D8 C8                   fmul     st(0)
002D:    DE C1                   faddp    st(1)
002F:    DD 00                   fld      qword ptr [eax]
0031:    D8 F9                   fdivr    st(1)
0033:    DD 46 D8                fld      qword ptr [esi - 0x28]
0036:    DE C9                   fmulp    st(1)
0038:    DC 66 F0                fsub     qword ptr [esi - 0x10]
003B:    DD 18                   fstp     qword ptr [eax]
003D:    DD 02                   fld      qword ptr [edx]
003F:    D8 F1                   fdiv     st(1)
0041:    DD 46 D8                fld      qword ptr [esi - 0x28]
0044:    DE C9                   fmulp    st(1)
0046:    DC 66 E8                fsub     qword ptr [esi - 0x18]
0049:    DD 1A                   fstp     qword ptr [edx]
004B:    DD 01                   fld      qword ptr [ecx]
004D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
004F:    DE F9                   fdivp    st(1)
0051:    DD 46 D8                fld      qword ptr [esi - 0x28]
0054:    DE C9                   fmulp    st(1)
0056:    DC 66 E0                fsub     qword ptr [esi - 0x20]
0059:    DD 19                   fstp     qword ptr [ecx]
005B:    5E                      pop      esi
005C:    5D                      pop      ebp
005D:    C2 08 00                ret      8
