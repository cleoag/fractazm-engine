; Disassembly: Aexion1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 225 bytes, decoded: 225 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    83 BE D0 00 00 00 00    cmp      dword ptr [esi + 0xd0], 0
0012:    75 79                   jne      0x8d    ; Conditional jump
0014:    FF 86 D0 00 00 00       inc      dword ptr [esi + 0xd0]
001A:    DD 47 F0                fld      qword ptr [edi - 0x10]
001D:    DD 01                   fld      qword ptr [ecx]
001F:    DD 02                   fld      qword ptr [edx]
0021:    DD 00                   fld      qword ptr [eax]
0023:    D9 C0                   fld      st(0)
0025:    D8 C2                   fadd     st(2)
0027:    D8 C3                   fadd     st(3)
0029:    D9 E1                   fabs    ; |ST(0)|
002B:    D8 C4                   fadd     st(4)
002D:    DD 18                   fstp     qword ptr [eax]
002F:    D9 C2                   fld      st(2)
0031:    D8 E1                   fsub     st(1)
0033:    D8 E2                   fsub     st(2)
0035:    D9 E1                   fabs    ; |ST(0)|
0037:    D8 C4                   fadd     st(4)
0039:    DD 1A                   fstp     qword ptr [edx]
003B:    D9 C1                   fld      st(1)
003D:    D8 E1                   fsub     st(1)
003F:    D8 E3                   fsub     st(3)
0041:    D9 E1                   fabs    ; |ST(0)|
0043:    D8 C4                   fadd     st(4)
0045:    DD 19                   fstp     qword ptr [ecx]
0047:    DE E1                   fsubrp   st(1)
0049:    DE E1                   fsubrp   st(1)
004B:    D9 E1                   fabs    ; |ST(0)|
004D:    D8 C1                   fadd     st(1)
004F:    DD 59 08                fstp     qword ptr [ecx + 8]
0052:    DD 46 28                fld      qword ptr [esi + 0x28]
0055:    DD 46 20                fld      qword ptr [esi + 0x20]
0058:    DD 46 18                fld      qword ptr [esi + 0x18]
005B:    D9 C0                   fld      st(0)
005D:    D8 C2                   fadd     st(2)
005F:    D8 C3                   fadd     st(3)
0061:    D9 E1                   fabs    ; |ST(0)|
0063:    D8 C4                   fadd     st(4)
0065:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0068:    D9 C2                   fld      st(2)
006A:    D8 E1                   fsub     st(1)
006C:    D8 E2                   fsub     st(2)
006E:    D9 E1                   fabs    ; |ST(0)|
0070:    D8 C4                   fadd     st(4)
0072:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0075:    D9 C1                   fld      st(1)
0077:    D8 E1                   fsub     st(1)
0079:    D8 E3                   fsub     st(3)
007B:    D9 E1                   fabs    ; |ST(0)|
007D:    D8 C4                   fadd     st(4)
007F:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0082:    DE E1                   fsubrp   st(1)
0084:    DE E1                   fsubrp   st(1)
0086:    D9 E1                   fabs    ; |ST(0)|
0088:    DE C1                   faddp    st(1)
008A:    DD 5E C8                fstp     qword ptr [esi - 0x38]
008D:    DD 41 08                fld      qword ptr [ecx + 8]
0090:    DD 01                   fld      qword ptr [ecx]
0092:    DD 02                   fld      qword ptr [edx]
0094:    DD 00                   fld      qword ptr [eax]
0096:    D9 C2                   fld      st(2)
0098:    D8 CC                   fmul     st(4)
009A:    D8 C0                   fadd     st(0)
009C:    D9 C1                   fld      st(1)
009E:    D8 CB                   fmul     st(3)
00A0:    D8 C0                   fadd     st(0)
00A2:    D9 C2                   fld      st(2)
00A4:    D8 C8                   fmul     st(0)
00A6:    D9 C4                   fld      st(4)
00A8:    D8 C8                   fmul     st(0)
00AA:    DE E9                   fsubp    st(1)
00AC:    D9 C0                   fld      st(0)
00AE:    D8 C3                   fadd     st(3)
00B0:    DC 46 18                fadd     qword ptr [esi + 0x18]
00B3:    DD 18                   fstp     qword ptr [eax]
00B5:    DE EA                   fsubp    st(2)
00B7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00B9:    DC 46 20                fadd     qword ptr [esi + 0x20]
00BC:    DD 1A                   fstp     qword ptr [edx]
00BE:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00C0:    D8 C8                   fmul     st(0)
00C2:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
00C4:    D8 C8                   fmul     st(0)
00C6:    DE EC                   fsubp    st(4)
00C8:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00CA:    D9 C1                   fld      st(1)
00CC:    D8 C1                   fadd     st(1)
00CE:    DC 46 28                fadd     qword ptr [esi + 0x28]
00D1:    DD 19                   fstp     qword ptr [ecx]
00D3:    DE E1                   fsubrp   st(1)
00D5:    DC 46 C8                fadd     qword ptr [esi - 0x38]
00D8:    DD 59 08                fstp     qword ptr [ecx + 8]
00DB:    5F                      pop      edi
00DC:    5E                      pop      esi
00DD:    5D                      pop      ebp
00DE:    C2 08 00                ret      8
