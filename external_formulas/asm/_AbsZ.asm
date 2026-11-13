; Disassembly: _AbsZ.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 33 bytes, decoded: 33 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DD 46 F0                fld      qword ptr [esi - 0x10]
000F:    DE C1                   faddp    st(1)
0011:    D9 E1                   fabs    ; |ST(0)|
0013:    DD 46 F0                fld      qword ptr [esi - 0x10]
0016:    D9 E0                   fchs    ; -ST(0)
0018:    DE C1                   faddp    st(1)
001A:    DD 19                   fstp     qword ptr [ecx]
001C:    5E                      pop      esi
001D:    5D                      pop      ebp
001E:    C2 08 00                ret      8
