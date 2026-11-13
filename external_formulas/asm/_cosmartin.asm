; Disassembly: _cosmartin.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 91 bytes, decoded: 91 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 02                   fld      qword ptr [edx]
000C:    DD 00                   fld      qword ptr [eax]
000E:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0011:    D9 FF                   fcos    ; cos(ST(0))
0013:    D9 E0                   fchs    ; -ST(0)
0015:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0018:    DE C1                   faddp    st(1)
001A:    DC 4E F0                fmul     qword ptr [esi - 0x10]
001D:    D9 D0                   fnop
001F:    DD 46 D8                fld      qword ptr [esi - 0x28]
0022:    DC 20                   fsub     qword ptr [eax]
0024:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0027:    D9 D0                   fnop
0029:    DD 1A                   fstp     qword ptr [edx]
002B:    DD 18                   fstp     qword ptr [eax]
002D:    D9 D0                   fnop
002F:    DD 41 08                fld      qword ptr [ecx + 8]
0032:    DD 01                   fld      qword ptr [ecx]
0034:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0037:    D9 FF                   fcos    ; cos(ST(0))
0039:    D9 E0                   fchs    ; -ST(0)
003B:    DC 4E E8                fmul     qword ptr [esi - 0x18]
003E:    DE C1                   faddp    st(1)
0040:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0043:    D9 D0                   fnop
0045:    DD 46 D0                fld      qword ptr [esi - 0x30]
0048:    DC 21                   fsub     qword ptr [ecx]
004A:    DC 4E F0                fmul     qword ptr [esi - 0x10]
004D:    D9 D0                   fnop
004F:    DD 59 08                fstp     qword ptr [ecx + 8]
0052:    DD 19                   fstp     qword ptr [ecx]
0054:    D9 D0                   fnop
0056:    5E                      pop      esi
0057:    5D                      pop      ebp
0058:    C2 08 00                ret      8
