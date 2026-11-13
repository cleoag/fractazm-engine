; Disassembly: CylinderHMVoid.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 248 bytes, decoded: 248 bytes (100.0%)
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
005B:    D9 C3                   fld      st(3)
005D:    D8 4F A4                fmul     dword ptr [edi - 0x5c]
0060:    DE C1                   faddp    st(1)
0062:    D9 54 24 1C             fst      dword ptr [esp + 0x1c]
0066:    D8 67 DC                fsub     dword ptr [edi - 0x24]
0069:    D9 C3                   fld      st(3)
006B:    D9 E1                   fabs    ; |ST(0)|
006D:    D8 67 E0                fsub     dword ptr [edi - 0x20]
0070:    D8 D1                   fcom     st(1)
0072:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0074:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0076:    72 02                   jb       0x7a    ; Conditional jump
0078:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
007A:    DD D8                   fstp     st(0)
007C:    D9 C0                   fld      st(0)
007E:    D9 E1                   fabs    ; |ST(0)|
0080:    D9 47 D8                fld      dword ptr [edi - 0x28]
0083:    D9 E1                   fabs    ; |ST(0)|
0085:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0087:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0089:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
008B:    73 0D                   jae      0x9a    ; Conditional jump
008D:    D9 E1                   fabs    ; |ST(0)|
008F:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0092:    DD D8                   fstp     st(0)
0094:    DD D8                   fstp     st(0)
0096:    DD D8                   fstp     st(0)
0098:    EB 59                   jmp      0xf3
009A:    D9 5C 24 18             fstp     dword ptr [esp + 0x18]
009E:    D9 C1                   fld      st(1)
00A0:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00A2:    DC 0F                   fmul     qword ptr [edi]
00A4:    DD 1C 24                fstp     qword ptr [esp]
00A7:    DD D8                   fstp     st(0)
00A9:    DC 0F                   fmul     qword ptr [edi]
00AB:    D8 77 DC                fdiv     dword ptr [edi - 0x24]
00AE:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00B2:    89 E0                   mov      eax, esp
00B4:    8B 57 F4                mov      edx, dword ptr [edi - 0xc]
00B7:    89 E1                   mov      ecx, esp
00B9:    FF 96 0C 01 00 00       call     dword ptr [esi + 0x10c]
00BF:    8B 4F F0                mov      ecx, dword ptr [edi - 0x10]
00C2:    83 E1 03                and      ecx, 3
00C5:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00C8:    DC 67 F8                fsub     qword ptr [edi - 8]
00CB:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00CE:    D8 4C 24 1C             fmul     dword ptr [esp + 0x1c]
00D2:    D8 6C 24 18             fsubr    dword ptr [esp + 0x18]
00D6:    D9 E1                   fabs    ; |ST(0)|
00D8:    DC 67 08                fsub     qword ptr [edi + 8]
00DB:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00DE:    8B 4F B0                mov      ecx, dword ptr [edi - 0x50]
00E1:    83 E1 03                and      ecx, 3
00E4:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00E7:    D8 47 AC                fadd     dword ptr [edi - 0x54]
00EA:    D8 4F A8                fmul     dword ptr [edi - 0x58]
00ED:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00F3:    83 C4 20                add      esp, 0x20
00F6:    59                      pop      ecx
00F7:    C3                      ret
