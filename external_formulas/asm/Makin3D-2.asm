; Disassembly: Makin3D-2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 88 bytes, decoded: 88 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    57                      push     edi
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    8B D8                   mov      ebx, eax
000E:    DD 03                   fld      qword ptr [ebx]
0010:    D8 C8                   fmul     st(0)
0012:    DD 01                   fld      qword ptr [ecx]
0014:    DC 0A                   fmul     qword ptr [edx]
0016:    D8 C0                   fadd     st(0)
0018:    DE C1                   faddp    st(1)
001A:    D9 D0                   fnop
001C:    DD 02                   fld      qword ptr [edx]
001E:    D8 C8                   fmul     st(0)
0020:    DD 01                   fld      qword ptr [ecx]
0022:    DC 0B                   fmul     qword ptr [ebx]
0024:    D8 C0                   fadd     st(0)
0026:    DE C1                   faddp    st(1)
0028:    D9 E0                   fchs    ; -ST(0)
002A:    D9 D0                   fnop
002C:    DD 01                   fld      qword ptr [ecx]
002E:    D8 C8                   fmul     st(0)
0030:    D9 E0                   fchs    ; -ST(0)
0032:    DD 02                   fld      qword ptr [edx]
0034:    DC 0B                   fmul     qword ptr [ebx]
0036:    D8 C0                   fadd     st(0)
0038:    DE C1                   faddp    st(1)
003A:    D9 D0                   fnop
003C:    DC 46 28                fadd     qword ptr [esi + 0x28]
003F:    DD 19                   fstp     qword ptr [ecx]
0041:    DC 46 20                fadd     qword ptr [esi + 0x20]
0044:    DD 1A                   fstp     qword ptr [edx]
0046:    DC 46 18                fadd     qword ptr [esi + 0x18]
0049:    DD 1B                   fstp     qword ptr [ebx]
004B:    D9 D0                   fnop
004D:    8B C3                   mov      eax, ebx
004F:    5F                      pop      edi
0050:    5E                      pop      esi
0051:    5B                      pop      ebx
0052:    89 EC                   mov      esp, ebp
0054:    5D                      pop      ebp
0055:    C2 08 00                ret      8
