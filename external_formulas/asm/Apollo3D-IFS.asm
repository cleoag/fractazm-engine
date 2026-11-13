; Disassembly: Apollo3D-IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 304 bytes, decoded: 304 bytes (100.0%)
; DEoption: 21

0000:    83 C4 C0                add      esp, -0x40
0003:    DD 46 90                fld      qword ptr [esi - 0x70]
0006:    DC 4F B0                fmul     qword ptr [edi - 0x50]
0009:    DD 46 88                fld      qword ptr [esi - 0x78]
000C:    DC 4F B0                fmul     qword ptr [edi - 0x50]
000F:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0012:    D9 C1                   fld      st(1)
0014:    D9 C1                   fld      st(1)
0016:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0018:    D9 E0                   fchs    ; -ST(0)
001A:    90                      nop
001B:    DC 0F                   fmul     qword ptr [edi]
001D:    DA 4F B8                fimul    dword ptr [edi - 0x48]
0020:    D9 FC                   frndint    ; round(ST(0))
0022:    DB 14 24                fist     dword ptr [esp]
0025:    8B 14 24                mov      edx, dword ptr [esp]
0028:    DA 77 B8                fidiv    dword ptr [edi - 0x48]
002B:    DC 4F 08                fmul     qword ptr [edi + 8]
002E:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0030:    D9 C3                   fld      st(3)
0032:    D8 C9                   fmul     st(1)
0034:    D9 C3                   fld      st(3)
0036:    D8 CB                   fmul     st(3)
0038:    DE C1                   faddp    st(1)
003A:    83 E2 01                and      edx, 1
003D:    75 03                   jne      0x42    ; Conditional jump
003F:    D9 E0                   fchs    ; -ST(0)
0041:    90                      nop
0042:    DD 5E 90                fstp     qword ptr [esi - 0x70]
0045:    D9 C2                   fld      st(2)
0047:    D8 C9                   fmul     st(1)
0049:    D9 C4                   fld      st(4)
004B:    D8 CB                   fmul     st(3)
004D:    DE E9                   fsubp    st(1)
004F:    90                      nop
0050:    90                      nop
0051:    DD 5E 88                fstp     qword ptr [esi - 0x78]
0054:    DD D8                   fstp     st(0)
0056:    DD D8                   fstp     st(0)
0058:    DD D8                   fstp     st(0)
005A:    DD D8                   fstp     st(0)
005C:    DD 46 98                fld      qword ptr [esi - 0x68]
005F:    DD 46 90                fld      qword ptr [esi - 0x70]
0062:    DD 46 88                fld      qword ptr [esi - 0x78]
0065:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0068:    D9 C0                   fld      st(0)
006A:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
006D:    D9 C2                   fld      st(2)
006F:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0072:    DE C1                   faddp    st(1)
0074:    D9 C3                   fld      st(3)
0076:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0079:    DE C1                   faddp    st(1)
007B:    D9 C1                   fld      st(1)
007D:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0080:    D9 C3                   fld      st(3)
0082:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0085:    DE C1                   faddp    st(1)
0087:    D9 C4                   fld      st(4)
0089:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
008C:    DE C1                   faddp    st(1)
008E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0090:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0093:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0095:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0098:    DE C3                   faddp    st(3)
009A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
009C:    D8 4F C8                fmul     dword ptr [edi - 0x38]
009F:    DE C2                   faddp    st(2)
00A1:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00A4:    D9 E0                   fchs    ; -ST(0)
00A6:    DD 5E 88                fstp     qword ptr [esi - 0x78]
00A9:    DD 5E 98                fstp     qword ptr [esi - 0x68]
00AC:    DD 46 88                fld      qword ptr [esi - 0x78]
00AF:    DD 54 24 10             fst      qword ptr [esp + 0x10]
00B3:    D8 C8                   fmul     st(0)
00B5:    DD 14 24                fst      qword ptr [esp]
00B8:    DD 46 90                fld      qword ptr [esi - 0x70]
00BB:    D8 C8                   fmul     st(0)
00BD:    D9 C0                   fld      st(0)
00BF:    DC 04 24                fadd     qword ptr [esp]
00C2:    DD 1C 24                fstp     qword ptr [esp]
00C5:    DE C1                   faddp    st(1)
00C7:    D9 E8                   fld1    ; Load 1.0
00C9:    DE E9                   fsubp    st(1)
00CB:    DD 04 24                fld      qword ptr [esp]
00CE:    D9 E8                   fld1    ; Load 1.0
00D0:    DE C1                   faddp    st(1)
00D2:    DD 46 90                fld      qword ptr [esi - 0x70]
00D5:    D8 C0                   fadd     st(0)
00D7:    DE C1                   faddp    st(1)
00D9:    DD 54 24 08             fst      qword ptr [esp + 8]
00DD:    DE F9                   fdivp    st(1)
00DF:    DC 47 A8                fadd     qword ptr [edi - 0x58]
00E2:    DD 54 24 20             fst      qword ptr [esp + 0x20]
00E6:    DC 77 B0                fdiv     qword ptr [edi - 0x50]
00E9:    DD 5E 88                fstp     qword ptr [esi - 0x78]
00EC:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00F0:    D8 C0                   fadd     st(0)
00F2:    D9 E0                   fchs    ; -ST(0)
00F4:    DC 74 24 08             fdiv     qword ptr [esp + 8]
00F8:    DD 54 24 28             fst      qword ptr [esp + 0x28]
00FC:    DC 77 B0                fdiv     qword ptr [edi - 0x50]
00FF:    DD 5E 90                fstp     qword ptr [esi - 0x70]
0102:    90                      nop
0103:    DD 44 24 20             fld      qword ptr [esp + 0x20]
0107:    D8 C8                   fmul     st(0)
0109:    DD 44 24 28             fld      qword ptr [esp + 0x28]
010D:    D8 C8                   fmul     st(0)
010F:    DE C1                   faddp    st(1)
0111:    DC 47 A0                fadd     qword ptr [edi - 0x60]
0114:    D9 FA                   fsqrt    ; sqrt(ST(0))
0116:    90                      nop
0117:    DD 47 E0                fld      qword ptr [edi - 0x20]
011A:    DE F1                   fdivrp   st(1)
011C:    D9 C0                   fld      st(0)
011E:    D9 E1                   fabs    ; |ST(0)|
0120:    DC 4E 70                fmul     qword ptr [esi + 0x70]
0123:    DD 5E 70                fstp     qword ptr [esi + 0x70]
0126:    DC 4E 98                fmul     qword ptr [esi - 0x68]
0129:    DD 5E 98                fstp     qword ptr [esi - 0x68]
012C:    83 C4 40                add      esp, 0x40
012F:    C3                      ret
