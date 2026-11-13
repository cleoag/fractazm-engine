; Disassembly: _SquareR.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 39 bytes, decoded: 39 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    DD 01                   fld      qword ptr [ecx]
0009:    DD 02                   fld      qword ptr [edx]
000B:    DD 00                   fld      qword ptr [eax]
000D:    DD 46 38                fld      qword ptr [esi + 0x38]
0010:    D9 C0                   fld      st(0)
0012:    D9 FA                   fsqrt    ; sqrt(ST(0))
0014:    DE F9                   fdivp    st(1)
0016:    DC CB                   fmul     st(3), st(0)
0018:    DC CA                   fmul     st(2), st(0)
001A:    DE C9                   fmulp    st(1)
001C:    DD 18                   fstp     qword ptr [eax]
001E:    DD 1A                   fstp     qword ptr [edx]
0020:    DD 19                   fstp     qword ptr [ecx]
0022:    5E                      pop      esi
0023:    5D                      pop      ebp
0024:    C2 08 00                ret      8
