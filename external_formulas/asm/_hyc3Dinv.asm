; Disassembly: _hyc3Dinv.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 148 bytes, decoded: 148 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DD 02                   fld      qword ptr [edx]
000E:    DD 00                   fld      qword ptr [eax]
0010:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0013:    DD 18                   fstp     qword ptr [eax]
0015:    DC 46 E8                fadd     qword ptr [esi - 0x18]
0018:    DD 1A                   fstp     qword ptr [edx]
001A:    DC 46 E0                fadd     qword ptr [esi - 0x20]
001D:    DD 19                   fstp     qword ptr [ecx]
001F:    DD 00                   fld      qword ptr [eax]
0021:    D8 C8                   fmul     st(0)
0023:    DD 00                   fld      qword ptr [eax]
0025:    DE C9                   fmulp    st(1)
0027:    DD 02                   fld      qword ptr [edx]
0029:    D8 C8                   fmul     st(0)
002B:    DD 02                   fld      qword ptr [edx]
002D:    DE C9                   fmulp    st(1)
002F:    DE C1                   faddp    st(1)
0031:    DD 01                   fld      qword ptr [ecx]
0033:    D8 C8                   fmul     st(0)
0035:    DD 01                   fld      qword ptr [ecx]
0037:    DE C9                   fmulp    st(1)
0039:    DE C1                   faddp    st(1)
003B:    DD 00                   fld      qword ptr [eax]
003D:    DC 09                   fmul     qword ptr [ecx]
003F:    DC 0A                   fmul     qword ptr [edx]
0041:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0043:    D8 E1                   fsub     st(1)
0045:    D8 E1                   fsub     st(1)
0047:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0049:    DE E1                   fsubrp   st(1)
004B:    D9 E0                   fchs    ; -ST(0)
004D:    DD 00                   fld      qword ptr [eax]
004F:    D8 C8                   fmul     st(0)
0051:    DD 02                   fld      qword ptr [edx]
0053:    DD 01                   fld      qword ptr [ecx]
0055:    DE C9                   fmulp    st(1)
0057:    DE E9                   fsubp    st(1)
0059:    D8 F1                   fdiv     st(1)
005B:    DC 4E D8                fmul     qword ptr [esi - 0x28]
005E:    DC 66 F0                fsub     qword ptr [esi - 0x10]
0061:    DD 02                   fld      qword ptr [edx]
0063:    D8 C8                   fmul     st(0)
0065:    DD 00                   fld      qword ptr [eax]
0067:    DD 01                   fld      qword ptr [ecx]
0069:    DE C9                   fmulp    st(1)
006B:    DE E9                   fsubp    st(1)
006D:    D8 F2                   fdiv     st(2)
006F:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0072:    DC 66 E8                fsub     qword ptr [esi - 0x18]
0075:    DD 01                   fld      qword ptr [ecx]
0077:    D8 C8                   fmul     st(0)
0079:    DD 00                   fld      qword ptr [eax]
007B:    DD 02                   fld      qword ptr [edx]
007D:    DE C9                   fmulp    st(1)
007F:    DE E9                   fsubp    st(1)
0081:    DE F3                   fdivrp   st(3)
0083:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0086:    DC 66 E0                fsub     qword ptr [esi - 0x20]
0089:    DD 19                   fstp     qword ptr [ecx]
008B:    DD 1A                   fstp     qword ptr [edx]
008D:    DD 18                   fstp     qword ptr [eax]
008F:    5E                      pop      esi
0090:    5D                      pop      ebp
0091:    C2 08 00                ret      8
