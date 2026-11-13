; Disassembly: SphereHMVoid.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 315 bytes, decoded: 315 bytes (100.0%)
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
003C:    D8 4F E0                fmul     dword ptr [edi - 0x20]
003F:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0041:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0044:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0046:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0049:    DE C3                   faddp    st(3)
004B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004D:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0050:    DE C2                   faddp    st(2)
0052:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0054:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0057:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0059:    D9 C2                   fld      st(2)
005B:    D8 C8                   fmul     st(0)
005D:    D9 C2                   fld      st(2)
005F:    D8 C8                   fmul     st(0)
0061:    DE C1                   faddp    st(1)
0063:    D9 C1                   fld      st(1)
0065:    D8 C8                   fmul     st(0)
0067:    DE C1                   faddp    st(1)
0069:    D9 FA                   fsqrt    ; sqrt(ST(0))
006B:    D9 E8                   fld1    ; Load 1.0
006D:    DE E9                   fsubp    st(1)
006F:    D9 C0                   fld      st(0)
0071:    D9 E1                   fabs    ; |ST(0)|
0073:    D9 47 D8                fld      dword ptr [edi - 0x28]
0076:    D9 E1                   fabs    ; |ST(0)|
0078:    DC 86 78 FF FF FF       fadd     qword ptr [esi - 0x88]
007E:    DC 47 08                fadd     qword ptr [edi + 8]
0081:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0083:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0085:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0087:    73 12                   jae      0x9b    ; Conditional jump
0089:    D9 E1                   fabs    ; |ST(0)|
008B:    DD 5E E0                fstp     qword ptr [esi - 0x20]
008E:    DD D8                   fstp     st(0)
0090:    DD D8                   fstp     st(0)
0092:    DD D8                   fstp     st(0)
0094:    90                      nop
0095:    90                      nop
0096:    E9 9B 00 00 00          jmp      0x136
009B:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
009F:    DD 1C 24                fstp     qword ptr [esp]
00A2:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00A6:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00AA:    DD 04 24                fld      qword ptr [esp]
00AD:    D9 E1                   fabs    ; |ST(0)|
00AF:    DD 44 24 08             fld      qword ptr [esp + 8]
00B3:    D9 E1                   fabs    ; |ST(0)|
00B5:    D8 D1                   fcom     st(1)
00B7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B9:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00BB:    72 04                   jb       0xc1    ; Conditional jump
00BD:    DD D9                   fstp     st(1)
00BF:    EB 02                   jmp      0xc3
00C1:    DD D8                   fstp     st(0)
00C3:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00C7:    D9 E1                   fabs    ; |ST(0)|
00C9:    D8 D1                   fcom     st(1)
00CB:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CD:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00CF:    72 04                   jb       0xd5    ; Conditional jump
00D1:    DD D9                   fstp     st(1)
00D3:    EB 02                   jmp      0xd7
00D5:    DD D8                   fstp     st(0)
00D7:    D9 E8                   fld1    ; Load 1.0
00D9:    DE E9                   fsubp    st(1)
00DB:    DC 4F A0                fmul     qword ptr [edi - 0x60]
00DE:    D9 E8                   fld1    ; Load 1.0
00E0:    DC 67 A0                fsub     qword ptr [edi - 0x60]
00E3:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
00E7:    90                      nop
00E8:    90                      nop
00E9:    90                      nop
00EA:    90                      nop
00EB:    90                      nop
00EC:    DE C1                   faddp    st(1)
00EE:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00F2:    90                      nop
00F3:    90                      nop
00F4:    90                      nop
00F5:    90                      nop
00F6:    90                      nop
00F7:    90                      nop
00F8:    90                      nop
00F9:    89 E0                   mov      eax, esp
00FB:    8B 57 F4                mov      edx, dword ptr [edi - 0xc]
00FE:    89 E1                   mov      ecx, esp
0100:    FF 96 08 01 00 00       call     dword ptr [esi + 0x108]
0106:    8B 4F F0                mov      ecx, dword ptr [edi - 0x10]
0109:    83 E1 03                and      ecx, 3
010C:    DD 04 CC                fld      qword ptr [esp + ecx*8]
010F:    DC 67 F8                fsub     qword ptr [edi - 8]
0112:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0115:    DC 6C 24 18             fsubr    qword ptr [esp + 0x18]
0119:    D9 E1                   fabs    ; |ST(0)|
011B:    DC 27                   fsub     qword ptr [edi]
011D:    90                      nop
011E:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0121:    8B 4F B0                mov      ecx, dword ptr [edi - 0x50]
0124:    83 E1 03                and      ecx, 3
0127:    DD 04 CC                fld      qword ptr [esp + ecx*8]
012A:    D8 47 AC                fadd     dword ptr [edi - 0x54]
012D:    D8 4F A8                fmul     dword ptr [edi - 0x58]
0130:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0136:    83 C4 20                add      esp, 0x20
0139:    59                      pop      ecx
013A:    C3                      ret
