; Disassembly: MandyCousin3.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 122 bytes, decoded: 122 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 00                   fld      qword ptr [eax]
000D:    D8 C8                   fmul     st(0)
000F:    DD 02                   fld      qword ptr [edx]
0011:    D8 C8                   fmul     st(0)
0013:    D9 E0                   fchs    ; -ST(0)
0015:    DE C1                   faddp    st(1)
0017:    DD 01                   fld      qword ptr [ecx]
0019:    D8 C8                   fmul     st(0)
001B:    DE C1                   faddp    st(1)
001D:    DD 41 08                fld      qword ptr [ecx + 8]
0020:    D8 C8                   fmul     st(0)
0022:    DE C1                   faddp    st(1)
0024:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0027:    DC 46 18                fadd     qword ptr [esi + 0x18]
002A:    D9 D0                   fnop
002C:    DD 00                   fld      qword ptr [eax]
002E:    DC 0A                   fmul     qword ptr [edx]
0030:    DD 01                   fld      qword ptr [ecx]
0032:    DC 49 08                fmul     qword ptr [ecx + 8]
0035:    DE C1                   faddp    st(1)
0037:    DC 4F E8                fmul     qword ptr [edi - 0x18]
003A:    DC 46 20                fadd     qword ptr [esi + 0x20]
003D:    D9 D0                   fnop
003F:    DD 00                   fld      qword ptr [eax]
0041:    DC 09                   fmul     qword ptr [ecx]
0043:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0046:    DD 02                   fld      qword ptr [edx]
0048:    DC 49 08                fmul     qword ptr [ecx + 8]
004B:    DC 4F D8                fmul     qword ptr [edi - 0x28]
004E:    DE C1                   faddp    st(1)
0050:    DC 46 28                fadd     qword ptr [esi + 0x28]
0053:    D9 D0                   fnop
0055:    DD 00                   fld      qword ptr [eax]
0057:    DC 49 08                fmul     qword ptr [ecx + 8]
005A:    DD 02                   fld      qword ptr [edx]
005C:    DC 09                   fmul     qword ptr [ecx]
005E:    DE E9                   fsubp    st(1)
0060:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0063:    DC 46 C8                fadd     qword ptr [esi - 0x38]
0066:    DC 47 C8                fadd     qword ptr [edi - 0x38]
0069:    DD 59 08                fstp     qword ptr [ecx + 8]
006C:    DD 19                   fstp     qword ptr [ecx]
006E:    DD 1A                   fstp     qword ptr [edx]
0070:    DD 18                   fstp     qword ptr [eax]
0072:    90                      nop
0073:    90                      nop
0074:    5F                      pop      edi
0075:    5E                      pop      esi
0076:    5D                      pop      ebp
0077:    C2 08 00                ret      8
