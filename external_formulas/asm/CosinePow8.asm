; Disassembly: CosinePow8.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 206 bytes, decoded: 206 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    DD 00                   fld      qword ptr [eax]
000A:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000D:    D8 C8                   fmul     st(0)
000F:    DD 02                   fld      qword ptr [edx]
0011:    D8 C8                   fmul     st(0)
0013:    DD 01                   fld      qword ptr [ecx]
0015:    D8 C8                   fmul     st(0)
0017:    D9 C2                   fld      st(2)
0019:    D8 C2                   fadd     st(2)
001B:    D9 C0                   fld      st(0)
001D:    D8 C8                   fmul     st(0)
001F:    D9 C2                   fld      st(2)
0021:    D8 C8                   fmul     st(0)
0023:    D9 C1                   fld      st(1)
0025:    D8 C1                   fadd     st(1)
0027:    DC 4F 18                fmul     qword ptr [edi + 0x18]
002A:    D8 CB                   fmul     st(3)
002C:    D8 CC                   fmul     st(4)
002E:    D9 C1                   fld      st(1)
0030:    D8 C8                   fmul     st(0)
0032:    DE E1                   fsubrp   st(1)
0034:    D9 C1                   fld      st(1)
0036:    D8 CB                   fmul     st(3)
0038:    DC 4F 20                fmul     qword ptr [edi + 0x20]
003B:    DE C1                   faddp    st(1)
003D:    D9 C2                   fld      st(2)
003F:    D8 C8                   fmul     st(0)
0041:    DE C1                   faddp    st(1)
0043:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0046:    DC 46 28                fadd     qword ptr [esi + 0x28]
0049:    D9 C3                   fld      st(3)
004B:    D8 E5                   fsub     st(5)
004D:    DC 4F 08                fmul     qword ptr [edi + 8]
0050:    D8 CD                   fmul     st(5)
0052:    D8 E3                   fsub     st(3)
0054:    D8 CC                   fmul     st(4)
0056:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0058:    DE CD                   fmulp    st(5)
005A:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
005C:    DE C1                   faddp    st(1)
005E:    DC 09                   fmul     qword ptr [ecx]
0060:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0063:    D9 C2                   fld      st(2)
0065:    D9 FA                   fsqrt    ; sqrt(ST(0))
0067:    DE CA                   fmulp    st(2)
0069:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
006B:    DE C9                   fmulp    st(1)
006D:    DC 07                   fadd     qword ptr [edi]
006F:    DE F9                   fdivp    st(1)
0071:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0073:    DD 19                   fstp     qword ptr [ecx]
0075:    D9 C1                   fld      st(1)
0077:    DC 4F 08                fmul     qword ptr [edi + 8]
007A:    D9 C3                   fld      st(3)
007C:    DC 4F 08                fmul     qword ptr [edi + 8]
007F:    D8 E3                   fsub     st(3)
0081:    D9 C4                   fld      st(4)
0083:    DC E2                   fsubr    st(2), st(0)
0085:    D8 C8                   fmul     st(0)
0087:    DC CA                   fmul     st(2), st(0)
0089:    D9 C4                   fld      st(4)
008B:    D8 C8                   fmul     st(0)
008D:    DC CA                   fmul     st(2), st(0)
008F:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0091:    DE C3                   faddp    st(3)
0093:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0095:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0098:    DC 08                   fmul     qword ptr [eax]
009A:    DC 0A                   fmul     qword ptr [edx]
009C:    D8 CB                   fmul     st(3)
009E:    DC 46 20                fadd     qword ptr [esi + 0x20]
00A1:    DD 1A                   fstp     qword ptr [edx]
00A3:    D9 C1                   fld      st(1)
00A5:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00A8:    D8 C1                   fadd     st(1)
00AA:    D8 C9                   fmul     st(1)
00AC:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00AE:    D8 C2                   fadd     st(2)
00B0:    DE CC                   fmulp    st(4)
00B2:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
00B4:    DE CB                   fmulp    st(3)
00B6:    D8 C8                   fmul     st(0)
00B8:    DE C3                   faddp    st(3)
00BA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00BC:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00BF:    DE EA                   fsubp    st(2)
00C1:    DE C9                   fmulp    st(1)
00C3:    DC 46 18                fadd     qword ptr [esi + 0x18]
00C6:    DD 18                   fstp     qword ptr [eax]
00C8:    5F                      pop      edi
00C9:    5E                      pop      esi
00CA:    5D                      pop      ebp
00CB:    C2 08 00                ret      8
