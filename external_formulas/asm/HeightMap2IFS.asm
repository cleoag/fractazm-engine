; Disassembly: HeightMap2IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 269 bytes, decoded: 269 bytes (100.0%)
; DEoption: 20

0000:    51                      push     ecx
0001:    83 C4 E0                add      esp, -0x20
0004:    DD 46 98                fld      qword ptr [esi - 0x68]
0007:    DD 46 90                fld      qword ptr [esi - 0x70]
000A:    DD 46 88                fld      qword ptr [esi - 0x78]
000D:    D9 C0                   fld      st(0)
000F:    D8 C8                   fmul     st(0)
0011:    D9 C2                   fld      st(2)
0013:    D8 C8                   fmul     st(0)
0015:    DE C1                   faddp    st(1)
0017:    D9 C3                   fld      st(3)
0019:    D8 C8                   fmul     st(0)
001B:    DE C1                   faddp    st(1)
001D:    D9 E8                   fld1    ; Load 1.0
001F:    DE F1                   fdivrp   st(1)
0021:    DC 17                   fcom     qword ptr [edi]
0023:    90                      nop
0024:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0026:    80 E4 41                and      ah, 0x41
0029:    76 0F                   jbe      0x3a    ; Conditional jump
002B:    DD D8                   fstp     st(0)
002D:    8B 4F A4                mov      ecx, dword ptr [edi - 0x5c]
0030:    83 E1 01                and      ecx, 1
0033:    C1 E1 03                shl      ecx, 3
0036:    DD 04 39                fld      qword ptr [ecx + edi]
0039:    90                      nop
003A:    DC C9                   fmul     st(1), st(0)
003C:    DC CA                   fmul     st(2), st(0)
003E:    DC CB                   fmul     st(3), st(0)
0040:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0043:    D9 C0                   fld      st(0)
0045:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0048:    D9 C2                   fld      st(2)
004A:    D8 4F BC                fmul     dword ptr [edi - 0x44]
004D:    DE C1                   faddp    st(1)
004F:    D9 C3                   fld      st(3)
0051:    D8 4F B8                fmul     dword ptr [edi - 0x48]
0054:    DE C1                   faddp    st(1)
0056:    D9 C1                   fld      st(1)
0058:    D8 4F D8                fmul     dword ptr [edi - 0x28]
005B:    D9 C3                   fld      st(3)
005D:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0060:    DE C1                   faddp    st(1)
0062:    D9 C4                   fld      st(4)
0064:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0067:    DE C1                   faddp    st(1)
0069:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
006B:    D8 4F CC                fmul     dword ptr [edi - 0x34]
006E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0070:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0073:    DE C3                   faddp    st(3)
0075:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0077:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
007A:    DE C2                   faddp    st(2)
007C:    D9 C2                   fld      st(2)
007E:    D8 47 E8                fadd     dword ptr [edi - 0x18]
0081:    D9 47 DC                fld      dword ptr [edi - 0x24]
0084:    DC 4F F8                fmul     qword ptr [edi - 8]
0087:    DE E9                   fsubp    st(1)
0089:    D9 E1                   fabs    ; |ST(0)|
008B:    D9 47 DC                fld      dword ptr [edi - 0x24]
008E:    D9 E1                   fabs    ; |ST(0)|
0090:    DC 86 78 FF FF FF       fadd     qword ptr [esi - 0x88]
0096:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0098:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
009C:    73 18                   jae      0xb6    ; Conditional jump
009E:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00A0:    DD 47 F8                fld      qword ptr [edi - 8]
00A3:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00A6:    D8 67 E8                fsub     dword ptr [edi - 0x18]
00A9:    DE E1                   fsubrp   st(1)
00AB:    DC 76 E0                fdiv     qword ptr [esi - 0x20]
00AE:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00B1:    83 C4 20                add      esp, 0x20
00B4:    59                      pop      ecx
00B5:    C3                      ret
00B6:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
00B9:    D8 67 F0                fsub     dword ptr [edi - 0x10]
00BC:    DD 1C 24                fstp     qword ptr [esp]
00BF:    D8 4F E0                fmul     dword ptr [edi - 0x20]
00C2:    D8 67 EC                fsub     dword ptr [edi - 0x14]
00C5:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00C9:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00CD:    89 E0                   mov      eax, esp
00CF:    8B 57 F4                mov      edx, dword ptr [edi - 0xc]
00D2:    89 E1                   mov      ecx, esp
00D4:    FF 96 0C 01 00 00       call     dword ptr [esi + 0x10c]
00DA:    8B 4F B4                mov      ecx, dword ptr [edi - 0x4c]
00DD:    83 E1 03                and      ecx, 3
00E0:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00E3:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00E6:    D8 67 E8                fsub     dword ptr [edi - 0x18]
00E9:    DC 64 24 18             fsub     qword ptr [esp + 0x18]
00ED:    DC 76 E0                fdiv     qword ptr [esi - 0x20]
00F0:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00F3:    8B 4F B0                mov      ecx, dword ptr [edi - 0x50]
00F6:    83 E1 03                and      ecx, 3
00F9:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00FC:    D8 47 AC                fadd     dword ptr [edi - 0x54]
00FF:    D8 4F A8                fmul     dword ptr [edi - 0x58]
0102:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0108:    83 C4 20                add      esp, 0x20
010B:    59                      pop      ecx
010C:    C3                      ret
