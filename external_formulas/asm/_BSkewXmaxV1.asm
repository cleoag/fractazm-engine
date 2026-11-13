; Disassembly: _BSkewXmaxV1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 123 bytes, decoded: 123 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 30 00 00 00       sub      esp, 0x30
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 03                   fld      qword ptr [ebx]
0018:    D9 E1                   fabs    ; |ST(0)|
001A:    DC 67 F0                fsub     qword ptr [edi - 0x10]
001D:    DD 03                   fld      qword ptr [ebx]
001F:    D9 E1                   fabs    ; |ST(0)|
0021:    D9 E0                   fchs    ; -ST(0)
0023:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0026:    DE C9                   fmulp    st(1)
0028:    DC 4F E8                fmul     qword ptr [edi - 0x18]
002B:    DD 5D F8                fstp     qword ptr [ebp - 8]
002E:    EB 05                   jmp      0x35
0030:    D8 E0                   fsub     st(0)
0032:    DD 5D F8                fstp     qword ptr [ebp - 8]
0035:    DD 46 18                fld      qword ptr [esi + 0x18]
0038:    D9 E1                   fabs    ; |ST(0)|
003A:    DD 46 18                fld      qword ptr [esi + 0x18]
003D:    D9 E1                   fabs    ; |ST(0)|
003F:    DC 47 C0                fadd     qword ptr [edi - 0x40]
0042:    DE C9                   fmulp    st(1)
0044:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0047:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
004A:    EB 05                   jmp      0x51
004C:    D8 E0                   fsub     st(0)
004E:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0051:    DD 02                   fld      qword ptr [edx]
0053:    D8 C8                   fmul     st(0)
0055:    DD 01                   fld      qword ptr [ecx]
0057:    D8 C8                   fmul     st(0)
0059:    DE C1                   faddp    st(1)
005B:    D9 FA                   fsqrt    ; sqrt(ST(0))
005D:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0060:    DC 45 F8                fadd     qword ptr [ebp - 8]
0063:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0066:    D9 E0                   fchs    ; -ST(0)
0068:    DC 03                   fadd     qword ptr [ebx]
006A:    D9 D0                   fnop
006C:    DD 1B                   fstp     qword ptr [ebx]
006E:    D9 D0                   fnop
0070:    8B C3                   mov      eax, ebx
0072:    5F                      pop      edi
0073:    5E                      pop      esi
0074:    5B                      pop      ebx
0075:    89 EC                   mov      esp, ebp
0077:    5D                      pop      ebp
0078:    C2 08 00                ret      8
