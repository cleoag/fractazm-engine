; Disassembly: _updateC_4d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 33 bytes, decoded: 33 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    DD 41 08                fld      qword ptr [ecx + 8]
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DD 02                   fld      qword ptr [edx]
000E:    DD 00                   fld      qword ptr [eax]
0010:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0013:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0016:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0019:    DD 5E C8                fstp     qword ptr [esi - 0x38]
001C:    5E                      pop      esi
001D:    5D                      pop      ebp
001E:    C2 08 00                ret      8
