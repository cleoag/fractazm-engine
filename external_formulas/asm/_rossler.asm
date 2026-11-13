; Disassembly: _rossler.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 86 bytes, decoded: 86 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 02                   fld      qword ptr [edx]
000C:    DC 01                   fadd     qword ptr [ecx]
000E:    D9 E0                   fchs    ; -ST(0)
0010:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0013:    D9 D0                   fnop
0015:    DD 00                   fld      qword ptr [eax]
0017:    DD 02                   fld      qword ptr [edx]
0019:    DC 4E E8                fmul     qword ptr [esi - 0x18]
001C:    DE C1                   faddp    st(1)
001E:    DC 41 08                fadd     qword ptr [ecx + 8]
0021:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0024:    D9 D0                   fnop
0026:    DD 46 E0                fld      qword ptr [esi - 0x20]
0029:    DD 00                   fld      qword ptr [eax]
002B:    DC 09                   fmul     qword ptr [ecx]
002D:    DE C1                   faddp    st(1)
002F:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0032:    D9 D0                   fnop
0034:    DD 46 D8                fld      qword ptr [esi - 0x28]
0037:    DC 09                   fmul     qword ptr [ecx]
0039:    D9 E0                   fchs    ; -ST(0)
003B:    DD 46 D0                fld      qword ptr [esi - 0x30]
003E:    DC 49 08                fmul     qword ptr [ecx + 8]
0041:    DE C1                   faddp    st(1)
0043:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0046:    D9 D0                   fnop
0048:    DD 59 08                fstp     qword ptr [ecx + 8]
004B:    DD 19                   fstp     qword ptr [ecx]
004D:    DD 1A                   fstp     qword ptr [edx]
004F:    DD 18                   fstp     qword ptr [eax]
0051:    5E                      pop      esi
0052:    5D                      pop      ebp
0053:    C2 08 00                ret      8
