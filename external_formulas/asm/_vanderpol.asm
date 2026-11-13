; Disassembly: _vanderpol.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 114 bytes, decoded: 114 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 00                   fld      qword ptr [eax]
000C:    DC 4E E0                fmul     qword ptr [esi - 0x20]
000F:    DC 01                   fadd     qword ptr [ecx]
0011:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0014:    D9 D0                   fnop
0016:    DD 02                   fld      qword ptr [edx]
0018:    DC 4E E0                fmul     qword ptr [esi - 0x20]
001B:    DC 41 08                fadd     qword ptr [ecx + 8]
001E:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0021:    D9 D0                   fnop
0023:    DD 02                   fld      qword ptr [edx]
0025:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0028:    DD 00                   fld      qword ptr [eax]
002A:    DC 4E D0                fmul     qword ptr [esi - 0x30]
002D:    D9 E0                   fchs    ; -ST(0)
002F:    DE C1                   faddp    st(1)
0031:    DD 00                   fld      qword ptr [eax]
0033:    D8 C8                   fmul     st(0)
0035:    D9 E0                   fchs    ; -ST(0)
0037:    DC 4E C0                fmul     qword ptr [esi - 0x40]
003A:    DC 46 C8                fadd     qword ptr [esi - 0x38]
003D:    DC 09                   fmul     qword ptr [ecx]
003F:    DE C1                   faddp    st(1)
0041:    D9 D0                   fnop
0043:    DD 00                   fld      qword ptr [eax]
0045:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0048:    DD 02                   fld      qword ptr [edx]
004A:    DC 4E D0                fmul     qword ptr [esi - 0x30]
004D:    D9 E0                   fchs    ; -ST(0)
004F:    DE C1                   faddp    st(1)
0051:    DD 02                   fld      qword ptr [edx]
0053:    D8 C8                   fmul     st(0)
0055:    D9 E0                   fchs    ; -ST(0)
0057:    DC 4E B8                fmul     qword ptr [esi - 0x48]
005A:    DC 46 E8                fadd     qword ptr [esi - 0x18]
005D:    DC 49 08                fmul     qword ptr [ecx + 8]
0060:    DE C1                   faddp    st(1)
0062:    D9 D0                   fnop
0064:    DD 59 08                fstp     qword ptr [ecx + 8]
0067:    DD 19                   fstp     qword ptr [ecx]
0069:    DD 1A                   fstp     qword ptr [edx]
006B:    DD 18                   fstp     qword ptr [eax]
006D:    5E                      pop      esi
006E:    5D                      pop      ebp
006F:    C2 08 00                ret      8
