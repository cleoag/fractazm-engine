; Disassembly: trans-qIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 198 bytes, decoded: 198 bytes (100.0%)
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
0055:    D9 E8                   fld1    ; Load 1.0
0057:    DD 46 88                fld      qword ptr [esi - 0x78]
005A:    D9 E1                   fabs    ; |ST(0)|
005C:    DE E9                   fsubp    st(1)
005E:    DD 5E 88                fstp     qword ptr [esi - 0x78]
0061:    D9 E8                   fld1    ; Load 1.0
0063:    DD 46 90                fld      qword ptr [esi - 0x70]
0066:    D9 E1                   fabs    ; |ST(0)|
0068:    DE E9                   fsubp    st(1)
006A:    DD 5E 90                fstp     qword ptr [esi - 0x70]
006D:    90                      nop
006E:    DD 46 88                fld      qword ptr [esi - 0x78]
0071:    D9 EE                   fldz    ; Load 0.0
0073:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0075:    9B                      wait
0076:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0078:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
007A:    73 1D                   jae      0x99    ; Conditional jump
007C:    DD 47 B4                fld      qword ptr [edi - 0x4c]
007F:    D9 E8                   fld1    ; Load 1.0
0081:    DE E9                   fsubp    st(1)
0083:    D9 C0                   fld      st(0)
0085:    D9 E8                   fld1    ; Load 1.0
0087:    DE E1                   fsubrp   st(1)
0089:    DC 4E 88                fmul     qword ptr [esi - 0x78]
008C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
008E:    DC 4E 88                fmul     qword ptr [esi - 0x78]
0091:    DC 4E 88                fmul     qword ptr [esi - 0x78]
0094:    DE C1                   faddp    st(1)
0096:    DD 5E 88                fstp     qword ptr [esi - 0x78]
0099:    DD 46 90                fld      qword ptr [esi - 0x70]
009C:    D9 EE                   fldz    ; Load 0.0
009E:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00A0:    9B                      wait
00A1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A5:    73 1D                   jae      0xc4    ; Conditional jump
00A7:    DD 47 AC                fld      qword ptr [edi - 0x54]
00AA:    D9 E8                   fld1    ; Load 1.0
00AC:    DE E9                   fsubp    st(1)
00AE:    D9 C0                   fld      st(0)
00B0:    D9 E8                   fld1    ; Load 1.0
00B2:    DE E1                   fsubrp   st(1)
00B4:    DC 4E 90                fmul     qword ptr [esi - 0x70]
00B7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00B9:    DC 4E 90                fmul     qword ptr [esi - 0x70]
00BC:    DC 4E 90                fmul     qword ptr [esi - 0x70]
00BF:    DE C1                   faddp    st(1)
00C1:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00C4:    C3                      ret
00C5:    90                      nop
