; Disassembly: transform2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 109 bytes, decoded: 109 bytes (100.0%)
; DEoption: 21

0000:    DD 46 98                fld      qword ptr [esi - 0x68]
0003:    83 7F B0 00             cmp      dword ptr [edi - 0x50], 0
0007:    74 02                   je       0xb    ; Conditional jump
0009:    D9 E1                   fabs    ; |ST(0)|
000B:    DD 46 90                fld      qword ptr [esi - 0x70]
000E:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0012:    74 02                   je       0x16    ; Conditional jump
0014:    D9 E1                   fabs    ; |ST(0)|
0016:    DD 46 88                fld      qword ptr [esi - 0x78]
0019:    83 7F B8 00             cmp      dword ptr [edi - 0x48], 0
001D:    74 02                   je       0x21    ; Conditional jump
001F:    D9 E1                   fabs    ; |ST(0)|
0021:    D9 C0                   fld      st(0)
0023:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0026:    D9 C2                   fld      st(2)
0028:    D8 4F C0                fmul     dword ptr [edi - 0x40]
002B:    DE C1                   faddp    st(1)
002D:    D9 C3                   fld      st(3)
002F:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0032:    DE C1                   faddp    st(1)
0034:    D9 C1                   fld      st(1)
0036:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0039:    D9 C3                   fld      st(3)
003B:    D8 4F D8                fmul     dword ptr [edi - 0x28]
003E:    DE C1                   faddp    st(1)
0040:    D9 C4                   fld      st(4)
0042:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0045:    DE C1                   faddp    st(1)
0047:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0049:    D8 4F D0                fmul     dword ptr [edi - 0x30]
004C:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004E:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0051:    DE C3                   faddp    st(3)
0053:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0055:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0058:    DE C2                   faddp    st(2)
005A:    DC 47 F0                fadd     qword ptr [edi - 0x10]
005D:    DD 5E 88                fstp     qword ptr [esi - 0x78]
0060:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0063:    DD 5E 90                fstp     qword ptr [esi - 0x70]
0066:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0069:    DD 5E 98                fstp     qword ptr [esi - 0x68]
006C:    C3                      ret
