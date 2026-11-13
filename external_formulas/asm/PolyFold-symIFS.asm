; Disassembly: PolyFold-symIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 161 bytes, decoded: 161 bytes (100.0%)
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
004B:    D9 C1                   fld      st(1)
004D:    D9 C1                   fld      st(1)
004F:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0051:    DC 6F B0                fsubr    qword ptr [edi - 0x50]
0054:    DC 0F                   fmul     qword ptr [edi]
0056:    DA 4F B8                fimul    dword ptr [edi - 0x48]
0059:    D9 FC                   frndint    ; round(ST(0))
005B:    83 C4 F0                add      esp, -0x10
005E:    90                      nop
005F:    DB 14 24                fist     dword ptr [esp]
0062:    8B 14 24                mov      edx, dword ptr [esp]
0065:    DA 77 B8                fidiv    dword ptr [edi - 0x48]
0068:    DC 4F 08                fmul     qword ptr [edi + 8]
006B:    D9 FB                   fsincos    ; ST(0)=cos, push sin
006D:    D9 C3                   fld      st(3)
006F:    D8 C9                   fmul     st(1)
0071:    D9 C3                   fld      st(3)
0073:    D8 CB                   fmul     st(3)
0075:    DE C1                   faddp    st(1)
0077:    83 E2 01                and      edx, 1
007A:    75 03                   jne      0x7f    ; Conditional jump
007C:    D9 E0                   fchs    ; -ST(0)
007E:    90                      nop
007F:    83 C4 10                add      esp, 0x10
0082:    90                      nop
0083:    DD 5E 90                fstp     qword ptr [esi - 0x70]
0086:    D9 C2                   fld      st(2)
0088:    D8 C9                   fmul     st(1)
008A:    D9 C4                   fld      st(4)
008C:    D8 CB                   fmul     st(3)
008E:    DE E9                   fsubp    st(1)
0090:    90                      nop
0091:    90                      nop
0092:    DD 5E 88                fstp     qword ptr [esi - 0x78]
0095:    DD D8                   fstp     st(0)
0097:    DD D8                   fstp     st(0)
0099:    DD D8                   fstp     st(0)
009B:    DD D8                   fstp     st(0)
009D:    DD 5E 98                fstp     qword ptr [esi - 0x68]
00A0:    C3                      ret
