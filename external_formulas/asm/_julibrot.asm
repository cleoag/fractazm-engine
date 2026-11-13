; Disassembly: _julibrot.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 50 bytes, decoded: 50 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    50                      push     eax
0005:    56                      push     esi
0006:    57                      push     edi
0007:    90                      nop
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B D8                   mov      ebx, eax
000D:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0010:    DD 47 F0                fld      qword ptr [edi - 0x10]
0013:    DD 19                   fstp     qword ptr [ecx]
0015:    DD 47 E8                fld      qword ptr [edi - 0x18]
0018:    DD 59 08                fstp     qword ptr [ecx + 8]
001B:    DD 47 E0                fld      qword ptr [edi - 0x20]
001E:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0021:    DD 47 D8                fld      qword ptr [edi - 0x28]
0024:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0027:    90                      nop
0028:    8B C3                   mov      eax, ebx
002A:    5F                      pop      edi
002B:    5E                      pop      esi
002C:    58                      pop      eax
002D:    5B                      pop      ebx
002E:    5D                      pop      ebp
002F:    C2 08 00                ret      8
