; Disassembly: _YplusSinZ2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 47 bytes, decoded: 47 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DC 4E F0                fmul     qword ptr [esi - 0x10]
000F:    D9 EB                   fldpi    ; Load pi
0011:    DE C9                   fmulp    st(1)
0013:    D9 FF                   fcos    ; cos(ST(0))
0015:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0018:    DC 66 E8                fsub     qword ptr [esi - 0x18]
001B:    DC 06                   fadd     qword ptr [esi]
001D:    DC 4E 08                fmul     qword ptr [esi + 8]
0020:    DD 01                   fld      qword ptr [ecx]
0022:    D9 FE                   fsin    ; sin(ST(0))
0024:    DE C9                   fmulp    st(1)
0026:    DC 02                   fadd     qword ptr [edx]
0028:    DD 1A                   fstp     qword ptr [edx]
002A:    5E                      pop      esi
002B:    5D                      pop      ebp
002C:    C2 08 00                ret      8
