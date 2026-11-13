; Disassembly: Makin3D-1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 80 bytes, decoded: 80 bytes (100.0%)

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
0012:    DD 02                   fld      qword ptr [edx]
0014:    D8 C8                   fmul     st(0)
0016:    DE E9                   fsubp    st(1)
0018:    DD 01                   fld      qword ptr [ecx]
001A:    D8 C8                   fmul     st(0)
001C:    DE E9                   fsubp    st(1)
001E:    D9 D0                   fnop
0020:    DD 03                   fld      qword ptr [ebx]
0022:    DC 0A                   fmul     qword ptr [edx]
0024:    D8 C0                   fadd     st(0)
0026:    D9 D0                   fnop
0028:    DD 01                   fld      qword ptr [ecx]
002A:    D8 C0                   fadd     st(0)
002C:    DD 03                   fld      qword ptr [ebx]
002E:    DC 22                   fsub     qword ptr [edx]
0030:    DE C9                   fmulp    st(1)
0032:    D9 D0                   fnop
0034:    DC 46 28                fadd     qword ptr [esi + 0x28]
0037:    DD 19                   fstp     qword ptr [ecx]
0039:    DC 46 20                fadd     qword ptr [esi + 0x20]
003C:    DD 1A                   fstp     qword ptr [edx]
003E:    DC 46 18                fadd     qword ptr [esi + 0x18]
0041:    DD 1B                   fstp     qword ptr [ebx]
0043:    D9 D0                   fnop
0045:    8B C3                   mov      eax, ebx
0047:    5F                      pop      edi
0048:    5E                      pop      esi
0049:    5B                      pop      ebx
004A:    89 EC                   mov      esp, ebp
004C:    5D                      pop      ebp
004D:    C2 08 00                ret      8
