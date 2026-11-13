; Disassembly: Menger1-IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 164 bytes, decoded: 164 bytes (100.0%)
; DEoption: 21

0000:    DD 46 98                fld      qword ptr [esi - 0x68]
0003:    D9 E1                   fabs    ; |ST(0)|
0005:    DD 46 90                fld      qword ptr [esi - 0x70]
0008:    D9 E1                   fabs    ; |ST(0)|
000A:    DD 46 88                fld      qword ptr [esi - 0x78]
000D:    D9 E1                   fabs    ; |ST(0)|
000F:    D8 D1                   fcom     st(1)
0011:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0013:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0015:    73 02                   jae      0x19    ; Conditional jump
0017:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0019:    D8 D2                   fcom     st(2)
001B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
001D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
001F:    73 02                   jae      0x23    ; Conditional jump
0021:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0023:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0025:    D8 D2                   fcom     st(2)
0027:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0029:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
002B:    73 02                   jae      0x2f    ; Conditional jump
002D:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
002F:    D9 C1                   fld      st(1)
0031:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0034:    D9 C1                   fld      st(1)
0036:    D8 4F B8                fmul     dword ptr [edi - 0x48]
0039:    DE C1                   faddp    st(1)
003B:    D9 C3                   fld      st(3)
003D:    D8 4F B4                fmul     dword ptr [edi - 0x4c]
0040:    DE C1                   faddp    st(1)
0042:    D9 C2                   fld      st(2)
0044:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0047:    D9 C2                   fld      st(2)
0049:    D8 4F D0                fmul     dword ptr [edi - 0x30]
004C:    DE C1                   faddp    st(1)
004E:    D9 C4                   fld      st(4)
0050:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0053:    DE C1                   faddp    st(1)
0055:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0057:    D8 4F C8                fmul     dword ptr [edi - 0x38]
005A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
005C:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
005F:    DE C2                   faddp    st(2)
0061:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0063:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0066:    DE C1                   faddp    st(1)
0068:    DD 47 F0                fld      qword ptr [edi - 0x10]
006B:    DD 46 70                fld      qword ptr [esi + 0x70]
006E:    D8 C9                   fmul     st(1)
0070:    DD 5E 70                fstp     qword ptr [esi + 0x70]
0073:    DC CA                   fmul     st(2), st(0)
0075:    DC C9                   fmul     st(1), st(0)
0077:    D9 C0                   fld      st(0)
0079:    D9 E8                   fld1    ; Load 1.0
007B:    DE E9                   fsubp    st(1)
007D:    D9 C0                   fld      st(0)
007F:    D9 C0                   fld      st(0)
0081:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0084:    DE ED                   fsubp    st(5)
0086:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0089:    DE EB                   fsubp    st(3)
008B:    D8 F1                   fdiv     st(1)
008D:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0090:    DC EC                   fsub     st(4), st(0)
0092:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
0094:    D9 E1                   fabs    ; |ST(0)|
0096:    DE EC                   fsubp    st(4)
0098:    DE CB                   fmulp    st(3)
009A:    DD 5E 90                fstp     qword ptr [esi - 0x70]
009D:    DD 5E 88                fstp     qword ptr [esi - 0x78]
00A0:    DD 5E 98                fstp     qword ptr [esi - 0x68]
00A3:    C3                      ret
