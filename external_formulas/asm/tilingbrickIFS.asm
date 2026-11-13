; Disassembly: tilingbrickIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 156 bytes, decoded: 156 bytes (100.0%)
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
0051:    DD 5E 98                fstp     qword ptr [esi - 0x68]
0054:    90                      nop
0055:    DD 47 AC                fld      qword ptr [edi - 0x54]
0058:    DD 46 90                fld      qword ptr [esi - 0x70]
005B:    90                      nop
005C:    D9 C0                   fld      st(0)
005E:    D8 F2                   fdiv     st(2)
0060:    83 C4 F8                add      esp, -8
0063:    DB 1C 24                fistp    dword ptr [esp]
0066:    DB 04 24                fild     dword ptr [esp]
0069:    8B 14 24                mov      edx, dword ptr [esp]
006C:    D8 CA                   fmul     st(2)
006E:    90                      nop
006F:    DE E9                   fsubp    st(1)
0071:    DD D9                   fstp     st(1)
0073:    DD 47 B4                fld      qword ptr [edi - 0x4c]
0076:    DD 46 88                fld      qword ptr [esi - 0x78]
0079:    8B 14 24                mov      edx, dword ptr [esp]
007C:    83 E2 01                and      edx, 1
007F:    75 03                   jne      0x84    ; Conditional jump
0081:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
0084:    D9 C0                   fld      st(0)
0086:    D8 F2                   fdiv     st(2)
0088:    90                      nop
0089:    D9 FC                   frndint    ; round(ST(0))
008B:    83 C4 08                add      esp, 8
008E:    D8 CA                   fmul     st(2)
0090:    90                      nop
0091:    DE E9                   fsubp    st(1)
0093:    DD D9                   fstp     st(1)
0095:    DD 5E 88                fstp     qword ptr [esi - 0x78]
0098:    DD 5E 90                fstp     qword ptr [esi - 0x70]
009B:    C3                      ret
