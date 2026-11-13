; Disassembly: HeightMapIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 209 bytes, decoded: 209 bytes (100.0%)
; DEoption: 20

0000:    51                      push     ecx
0001:    83 C4 E0                add      esp, -0x20
0004:    DD 46 98                fld      qword ptr [esi - 0x68]
0007:    DD 46 90                fld      qword ptr [esi - 0x70]
000A:    DD 46 88                fld      qword ptr [esi - 0x78]
000D:    D9 C0                   fld      st(0)
000F:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0012:    D9 C2                   fld      st(2)
0014:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0017:    DE C1                   faddp    st(1)
0019:    D9 C3                   fld      st(3)
001B:    D8 4F B8                fmul     dword ptr [edi - 0x48]
001E:    DE C1                   faddp    st(1)
0020:    D9 C1                   fld      st(1)
0022:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0025:    D9 C3                   fld      st(3)
0027:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
002A:    DE C1                   faddp    st(1)
002C:    D9 C4                   fld      st(4)
002E:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0031:    DE C1                   faddp    st(1)
0033:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0035:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0038:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
003A:    D8 4F C8                fmul     dword ptr [edi - 0x38]
003D:    DE C3                   faddp    st(3)
003F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0041:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0044:    DE C2                   faddp    st(2)
0046:    D9 C2                   fld      st(2)
0048:    D8 47 E8                fadd     dword ptr [edi - 0x18]
004B:    D9 47 DC                fld      dword ptr [edi - 0x24]
004E:    DC 4F F8                fmul     qword ptr [edi - 8]
0051:    DE E9                   fsubp    st(1)
0053:    D9 E1                   fabs    ; |ST(0)|
0055:    D9 47 DC                fld      dword ptr [edi - 0x24]
0058:    D9 E1                   fabs    ; |ST(0)|
005A:    DC 86 78 FF FF FF       fadd     qword ptr [esi - 0x88]
0060:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0062:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0064:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0066:    73 15                   jae      0x7d    ; Conditional jump
0068:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
006A:    DD 47 F8                fld      qword ptr [edi - 8]
006D:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0070:    D8 67 E8                fsub     dword ptr [edi - 0x18]
0073:    DE E1                   fsubrp   st(1)
0075:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0078:    83 C4 20                add      esp, 0x20
007B:    59                      pop      ecx
007C:    C3                      ret
007D:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
0080:    D8 67 F0                fsub     dword ptr [edi - 0x10]
0083:    DD 1C 24                fstp     qword ptr [esp]
0086:    D8 4F E0                fmul     dword ptr [edi - 0x20]
0089:    D8 67 EC                fsub     dword ptr [edi - 0x14]
008C:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0090:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0094:    89 E0                   mov      eax, esp
0096:    8B 57 F4                mov      edx, dword ptr [edi - 0xc]
0099:    89 E1                   mov      ecx, esp
009B:    FF 96 0C 01 00 00       call     dword ptr [esi + 0x10c]
00A1:    8B 4F B4                mov      ecx, dword ptr [edi - 0x4c]
00A4:    83 E1 03                and      ecx, 3
00A7:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00AA:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00AD:    D8 67 E8                fsub     dword ptr [edi - 0x18]
00B0:    DC 64 24 18             fsub     qword ptr [esp + 0x18]
00B4:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00B7:    8B 4F B0                mov      ecx, dword ptr [edi - 0x50]
00BA:    83 E1 03                and      ecx, 3
00BD:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00C0:    D8 47 AC                fadd     dword ptr [edi - 0x54]
00C3:    D8 4F A8                fmul     dword ptr [edi - 0x58]
00C6:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00CC:    83 C4 20                add      esp, 0x20
00CF:    59                      pop      ecx
00D0:    C3                      ret
