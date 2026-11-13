; Disassembly: _SinhZ.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 53 bytes, decoded: 53 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DD 46 E0                fld      qword ptr [esi - 0x20]
000F:    DE C9                   fmulp    st(1)
0011:    DD 46 F0                fld      qword ptr [esi - 0x10]
0014:    DE C1                   faddp    st(1)
0016:    D9 C0                   fld      st(0)
0018:    D8 C8                   fmul     st(0)
001A:    D9 E8                   fld1    ; Load 1.0
001C:    DE C1                   faddp    st(1)
001E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0020:    DE C1                   faddp    st(1)
0022:    DD 46 D8                fld      qword ptr [esi - 0x28]
0025:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0027:    D9 F1                   fyl2x
0029:    DD 46 E8                fld      qword ptr [esi - 0x18]
002C:    DE E9                   fsubp    st(1)
002E:    DD 19                   fstp     qword ptr [ecx]
0030:    5E                      pop      esi
0031:    5D                      pop      ebp
0032:    C2 08 00                ret      8
