; Disassembly: foldingoctIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 186 bytes, decoded: 186 bytes (100.0%)
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
004B:    8B 57 B8                mov      edx, dword ptr [edi - 0x48]
004E:    83 E2 01                and      edx, 1
0051:    74 02                   je       0x55    ; Conditional jump
0053:    D9 E1                   fabs    ; |ST(0)|
0055:    DD 5E 88                fstp     qword ptr [esi - 0x78]
0058:    8B 57 B4                mov      edx, dword ptr [edi - 0x4c]
005B:    83 E2 01                and      edx, 1
005E:    74 02                   je       0x62    ; Conditional jump
0060:    D9 E1                   fabs    ; |ST(0)|
0062:    DD 5E 90                fstp     qword ptr [esi - 0x70]
0065:    8B 57 B0                mov      edx, dword ptr [edi - 0x50]
0068:    83 E2 01                and      edx, 1
006B:    74 02                   je       0x6f    ; Conditional jump
006D:    D9 E1                   fabs    ; |ST(0)|
006F:    DD 46 90                fld      qword ptr [esi - 0x70]
0072:    DD 46 88                fld      qword ptr [esi - 0x78]
0075:    8B 57 B8                mov      edx, dword ptr [edi - 0x48]
0078:    83 E2 02                and      edx, 2
007B:    74 0B                   je       0x88    ; Conditional jump
007D:    D8 D1                   fcom     st(1)
007F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0081:    80 E4 41                and      ah, 0x41
0084:    74 02                   je       0x88    ; Conditional jump
0086:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0088:    8B 57 B4                mov      edx, dword ptr [edi - 0x4c]
008B:    83 E2 02                and      edx, 2
008E:    74 0B                   je       0x9b    ; Conditional jump
0090:    D8 D2                   fcom     st(2)
0092:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0094:    80 E4 41                and      ah, 0x41
0097:    74 02                   je       0x9b    ; Conditional jump
0099:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
009B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
009D:    8B 57 B0                mov      edx, dword ptr [edi - 0x50]
00A0:    83 E2 02                and      edx, 2
00A3:    74 0B                   je       0xb0    ; Conditional jump
00A5:    D8 D2                   fcom     st(2)
00A7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A9:    80 E4 41                and      ah, 0x41
00AC:    74 02                   je       0xb0    ; Conditional jump
00AE:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00B0:    DD 5E 88                fstp     qword ptr [esi - 0x78]
00B3:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00B6:    DD 5E 98                fstp     qword ptr [esi - 0x68]
00B9:    C3                      ret
