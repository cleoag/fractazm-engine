; Disassembly: _dynamic.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 126 bytes, decoded: 126 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 00                   fld      qword ptr [eax]
000C:    DD 02                   fld      qword ptr [edx]
000E:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0011:    D9 FF                   fcos    ; cos(ST(0))
0013:    D9 E0                   fchs    ; -ST(0)
0015:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0018:    DE C1                   faddp    st(1)
001A:    DD 01                   fld      qword ptr [ecx]
001C:    DC 4E D0                fmul     qword ptr [esi - 0x30]
001F:    D9 FE                   fsin    ; sin(ST(0))
0021:    D9 E0                   fchs    ; -ST(0)
0023:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0026:    DE C1                   faddp    st(1)
0028:    DC 4E F0                fmul     qword ptr [esi - 0x10]
002B:    D9 D0                   fnop
002D:    DD 02                   fld      qword ptr [edx]
002F:    DD 01                   fld      qword ptr [ecx]
0031:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0034:    D9 FF                   fcos    ; cos(ST(0))
0036:    D9 E0                   fchs    ; -ST(0)
0038:    DC 4E E8                fmul     qword ptr [esi - 0x18]
003B:    DE C1                   faddp    st(1)
003D:    DD 00                   fld      qword ptr [eax]
003F:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0042:    D9 FE                   fsin    ; sin(ST(0))
0044:    D9 E0                   fchs    ; -ST(0)
0046:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0049:    DE C1                   faddp    st(1)
004B:    DC 4E F0                fmul     qword ptr [esi - 0x10]
004E:    D9 D0                   fnop
0050:    DD 01                   fld      qword ptr [ecx]
0052:    DD 00                   fld      qword ptr [eax]
0054:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0057:    D9 FF                   fcos    ; cos(ST(0))
0059:    D9 E0                   fchs    ; -ST(0)
005B:    DC 4E E8                fmul     qword ptr [esi - 0x18]
005E:    DE C1                   faddp    st(1)
0060:    DD 02                   fld      qword ptr [edx]
0062:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0065:    D9 FE                   fsin    ; sin(ST(0))
0067:    D9 E0                   fchs    ; -ST(0)
0069:    DC 4E E0                fmul     qword ptr [esi - 0x20]
006C:    DE C1                   faddp    st(1)
006E:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0071:    D9 D0                   fnop
0073:    DD 19                   fstp     qword ptr [ecx]
0075:    DD 1A                   fstp     qword ptr [edx]
0077:    DD 18                   fstp     qword ptr [eax]
0079:    5E                      pop      esi
007A:    5D                      pop      ebp
007B:    C2 08 00                ret      8
