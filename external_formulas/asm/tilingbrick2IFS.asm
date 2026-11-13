; Disassembly: tilingbrick2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 147 bytes, decoded: 147 bytes (100.0%)
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
0069:    D8 CA                   fmul     st(2)
006B:    DE E9                   fsubp    st(1)
006D:    DD D9                   fstp     st(1)
006F:    DD 47 B4                fld      qword ptr [edi - 0x4c]
0072:    DD 47 A4                fld      qword ptr [edi - 0x5c]
0075:    DA 0C 24                fimul    dword ptr [esp]
0078:    DC 46 88                fadd     qword ptr [esi - 0x78]
007B:    D9 C0                   fld      st(0)
007D:    D8 F2                   fdiv     st(2)
007F:    90                      nop
0080:    D9 FC                   frndint    ; round(ST(0))
0082:    83 C4 08                add      esp, 8
0085:    D8 CA                   fmul     st(2)
0087:    90                      nop
0088:    DE E9                   fsubp    st(1)
008A:    DD D9                   fstp     st(1)
008C:    DD 5E 88                fstp     qword ptr [esi - 0x78]
008F:    DD 5E 90                fstp     qword ptr [esi - 0x70]
0092:    C3                      ret
