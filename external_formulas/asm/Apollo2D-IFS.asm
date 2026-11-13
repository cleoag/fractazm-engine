; Disassembly: Apollo2D-IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 247 bytes, decoded: 247 bytes (100.0%)
; DEoption: 21

0000:    83 C4 F8                add      esp, -8
0003:    DD 47 B0                fld      qword ptr [edi - 0x50]
0006:    DD 46 90                fld      qword ptr [esi - 0x70]
0009:    D8 C9                   fmul     st(1)
000B:    DD 46 88                fld      qword ptr [esi - 0x78]
000E:    D8 CA                   fmul     st(2)
0010:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0013:    D9 C1                   fld      st(1)
0015:    D9 C1                   fld      st(1)
0017:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0019:    D9 E0                   fchs    ; -ST(0)
001B:    DC 0F                   fmul     qword ptr [edi]
001D:    DB 47 B8                fild     dword ptr [edi - 0x48]
0020:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0022:    D8 C9                   fmul     st(1)
0024:    D9 FC                   frndint    ; round(ST(0))
0026:    DB 14 24                fist     dword ptr [esp]
0029:    DE F1                   fdivrp   st(1)
002B:    DC 4F 08                fmul     qword ptr [edi + 8]
002E:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0030:    D9 C3                   fld      st(3)
0032:    D8 C9                   fmul     st(1)
0034:    D9 C3                   fld      st(3)
0036:    D8 CB                   fmul     st(3)
0038:    DE C1                   faddp    st(1)
003A:    83 24 24 01             and      dword ptr [esp], 1
003E:    75 02                   jne      0x42    ; Conditional jump
0040:    D9 E0                   fchs    ; -ST(0)
0042:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
0044:    DE CA                   fmulp    st(2)
0046:    DE CA                   fmulp    st(2)
0048:    DE E9                   fsubp    st(1)
004A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
004C:    3E DD 46 98             fld      qword ptr ds:[esi - 0x68]
0050:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0052:    3E DC 47 E8             fadd     qword ptr ds:[edi - 0x18]
0056:    D9 C0                   fld      st(0)
0058:    3E D8 4F C4             fmul     dword ptr ds:[edi - 0x3c]
005C:    D9 C2                   fld      st(2)
005E:    3E D8 4F C0             fmul     dword ptr ds:[edi - 0x40]
0062:    DE C1                   faddp    st(1)
0064:    D9 C3                   fld      st(3)
0066:    3E D8 4F BC             fmul     dword ptr ds:[edi - 0x44]
006A:    DE C1                   faddp    st(1)
006C:    D9 C1                   fld      st(1)
006E:    3E D8 4F DC             fmul     dword ptr ds:[edi - 0x24]
0072:    D9 C3                   fld      st(3)
0074:    3E D8 4F D8             fmul     dword ptr ds:[edi - 0x28]
0078:    DE C1                   faddp    st(1)
007A:    D9 C4                   fld      st(4)
007C:    3E D8 4F D4             fmul     dword ptr ds:[edi - 0x2c]
0080:    DE C1                   faddp    st(1)
0082:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0084:    3E D8 4F D0             fmul     dword ptr ds:[edi - 0x30]
0088:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008A:    3E D8 4F CC             fmul     dword ptr ds:[edi - 0x34]
008E:    DE C3                   faddp    st(3)
0090:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0092:    3E D8 4F C8             fmul     dword ptr ds:[edi - 0x38]
0096:    DE C2                   faddp    st(2)
0098:    3E DD 5E 90             fstp     qword ptr ds:[esi - 0x70]
009C:    D9 E0                   fchs    ; -ST(0)
009E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00A0:    3E DC 4F E0             fmul     qword ptr ds:[edi - 0x20]
00A4:    3E DD 5E 98             fstp     qword ptr ds:[esi - 0x68]
00A8:    D9 C0                   fld      st(0)
00AA:    D8 C8                   fmul     st(0)
00AC:    3E DD 46 90             fld      qword ptr ds:[esi - 0x70]
00B0:    D8 C8                   fmul     st(0)
00B2:    DE C1                   faddp    st(1)
00B4:    D9 C0                   fld      st(0)
00B6:    D9 E8                   fld1    ; Load 1.0
00B8:    DE E9                   fsubp    st(1)
00BA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00BC:    D9 E8                   fld1    ; Load 1.0
00BE:    DE C1                   faddp    st(1)
00C0:    3E DD 46 90             fld      qword ptr ds:[esi - 0x70]
00C4:    D8 C0                   fadd     st(0)
00C6:    DE C1                   faddp    st(1)
00C8:    DE CB                   fmulp    st(3)
00CA:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00CC:    D9 E8                   fld1    ; Load 1.0
00CE:    DE F1                   fdivrp   st(1)
00D0:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00D2:    D8 CA                   fmul     st(2)
00D4:    3E DC 47 A8             fadd     qword ptr ds:[edi - 0x58]
00D8:    3E DD 5E 88             fstp     qword ptr ds:[esi - 0x78]
00DC:    D8 C0                   fadd     st(0)
00DE:    D9 E0                   fchs    ; -ST(0)
00E0:    DE C9                   fmulp    st(1)
00E2:    3E DD 5E 90             fstp     qword ptr ds:[esi - 0x70]
00E6:    DD 46 70                fld      qword ptr [esi + 0x70]
00E9:    DD 47 E0                fld      qword ptr [edi - 0x20]
00EC:    D9 E1                   fabs    ; |ST(0)|
00EE:    DE C9                   fmulp    st(1)
00F0:    DD 5E 70                fstp     qword ptr [esi + 0x70]
00F3:    83 C4 08                add      esp, 8
00F6:    C3                      ret
