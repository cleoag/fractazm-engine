; Disassembly: _Folding2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 87 bytes, decoded: 87 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 46 F0                fld      qword ptr [esi - 0x10]
000D:    DD 01                   fld      qword ptr [ecx]
000F:    D9 C0                   fld      st(0)
0011:    D8 E2                   fsub     st(2)
0013:    D9 E1                   fabs    ; |ST(0)|
0015:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0017:    DC 66 E8                fsub     qword ptr [esi - 0x18]
001A:    D9 E1                   fabs    ; |ST(0)|
001C:    DE E1                   fsubrp   st(1)
001E:    DC 01                   fadd     qword ptr [ecx]
0020:    D8 E1                   fsub     st(1)
0022:    DD 19                   fstp     qword ptr [ecx]
0024:    DD 02                   fld      qword ptr [edx]
0026:    D9 C0                   fld      st(0)
0028:    D8 E2                   fsub     st(2)
002A:    D9 E1                   fabs    ; |ST(0)|
002C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002E:    DC 66 E8                fsub     qword ptr [esi - 0x18]
0031:    D9 E1                   fabs    ; |ST(0)|
0033:    DE E1                   fsubrp   st(1)
0035:    DC 02                   fadd     qword ptr [edx]
0037:    D8 E1                   fsub     st(1)
0039:    DD 1A                   fstp     qword ptr [edx]
003B:    DD 00                   fld      qword ptr [eax]
003D:    D9 C0                   fld      st(0)
003F:    D8 E2                   fsub     st(2)
0041:    D9 E1                   fabs    ; |ST(0)|
0043:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0045:    DC 66 E8                fsub     qword ptr [esi - 0x18]
0048:    D9 E1                   fabs    ; |ST(0)|
004A:    DE E1                   fsubrp   st(1)
004C:    DC 00                   fadd     qword ptr [eax]
004E:    DE E1                   fsubrp   st(1)
0050:    DD 18                   fstp     qword ptr [eax]
0052:    5E                      pop      esi
0053:    5D                      pop      ebp
0054:    C2 08 00                ret      8
