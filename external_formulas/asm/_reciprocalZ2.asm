; Disassembly: _reciprocalZ2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 39 bytes, decoded: 39 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    52                      push     edx
0005:    DD 01                   fld      qword ptr [ecx]
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8A 51 07                mov      dl, byte ptr [ecx + 7]
000D:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
0010:    D9 E1                   fabs    ; |ST(0)|
0012:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0015:    80 E2 80                and      dl, 0x80
0018:    D9 E8                   fld1    ; Load 1.0
001A:    DE F1                   fdivrp   st(1)
001C:    DD 19                   fstp     qword ptr [ecx]
001E:    30 51 07                xor      byte ptr [ecx + 7], dl
0021:    5A                      pop      edx
0022:    5E                      pop      esi
0023:    5D                      pop      ebp
0024:    C2 08 00                ret      8
