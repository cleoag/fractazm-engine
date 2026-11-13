; Disassembly: amazingIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 236 bytes, decoded: 236 bytes (100.0%)
; DEoption: 21

0000:    DD 47 B4                fld      qword ptr [edi - 0x4c]
0003:    DD 46 98                fld      qword ptr [esi - 0x68]
0006:    D9 C0                   fld      st(0)
0008:    D8 E2                   fsub     st(2)
000A:    D9 E1                   fabs    ; |ST(0)|
000C:    D8 C1                   fadd     st(1)
000E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0010:    D8 C2                   fadd     st(2)
0012:    D9 E1                   fabs    ; |ST(0)|
0014:    DE E1                   fsubrp   st(1)
0016:    DD 5E 98                fstp     qword ptr [esi - 0x68]
0019:    DD D8                   fstp     st(0)
001B:    DD 47 AC                fld      qword ptr [edi - 0x54]
001E:    DD 46 90                fld      qword ptr [esi - 0x70]
0021:    D9 C0                   fld      st(0)
0023:    D8 E2                   fsub     st(2)
0025:    D9 E1                   fabs    ; |ST(0)|
0027:    D8 C1                   fadd     st(1)
0029:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002B:    D8 C2                   fadd     st(2)
002D:    D9 E1                   fabs    ; |ST(0)|
002F:    DE E1                   fsubrp   st(1)
0031:    DD 5E 90                fstp     qword ptr [esi - 0x70]
0034:    DD D8                   fstp     st(0)
0036:    DD 47 A4                fld      qword ptr [edi - 0x5c]
0039:    DD 46 88                fld      qword ptr [esi - 0x78]
003C:    D9 C0                   fld      st(0)
003E:    D8 E2                   fsub     st(2)
0040:    D9 E1                   fabs    ; |ST(0)|
0042:    D8 C1                   fadd     st(1)
0044:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0046:    D8 C2                   fadd     st(2)
0048:    D9 E1                   fabs    ; |ST(0)|
004A:    DE E1                   fsubrp   st(1)
004C:    DD 5E 88                fstp     qword ptr [esi - 0x78]
004F:    DD D8                   fstp     st(0)
0051:    DD 46 98                fld      qword ptr [esi - 0x68]
0054:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0057:    DD 46 90                fld      qword ptr [esi - 0x70]
005A:    DC 47 E8                fadd     qword ptr [edi - 0x18]
005D:    DD 46 88                fld      qword ptr [esi - 0x78]
0060:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0063:    D9 C0                   fld      st(0)
0065:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0068:    D9 C2                   fld      st(2)
006A:    D8 4F C0                fmul     dword ptr [edi - 0x40]
006D:    DE C1                   faddp    st(1)
006F:    D9 C3                   fld      st(3)
0071:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0074:    DE C1                   faddp    st(1)
0076:    D9 C1                   fld      st(1)
0078:    D8 4F DC                fmul     dword ptr [edi - 0x24]
007B:    D9 C3                   fld      st(3)
007D:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0080:    DE C1                   faddp    st(1)
0082:    D9 C4                   fld      st(4)
0084:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0087:    DE C1                   faddp    st(1)
0089:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
008B:    D8 4F D0                fmul     dword ptr [edi - 0x30]
008E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0090:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0093:    DE C3                   faddp    st(3)
0095:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0097:    D8 4F C8                fmul     dword ptr [edi - 0x38]
009A:    DE C2                   faddp    st(2)
009C:    D9 C0                   fld      st(0)
009E:    D8 C8                   fmul     st(0)
00A0:    D9 C2                   fld      st(2)
00A2:    D8 C8                   fmul     st(0)
00A4:    DE C1                   faddp    st(1)
00A6:    D9 C3                   fld      st(3)
00A8:    D8 C8                   fmul     st(0)
00AA:    DE C1                   faddp    st(1)
00AC:    D9 E8                   fld1    ; Load 1.0
00AE:    DE F1                   fdivrp   st(1)
00B0:    D9 E8                   fld1    ; Load 1.0
00B2:    D8 D9                   fcomp    st(1)
00B4:    9B                      wait
00B5:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B7:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00B9:    73 13                   jae      0xce    ; Conditional jump
00BB:    DC 57 9C                fcom     qword ptr [edi - 0x64]
00BE:    9B                      wait
00BF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C1:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00C3:    76 15                   jbe      0xda    ; Conditional jump
00C5:    DD D8                   fstp     st(0)
00C7:    DD 47 9C                fld      qword ptr [edi - 0x64]
00CA:    EB 0E                   jmp      0xda
00CC:    90                      nop
00CD:    90                      nop
00CE:    DD D8                   fstp     st(0)
00D0:    DD 5E 88                fstp     qword ptr [esi - 0x78]
00D3:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00D6:    DD 5E 98                fstp     qword ptr [esi - 0x68]
00D9:    C3                      ret
00DA:    DC C9                   fmul     st(1), st(0)
00DC:    DC CA                   fmul     st(2), st(0)
00DE:    DC CB                   fmul     st(3), st(0)
00E0:    DD 46 70                fld      qword ptr [esi + 0x70]
00E3:    D8 C9                   fmul     st(1)
00E5:    D9 E1                   fabs    ; |ST(0)|
00E7:    DD 5E 70                fstp     qword ptr [esi + 0x70]
00EA:    EB E2                   jmp      0xce
