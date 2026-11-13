; Disassembly: transLogSpIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 240 bytes, decoded: 240 bytes (100.0%)
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
0054:    83 C4 D8                add      esp, -0x28
0057:    DD 46 88                fld      qword ptr [esi - 0x78]
005A:    D8 C8                   fmul     st(0)
005C:    DD 46 90                fld      qword ptr [esi - 0x70]
005F:    D8 C8                   fmul     st(0)
0061:    DE C1                   faddp    st(1)
0063:    D9 FA                   fsqrt    ; sqrt(ST(0))
0065:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
0068:    DD 54 24 10             fst      qword ptr [esp + 0x10]
006C:    DD 47 AC                fld      qword ptr [edi - 0x54]
006F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0071:    D9 F1                   fyl2x
0073:    DC 0F                   fmul     qword ptr [edi]
0075:    DD 1C 24                fstp     qword ptr [esp]
0078:    DD 46 90                fld      qword ptr [esi - 0x70]
007B:    DD 46 88                fld      qword ptr [esi - 0x78]
007E:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0080:    DC 0F                   fmul     qword ptr [edi]
0082:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0086:    DB 47 B8                fild     dword ptr [edi - 0x48]
0089:    DC 0C 24                fmul     qword ptr [esp]
008C:    DB 47 B4                fild     dword ptr [edi - 0x4c]
008F:    DC 4C 24 08             fmul     qword ptr [esp + 8]
0093:    DE C1                   faddp    st(1)
0095:    D9 C0                   fld      st(0)
0097:    D9 FC                   frndint    ; round(ST(0))
0099:    DE E1                   fsubrp   st(1)
009B:    DC 4F 08                fmul     qword ptr [edi + 8]
009E:    DD 5E 88                fstp     qword ptr [esi - 0x78]
00A1:    DB 47 B4                fild     dword ptr [edi - 0x4c]
00A4:    DC 0C 24                fmul     qword ptr [esp]
00A7:    DB 47 B8                fild     dword ptr [edi - 0x48]
00AA:    DC 4C 24 08             fmul     qword ptr [esp + 8]
00AE:    D9 E0                   fchs    ; -ST(0)
00B0:    DE C1                   faddp    st(1)
00B2:    D9 C0                   fld      st(0)
00B4:    D9 FC                   frndint    ; round(ST(0))
00B6:    DE E1                   fsubrp   st(1)
00B8:    DC 4F 08                fmul     qword ptr [edi + 8]
00BB:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00BE:    DD 46 70                fld      qword ptr [esi + 0x70]
00C1:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00C5:    D9 FA                   fsqrt    ; sqrt(ST(0))
00C7:    90                      nop
00C8:    90                      nop
00C9:    90                      nop
00CA:    DC 47 10                fadd     qword ptr [edi + 0x10]
00CD:    DE F1                   fdivrp   st(1)
00CF:    DB 47 B4                fild     dword ptr [edi - 0x4c]
00D2:    D8 C8                   fmul     st(0)
00D4:    DB 47 B8                fild     dword ptr [edi - 0x48]
00D7:    D8 C8                   fmul     st(0)
00D9:    DE C1                   faddp    st(1)
00DB:    DD 47 AC                fld      qword ptr [edi - 0x54]
00DE:    D8 C8                   fmul     st(0)
00E0:    DE C1                   faddp    st(1)
00E2:    D9 FA                   fsqrt    ; sqrt(ST(0))
00E4:    DE C9                   fmulp    st(1)
00E6:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00E9:    DD 5E 70                fstp     qword ptr [esi + 0x70]
00EC:    83 C4 28                add      esp, 0x28
00EF:    C3                      ret
