; Disassembly: cayley2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 194 bytes, decoded: 194 bytes (100.0%)
; DEoption: 21

0000:    DD 46 98                fld      qword ptr [esi - 0x68]
0003:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0006:    DD 46 90                fld      qword ptr [esi - 0x70]
0009:    DC 47 E8                fadd     qword ptr [edi - 0x18]
000C:    DD 46 88                fld      qword ptr [esi - 0x78]
000F:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0012:    D9 C0                   fld      st(0)
0014:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0017:    D9 C2                   fld      st(2)
0019:    D8 4F C0                fmul     dword ptr [edi - 0x40]
001C:    DE C1                   faddp    st(1)
001E:    D9 C3                   fld      st(3)
0020:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0023:    DE C1                   faddp    st(1)
0025:    D9 C1                   fld      st(1)
0027:    D8 4F DC                fmul     dword ptr [edi - 0x24]
002A:    D9 C3                   fld      st(3)
002C:    D8 4F D8                fmul     dword ptr [edi - 0x28]
002F:    DE C1                   faddp    st(1)
0031:    D9 C4                   fld      st(4)
0033:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0036:    DE C1                   faddp    st(1)
0038:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003A:    D8 4F D0                fmul     dword ptr [edi - 0x30]
003D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
003F:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0042:    DE C3                   faddp    st(3)
0044:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0046:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0049:    DE C2                   faddp    st(2)
004B:    DD 5E 88                fstp     qword ptr [esi - 0x78]
004E:    DD 5E 90                fstp     qword ptr [esi - 0x70]
0051:    DC 4F B4                fmul     qword ptr [edi - 0x4c]
0054:    DD 5E 98                fstp     qword ptr [esi - 0x68]
0057:    DD 47 B4                fld      qword ptr [edi - 0x4c]
005A:    D9 E1                   fabs    ; |ST(0)|
005C:    DC 4E 70                fmul     qword ptr [esi + 0x70]
005F:    DD 5E 70                fstp     qword ptr [esi + 0x70]
0062:    90                      nop
0063:    83 C4 E4                add      esp, -0x1c
0066:    DD 46 88                fld      qword ptr [esi - 0x78]
0069:    D8 C8                   fmul     st(0)
006B:    DD 54 24 10             fst      qword ptr [esp + 0x10]
006F:    DD 46 90                fld      qword ptr [esi - 0x70]
0072:    D8 C8                   fmul     st(0)
0074:    D9 C0                   fld      st(0)
0076:    DC 64 24 10             fsub     qword ptr [esp + 0x10]
007A:    D8 C0                   fadd     st(0)
007C:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0080:    D9 C0                   fld      st(0)
0082:    D8 CA                   fmul     st(2)
0084:    D8 C0                   fadd     st(0)
0086:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0088:    D8 C8                   fmul     st(0)
008A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
008C:    D8 C8                   fmul     st(0)
008E:    DE C1                   faddp    st(1)
0090:    DE C1                   faddp    st(1)
0092:    DD 14 24                fst      qword ptr [esp]
0095:    D9 E8                   fld1    ; Load 1.0
0097:    DE E9                   fsubp    st(1)
0099:    DD 04 24                fld      qword ptr [esp]
009C:    DD 46 90                fld      qword ptr [esi - 0x70]
009F:    DC 4E 88                fmul     qword ptr [esi - 0x78]
00A2:    D8 C0                   fadd     st(0)
00A4:    D8 C0                   fadd     st(0)
00A6:    DE C1                   faddp    st(1)
00A8:    D9 E8                   fld1    ; Load 1.0
00AA:    DE C1                   faddp    st(1)
00AC:    DD 14 24                fst      qword ptr [esp]
00AF:    DE F9                   fdivp    st(1)
00B1:    DD 5E 88                fstp     qword ptr [esi - 0x78]
00B4:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00B8:    DC 34 24                fdiv     qword ptr [esp]
00BB:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00BE:    83 C4 1C                add      esp, 0x1c
00C1:    C3                      ret
