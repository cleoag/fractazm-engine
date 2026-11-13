; Disassembly: _rabinovich-f.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 100 bytes, decoded: 100 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DC 66 E8                fsub     qword ptr [esi - 0x18]
000F:    DD 00                   fld      qword ptr [eax]
0011:    D8 C8                   fmul     st(0)
0013:    DE C1                   faddp    st(1)
0015:    DC 0A                   fmul     qword ptr [edx]
0017:    DD 00                   fld      qword ptr [eax]
0019:    DC 4E D8                fmul     qword ptr [esi - 0x28]
001C:    DE C1                   faddp    st(1)
001E:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0021:    D9 D0                   fnop
0023:    DD 01                   fld      qword ptr [ecx]
0025:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0028:    DC 46 E8                fadd     qword ptr [esi - 0x18]
002B:    DD 00                   fld      qword ptr [eax]
002D:    D8 C8                   fmul     st(0)
002F:    D9 E0                   fchs    ; -ST(0)
0031:    DE C1                   faddp    st(1)
0033:    DC 08                   fmul     qword ptr [eax]
0035:    DD 02                   fld      qword ptr [edx]
0037:    DC 4E D8                fmul     qword ptr [esi - 0x28]
003A:    DE C1                   faddp    st(1)
003C:    DC 4E F0                fmul     qword ptr [esi - 0x10]
003F:    D9 D0                   fnop
0041:    DD 46 E8                fld      qword ptr [esi - 0x18]
0044:    DC 66 E0                fsub     qword ptr [esi - 0x20]
0047:    DC 09                   fmul     qword ptr [ecx]
0049:    DD 46 D0                fld      qword ptr [esi - 0x30]
004C:    DD 00                   fld      qword ptr [eax]
004E:    DC 0A                   fmul     qword ptr [edx]
0050:    DE C1                   faddp    st(1)
0052:    DE C9                   fmulp    st(1)
0054:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0057:    D9 D0                   fnop
0059:    DD 19                   fstp     qword ptr [ecx]
005B:    DD 1A                   fstp     qword ptr [edx]
005D:    DD 18                   fstp     qword ptr [eax]
005F:    5E                      pop      esi
0060:    5D                      pop      ebp
0061:    C2 08 00                ret      8
