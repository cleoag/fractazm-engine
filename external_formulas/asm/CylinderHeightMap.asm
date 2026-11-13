; Disassembly: CylinderHeightMap.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 235 bytes, decoded: 235 bytes (100.0%)
; DEoption: 20

0000:    51                      push     ecx
0001:    83 C4 E0                add      esp, -0x20
0004:    DD 46 98                fld      qword ptr [esi - 0x68]
0007:    D8 67 E4                fsub     dword ptr [edi - 0x1c]
000A:    DD 46 90                fld      qword ptr [esi - 0x70]
000D:    D8 67 E8                fsub     dword ptr [edi - 0x18]
0010:    DD 46 88                fld      qword ptr [esi - 0x78]
0013:    D8 67 EC                fsub     dword ptr [edi - 0x14]
0016:    D9 C0                   fld      st(0)
0018:    D8 4F BC                fmul     dword ptr [edi - 0x44]
001B:    D9 C2                   fld      st(2)
001D:    D8 4F B8                fmul     dword ptr [edi - 0x48]
0020:    DE C1                   faddp    st(1)
0022:    D9 C3                   fld      st(3)
0024:    D8 4F B4                fmul     dword ptr [edi - 0x4c]
0027:    DE C1                   faddp    st(1)
0029:    D9 C1                   fld      st(1)
002B:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
002E:    D9 C3                   fld      st(3)
0030:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0033:    DE C1                   faddp    st(1)
0035:    D9 C4                   fld      st(4)
0037:    D8 4F CC                fmul     dword ptr [edi - 0x34]
003A:    DE C1                   faddp    st(1)
003C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003E:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0041:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0043:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0046:    DE C3                   faddp    st(3)
0048:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004A:    D8 4F C0                fmul     dword ptr [edi - 0x40]
004D:    DE C2                   faddp    st(2)
004F:    D9 C1                   fld      st(1)
0051:    D8 C8                   fmul     st(0)
0053:    D9 C1                   fld      st(1)
0055:    D8 C8                   fmul     st(0)
0057:    DE C1                   faddp    st(1)
0059:    D9 FA                   fsqrt    ; sqrt(ST(0))
005B:    D9 54 24 1C             fst      dword ptr [esp + 0x1c]
005F:    D8 67 DC                fsub     dword ptr [edi - 0x24]
0062:    D9 C3                   fld      st(3)
0064:    D9 E1                   fabs    ; |ST(0)|
0066:    D8 67 E0                fsub     dword ptr [edi - 0x20]
0069:    D8 D1                   fcom     st(1)
006B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006F:    72 02                   jb       0x73    ; Conditional jump
0071:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0073:    DD D8                   fstp     st(0)
0075:    D9 C0                   fld      st(0)
0077:    D9 E1                   fabs    ; |ST(0)|
0079:    D9 47 D8                fld      dword ptr [edi - 0x28]
007C:    D9 E1                   fabs    ; |ST(0)|
007E:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0080:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0082:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0084:    73 0C                   jae      0x92    ; Conditional jump
0086:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0089:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
008B:    DD D8                   fstp     st(0)
008D:    83 C4 20                add      esp, 0x20
0090:    59                      pop      ecx
0091:    C3                      ret
0092:    D9 5C 24 18             fstp     dword ptr [esp + 0x18]
0096:    D9 C1                   fld      st(1)
0098:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
009A:    DC 0F                   fmul     qword ptr [edi]
009C:    DD 1C 24                fstp     qword ptr [esp]
009F:    DD D8                   fstp     st(0)
00A1:    DC 0F                   fmul     qword ptr [edi]
00A3:    D8 77 DC                fdiv     dword ptr [edi - 0x24]
00A6:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00AA:    89 E0                   mov      eax, esp
00AC:    8B 57 F4                mov      edx, dword ptr [edi - 0xc]
00AF:    89 E1                   mov      ecx, esp
00B1:    FF 96 0C 01 00 00       call     dword ptr [esi + 0x10c]
00B7:    8B 4F F0                mov      ecx, dword ptr [edi - 0x10]
00BA:    83 E1 03                and      ecx, 3
00BD:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00C0:    DC 67 F8                fsub     qword ptr [edi - 8]
00C3:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00C6:    D8 4C 24 1C             fmul     dword ptr [esp + 0x1c]
00CA:    D8 6C 24 18             fsubr    dword ptr [esp + 0x18]
00CE:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00D1:    8B 4F B0                mov      ecx, dword ptr [edi - 0x50]
00D4:    83 E1 03                and      ecx, 3
00D7:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00DA:    D8 47 AC                fadd     dword ptr [edi - 0x54]
00DD:    D8 4F A8                fmul     dword ptr [edi - 0x58]
00E0:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00E6:    83 C4 20                add      esp, 0x20
00E9:    59                      pop      ecx
00EA:    C3                      ret
