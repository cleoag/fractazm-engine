; Disassembly: _sqr_3D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 57 bytes, decoded: 57 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B D8                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 03                   fld      qword ptr [ebx]
000F:    D8 C8                   fmul     st(0)
0011:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0014:    DC 46 E8                fadd     qword ptr [esi - 0x18]
0017:    DD 1B                   fstp     qword ptr [ebx]
0019:    DD 02                   fld      qword ptr [edx]
001B:    D8 C8                   fmul     st(0)
001D:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0020:    DC 46 E0                fadd     qword ptr [esi - 0x20]
0023:    DD 1A                   fstp     qword ptr [edx]
0025:    DD 01                   fld      qword ptr [ecx]
0027:    D8 C8                   fmul     st(0)
0029:    DC 4E F0                fmul     qword ptr [esi - 0x10]
002C:    DC 46 D8                fadd     qword ptr [esi - 0x28]
002F:    DD 19                   fstp     qword ptr [ecx]
0031:    8B C3                   mov      eax, ebx
0033:    5B                      pop      ebx
0034:    5E                      pop      esi
0035:    5D                      pop      ebp
0036:    C2 08 00                ret      8
