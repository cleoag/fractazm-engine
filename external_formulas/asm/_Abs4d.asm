; Disassembly: _Abs4d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 33 bytes, decoded: 33 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    DD 41 08                fld      qword ptr [ecx + 8]
0006:    DD 01                   fld      qword ptr [ecx]
0008:    DD 02                   fld      qword ptr [edx]
000A:    DD 00                   fld      qword ptr [eax]
000C:    D9 E1                   fabs    ; |ST(0)|
000E:    DD 18                   fstp     qword ptr [eax]
0010:    D9 E1                   fabs    ; |ST(0)|
0012:    DD 1A                   fstp     qword ptr [edx]
0014:    D9 E1                   fabs    ; |ST(0)|
0016:    DD 19                   fstp     qword ptr [ecx]
0018:    D9 E1                   fabs    ; |ST(0)|
001A:    DD 59 08                fstp     qword ptr [ecx + 8]
001D:    5D                      pop      ebp
001E:    C2 08 00                ret      8
