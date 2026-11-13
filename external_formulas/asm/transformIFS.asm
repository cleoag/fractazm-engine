; Disassembly: transformIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 128 bytes, decoded: 128 bytes (100.0%)
; DEoption: 21

0000:    DD 47 A8                fld      qword ptr [edi - 0x58]
0003:    DD 46 98                fld      qword ptr [esi - 0x68]
0006:    D8 C9                   fmul     st(1)
0008:    DC 47 E0                fadd     qword ptr [edi - 0x20]
000B:    DD 46 90                fld      qword ptr [esi - 0x70]
000E:    D8 CA                   fmul     st(2)
0010:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0013:    DD 46 88                fld      qword ptr [esi - 0x78]
0016:    D8 CB                   fmul     st(3)
0018:    DC 47 F0                fadd     qword ptr [edi - 0x10]
001B:    DD 46 70                fld      qword ptr [esi + 0x70]
001E:    D8 CC                   fmul     st(4)
0020:    DD 5E 70                fstp     qword ptr [esi + 0x70]
0023:    D9 C0                   fld      st(0)
0025:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0028:    D9 C2                   fld      st(2)
002A:    D8 4F C0                fmul     dword ptr [edi - 0x40]
002D:    DE C1                   faddp    st(1)
002F:    D9 C3                   fld      st(3)
0031:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0034:    DE C1                   faddp    st(1)
0036:    D9 C1                   fld      st(1)
0038:    D8 4F DC                fmul     dword ptr [edi - 0x24]
003B:    D9 C3                   fld      st(3)
003D:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0040:    DE C1                   faddp    st(1)
0042:    D9 C4                   fld      st(4)
0044:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0047:    DE C1                   faddp    st(1)
0049:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
004B:    D8 4F D0                fmul     dword ptr [edi - 0x30]
004E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0050:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0053:    DE C3                   faddp    st(3)
0055:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0057:    D8 4F C8                fmul     dword ptr [edi - 0x38]
005A:    DE C2                   faddp    st(2)
005C:    83 7F B8 00             cmp      dword ptr [edi - 0x48], 0
0060:    74 02                   je       0x64    ; Conditional jump
0062:    D9 E1                   fabs    ; |ST(0)|
0064:    DD 5E 88                fstp     qword ptr [esi - 0x78]
0067:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
006B:    74 02                   je       0x6f    ; Conditional jump
006D:    D9 E1                   fabs    ; |ST(0)|
006F:    DD 5E 90                fstp     qword ptr [esi - 0x70]
0072:    83 7F B0 00             cmp      dword ptr [edi - 0x50], 0
0076:    74 02                   je       0x7a    ; Conditional jump
0078:    D9 E1                   fabs    ; |ST(0)|
007A:    DD 5E 98                fstp     qword ptr [esi - 0x68]
007D:    DD D8                   fstp     st(0)
007F:    C3                      ret
