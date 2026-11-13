; Disassembly: _lorenz.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 96 bytes, decoded: 96 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 02                   fld      qword ptr [edx]
000C:    DC 20                   fsub     qword ptr [eax]
000E:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0011:    DC 41 08                fadd     qword ptr [ecx + 8]
0014:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0017:    D9 D0                   fnop
0019:    DD 00                   fld      qword ptr [eax]
001B:    DC 4E D8                fmul     qword ptr [esi - 0x28]
001E:    DC 22                   fsub     qword ptr [edx]
0020:    DD 00                   fld      qword ptr [eax]
0022:    DC 09                   fmul     qword ptr [ecx]
0024:    D9 E0                   fchs    ; -ST(0)
0026:    DE C1                   faddp    st(1)
0028:    DC 4E F0                fmul     qword ptr [esi - 0x10]
002B:    D9 D0                   fnop
002D:    DD 00                   fld      qword ptr [eax]
002F:    DC 0A                   fmul     qword ptr [edx]
0031:    DD 01                   fld      qword ptr [ecx]
0033:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0036:    D9 E0                   fchs    ; -ST(0)
0038:    DE C1                   faddp    st(1)
003A:    DC 4E F0                fmul     qword ptr [esi - 0x10]
003D:    D9 D0                   fnop
003F:    DD 41 08                fld      qword ptr [ecx + 8]
0042:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0045:    DD 02                   fld      qword ptr [edx]
0047:    DC 09                   fmul     qword ptr [ecx]
0049:    D9 E0                   fchs    ; -ST(0)
004B:    DE C1                   faddp    st(1)
004D:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0050:    D9 D0                   fnop
0052:    DD 59 08                fstp     qword ptr [ecx + 8]
0055:    DD 19                   fstp     qword ptr [ecx]
0057:    DD 1A                   fstp     qword ptr [edx]
0059:    DD 18                   fstp     qword ptr [eax]
005B:    5E                      pop      esi
005C:    5D                      pop      ebp
005D:    C2 08 00                ret      8
