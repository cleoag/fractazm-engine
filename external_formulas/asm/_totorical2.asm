; Disassembly: _totorical2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 72 bytes, decoded: 72 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 00                   fld      qword ptr [eax]
000C:    D8 C8                   fmul     st(0)
000E:    DD 02                   fld      qword ptr [edx]
0010:    D8 C8                   fmul     st(0)
0012:    DE C1                   faddp    st(1)
0014:    D9 FA                   fsqrt    ; sqrt(ST(0))
0016:    DC 66 F0                fsub     qword ptr [esi - 0x10]
0019:    90                      nop
001A:    D9 C0                   fld      st(0)
001C:    D8 C8                   fmul     st(0)
001E:    DD 01                   fld      qword ptr [ecx]
0020:    D8 C8                   fmul     st(0)
0022:    DE C1                   faddp    st(1)
0024:    D9 FA                   fsqrt    ; sqrt(ST(0))
0026:    90                      nop
0027:    D9 C1                   fld      st(1)
0029:    DD 01                   fld      qword ptr [ecx]
002B:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
002D:    DC 66 E8                fsub     qword ptr [esi - 0x18]
0030:    DC 26                   fsub     qword ptr [esi]
0032:    90                      nop
0033:    90                      nop
0034:    DD 02                   fld      qword ptr [edx]
0036:    DD 00                   fld      qword ptr [eax]
0038:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
003A:    90                      nop
003B:    DD 19                   fstp     qword ptr [ecx]
003D:    DD 1A                   fstp     qword ptr [edx]
003F:    DD 18                   fstp     qword ptr [eax]
0041:    DD D8                   fstp     st(0)
0043:    5E                      pop      esi
0044:    5D                      pop      ebp
0045:    C2 08 00                ret      8
