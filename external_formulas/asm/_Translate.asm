; Disassembly: _Translate.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 36 bytes, decoded: 36 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DD 02                   fld      qword ptr [edx]
000E:    DD 00                   fld      qword ptr [eax]
0010:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0013:    DD 18                   fstp     qword ptr [eax]
0015:    DC 46 E8                fadd     qword ptr [esi - 0x18]
0018:    DD 1A                   fstp     qword ptr [edx]
001A:    DC 46 E0                fadd     qword ptr [esi - 0x20]
001D:    DD 19                   fstp     qword ptr [ecx]
001F:    5E                      pop      esi
0020:    5D                      pop      ebp
0021:    C2 08 00                ret      8
