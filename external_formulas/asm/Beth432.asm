; Disassembly: Beth432.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 267 bytes, decoded: 267 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    57                      push     edi
0004:    56                      push     esi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 01                   fld      qword ptr [ecx]
000D:    D8 C8                   fmul     st(0)
000F:    D9 C0                   fld      st(0)
0011:    D8 C8                   fmul     st(0)
0013:    DD 02                   fld      qword ptr [edx]
0015:    D8 C8                   fmul     st(0)
0017:    D9 C0                   fld      st(0)
0019:    D8 C8                   fmul     st(0)
001B:    DD 00                   fld      qword ptr [eax]
001D:    D8 C8                   fmul     st(0)
001F:    D9 C0                   fld      st(0)
0021:    D8 C8                   fmul     st(0)
0023:    D9 C2                   fld      st(2)
0025:    D8 CE                   fmul     st(6)
0027:    D9 C4                   fld      st(4)
0029:    D8 CE                   fmul     st(6)
002B:    DE C1                   faddp    st(1)
002D:    DC 4F 28                fmul     qword ptr [edi + 0x28]
0030:    D9 C4                   fld      st(4)
0032:    D8 CF                   fmul     st(7)
0034:    DC 4F 08                fmul     qword ptr [edi + 8]
0037:    D8 C4                   fadd     st(4)
0039:    D8 C6                   fadd     st(6)
003B:    D8 CB                   fmul     st(3)
003D:    DC 4F 20                fmul     qword ptr [edi + 0x20]
0040:    DE C1                   faddp    st(1)
0042:    D9 C6                   fld      st(6)
0044:    D8 C5                   fadd     st(5)
0046:    D8 CA                   fmul     st(2)
0048:    DC 4F 18                fmul     qword ptr [edi + 0x18]
004B:    DE C1                   faddp    st(1)
004D:    D9 C1                   fld      st(1)
004F:    D8 CB                   fmul     st(3)
0051:    DC C1                   fadd     st(1), st(0)
0053:    DE C1                   faddp    st(1)
0055:    D9 C3                   fld      st(3)
0057:    D8 CD                   fmul     st(5)
0059:    DC 4F 10                fmul     qword ptr [edi + 0x10]
005C:    DE C1                   faddp    st(1)
005E:    D9 C5                   fld      st(5)
0060:    D8 CF                   fmul     st(7)
0062:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0065:    DE C1                   faddp    st(1)
0067:    DC 0F                   fmul     qword ptr [edi]
0069:    DC 08                   fmul     qword ptr [eax]
006B:    DC 46 18                fadd     qword ptr [esi + 0x18]
006E:    DD 18                   fstp     qword ptr [eax]
0070:    D9 C4                   fld      st(4)
0072:    D8 CA                   fmul     st(2)
0074:    D9 C6                   fld      st(6)
0076:    D8 CA                   fmul     st(2)
0078:    DE C1                   faddp    st(1)
007A:    DC 4F 28                fmul     qword ptr [edi + 0x28]
007D:    D9 C2                   fld      st(2)
007F:    D8 CF                   fmul     st(7)
0081:    DC 4F 08                fmul     qword ptr [edi + 8]
0084:    D8 C2                   fadd     st(2)
0086:    D8 C6                   fadd     st(6)
0088:    D8 CD                   fmul     st(5)
008A:    DC 4F 20                fmul     qword ptr [edi + 0x20]
008D:    DE C1                   faddp    st(1)
008F:    D9 C6                   fld      st(6)
0091:    D8 C3                   fadd     st(3)
0093:    D8 CC                   fmul     st(4)
0095:    DC 4F 18                fmul     qword ptr [edi + 0x18]
0098:    DE C1                   faddp    st(1)
009A:    D9 C3                   fld      st(3)
009C:    D8 CD                   fmul     st(5)
009E:    D8 C0                   fadd     st(0)
00A0:    DE C1                   faddp    st(1)
00A2:    D9 C1                   fld      st(1)
00A4:    D8 CB                   fmul     st(3)
00A6:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00A9:    DE C1                   faddp    st(1)
00AB:    D9 C5                   fld      st(5)
00AD:    D8 CF                   fmul     st(7)
00AF:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00B2:    DE C1                   faddp    st(1)
00B4:    DC 0F                   fmul     qword ptr [edi]
00B6:    DC 0A                   fmul     qword ptr [edx]
00B8:    DC 46 20                fadd     qword ptr [esi + 0x20]
00BB:    DD 1A                   fstp     qword ptr [edx]
00BD:    D9 C0                   fld      st(0)
00BF:    D8 CC                   fmul     st(4)
00C1:    D9 C2                   fld      st(2)
00C3:    D8 CC                   fmul     st(4)
00C5:    DE C1                   faddp    st(1)
00C7:    DC 4F 28                fmul     qword ptr [edi + 0x28]
00CA:    D9 C2                   fld      st(2)
00CC:    D8 CD                   fmul     st(5)
00CE:    DC 4F 08                fmul     qword ptr [edi + 8]
00D1:    D8 C2                   fadd     st(2)
00D3:    D8 C4                   fadd     st(4)
00D5:    D8 CF                   fmul     st(7)
00D7:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00DA:    DE C1                   faddp    st(1)
00DC:    D9 C4                   fld      st(4)
00DE:    D8 C3                   fadd     st(3)
00E0:    D8 CE                   fmul     st(6)
00E2:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00E5:    DE C1                   faddp    st(1)
00E7:    D9 CE                   fxch     st(6)    ; Exchange ST(0) and st(6)
00E9:    DE CD                   fmulp    st(5)
00EB:    DE C9                   fmulp    st(1)
00ED:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00EF:    DE CA                   fmulp    st(2)
00F1:    DE C1                   faddp    st(1)
00F3:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00F6:    DE C2                   faddp    st(2)
00F8:    D8 C0                   fadd     st(0)
00FA:    DE C1                   faddp    st(1)
00FC:    DC 0F                   fmul     qword ptr [edi]
00FE:    DC 09                   fmul     qword ptr [ecx]
0100:    DC 46 28                fadd     qword ptr [esi + 0x28]
0103:    DD 19                   fstp     qword ptr [ecx]
0105:    5E                      pop      esi
0106:    5F                      pop      edi
0107:    5D                      pop      ebp
0108:    C2 08 00                ret      8
