; Disassembly: _FlipXZc.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 43 bytes, decoded: 43 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    83 7F F4 00             cmp      dword ptr [edi - 0xc], 0
000F:    74 0C                   je       0x1d    ; Conditional jump
0011:    DD 46 18                fld      qword ptr [esi + 0x18]
0014:    DD 46 28                fld      qword ptr [esi + 0x28]
0017:    DD 5E 18                fstp     qword ptr [esi + 0x18]
001A:    DD 5E 28                fstp     qword ptr [esi + 0x28]
001D:    DD 01                   fld      qword ptr [ecx]
001F:    DD 00                   fld      qword ptr [eax]
0021:    DD 19                   fstp     qword ptr [ecx]
0023:    DD 18                   fstp     qword ptr [eax]
0025:    5F                      pop      edi
0026:    5E                      pop      esi
0027:    5D                      pop      ebp
0028:    C2 08 00                ret      8
