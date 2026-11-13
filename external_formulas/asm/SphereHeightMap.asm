; Disassembly: SphereHeightMap.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 222 bytes, decoded: 222 bytes (100.0%)
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
0092:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0096:    DD 1C 24                fstp     qword ptr [esp]
0099:    DD 5C 24 08             fstp     qword ptr [esp + 8]
009D:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A1:    89 E0                   mov      eax, esp
00A3:    8B 57 F4                mov      edx, dword ptr [edi - 0xc]
00A6:    89 E1                   mov      ecx, esp
00A8:    FF 96 08 01 00 00       call     dword ptr [esi + 0x108]
00AE:    8B 4F F0                mov      ecx, dword ptr [edi - 0x10]
00B1:    83 E1 03                and      ecx, 3
00B4:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00B7:    DC 67 F8                fsub     qword ptr [edi - 8]
00BA:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00BD:    DC 6C 24 18             fsubr    qword ptr [esp + 0x18]
00C1:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00C4:    8B 4F B0                mov      ecx, dword ptr [edi - 0x50]
00C7:    83 E1 03                and      ecx, 3
00CA:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00CD:    D8 47 AC                fadd     dword ptr [edi - 0x54]
00D0:    D8 4F A8                fmul     dword ptr [edi - 0x58]
00D3:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00D9:    83 C4 20                add      esp, 0x20
00DC:    59                      pop      ecx
00DD:    C3                      ret
