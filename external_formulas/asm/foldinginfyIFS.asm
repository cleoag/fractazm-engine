; Disassembly: foldinginfyIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 293 bytes, decoded: 293 bytes (100.0%)
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
0055:    DD 47 B4                fld      qword ptr [edi - 0x4c]
0058:    D9 E4                   ftst
005A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005C:    D9 D0                   fnop
005E:    9E                      sahf
005F:    DD D8                   fstp     st(0)
0061:    74 36                   je       0x99    ; Conditional jump
0063:    DD 47 B4                fld      qword ptr [edi - 0x4c]
0066:    D8 C0                   fadd     st(0)
0068:    D8 C0                   fadd     st(0)
006A:    DD 46 88                fld      qword ptr [esi - 0x78]
006D:    90                      nop
006E:    D9 C0                   fld      st(0)
0070:    D8 F2                   fdiv     st(2)
0072:    90                      nop
0073:    D9 FC                   frndint    ; round(ST(0))
0075:    D8 CA                   fmul     st(2)
0077:    90                      nop
0078:    DE E9                   fsubp    st(1)
007A:    DD D9                   fstp     st(1)
007C:    D9 E0                   fchs    ; -ST(0)
007E:    D9 C0                   fld      st(0)
0080:    D9 E0                   fchs    ; -ST(0)
0082:    DC 47 B4                fadd     qword ptr [edi - 0x4c]
0085:    D9 E1                   fabs    ; |ST(0)|
0087:    D9 C1                   fld      st(1)
0089:    D9 E0                   fchs    ; -ST(0)
008B:    DC 67 B4                fsub     qword ptr [edi - 0x4c]
008E:    D9 E1                   fabs    ; |ST(0)|
0090:    D9 E0                   fchs    ; -ST(0)
0092:    DE C1                   faddp    st(1)
0094:    DE C1                   faddp    st(1)
0096:    DD 5E 88                fstp     qword ptr [esi - 0x78]
0099:    90                      nop
009A:    DD 47 AC                fld      qword ptr [edi - 0x54]
009D:    D9 E4                   ftst
009F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A1:    D9 D0                   fnop
00A3:    9E                      sahf
00A4:    DD D8                   fstp     st(0)
00A6:    74 36                   je       0xde    ; Conditional jump
00A8:    DD 47 AC                fld      qword ptr [edi - 0x54]
00AB:    D8 C0                   fadd     st(0)
00AD:    D8 C0                   fadd     st(0)
00AF:    DD 46 90                fld      qword ptr [esi - 0x70]
00B2:    90                      nop
00B3:    D9 C0                   fld      st(0)
00B5:    D8 F2                   fdiv     st(2)
00B7:    90                      nop
00B8:    D9 FC                   frndint    ; round(ST(0))
00BA:    D8 CA                   fmul     st(2)
00BC:    90                      nop
00BD:    DE E9                   fsubp    st(1)
00BF:    DD D9                   fstp     st(1)
00C1:    D9 E0                   fchs    ; -ST(0)
00C3:    D9 C0                   fld      st(0)
00C5:    D9 E0                   fchs    ; -ST(0)
00C7:    DC 47 AC                fadd     qword ptr [edi - 0x54]
00CA:    D9 E1                   fabs    ; |ST(0)|
00CC:    D9 C1                   fld      st(1)
00CE:    D9 E0                   fchs    ; -ST(0)
00D0:    DC 67 AC                fsub     qword ptr [edi - 0x54]
00D3:    D9 E1                   fabs    ; |ST(0)|
00D5:    D9 E0                   fchs    ; -ST(0)
00D7:    DE C1                   faddp    st(1)
00D9:    DE C1                   faddp    st(1)
00DB:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00DE:    90                      nop
00DF:    DD 47 A4                fld      qword ptr [edi - 0x5c]
00E2:    D9 E4                   ftst
00E4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E6:    D9 D0                   fnop
00E8:    9E                      sahf
00E9:    DD D8                   fstp     st(0)
00EB:    74 36                   je       0x123    ; Conditional jump
00ED:    DD 47 A4                fld      qword ptr [edi - 0x5c]
00F0:    D8 C0                   fadd     st(0)
00F2:    D8 C0                   fadd     st(0)
00F4:    DD 46 98                fld      qword ptr [esi - 0x68]
00F7:    90                      nop
00F8:    D9 C0                   fld      st(0)
00FA:    D8 F2                   fdiv     st(2)
00FC:    90                      nop
00FD:    D9 FC                   frndint    ; round(ST(0))
00FF:    D8 CA                   fmul     st(2)
0101:    90                      nop
0102:    DE E9                   fsubp    st(1)
0104:    DD D9                   fstp     st(1)
0106:    D9 E0                   fchs    ; -ST(0)
0108:    D9 C0                   fld      st(0)
010A:    D9 E0                   fchs    ; -ST(0)
010C:    DC 47 A4                fadd     qword ptr [edi - 0x5c]
010F:    D9 E1                   fabs    ; |ST(0)|
0111:    D9 C1                   fld      st(1)
0113:    D9 E0                   fchs    ; -ST(0)
0115:    DC 67 A4                fsub     qword ptr [edi - 0x5c]
0118:    D9 E1                   fabs    ; |ST(0)|
011A:    D9 E0                   fchs    ; -ST(0)
011C:    DE C1                   faddp    st(1)
011E:    DE C1                   faddp    st(1)
0120:    DD 5E 98                fstp     qword ptr [esi - 0x68]
0123:    90                      nop
0124:    C3                      ret
