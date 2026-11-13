; Disassembly: _poincare.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 111 bytes, decoded: 111 bytes (100.0%)
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
002F:    D9 E0                   fchs    ; -ST(0)
0031:    DD 46 D0                fld      qword ptr [esi - 0x30]
0034:    DE C1                   faddp    st(1)
0036:    D9 E1                   fabs    ; |ST(0)|
0038:    DD 00                   fld      qword ptr [eax]
003A:    D9 E1                   fabs    ; |ST(0)|
003C:    D8 F1                   fdiv     st(1)
003E:    DD 46 D8                fld      qword ptr [esi - 0x28]
0041:    DE C9                   fmulp    st(1)
0043:    DC 66 F0                fsub     qword ptr [esi - 0x10]
0046:    DD 18                   fstp     qword ptr [eax]
0048:    DD 02                   fld      qword ptr [edx]
004A:    D9 E1                   fabs    ; |ST(0)|
004C:    D8 F1                   fdiv     st(1)
004E:    DD 46 D8                fld      qword ptr [esi - 0x28]
0051:    DE C9                   fmulp    st(1)
0053:    DC 66 E8                fsub     qword ptr [esi - 0x18]
0056:    DD 1A                   fstp     qword ptr [edx]
0058:    DD 01                   fld      qword ptr [ecx]
005A:    D9 E1                   fabs    ; |ST(0)|
005C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
005E:    DE F9                   fdivp    st(1)
0060:    DD 46 D8                fld      qword ptr [esi - 0x28]
0063:    DE C9                   fmulp    st(1)
0065:    DC 66 E0                fsub     qword ptr [esi - 0x20]
0068:    DD 19                   fstp     qword ptr [ecx]
006A:    5E                      pop      esi
006B:    5D                      pop      ebp
006C:    C2 08 00                ret      8
