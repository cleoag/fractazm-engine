; Disassembly: totoricalIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 248 bytes, decoded: 248 bytes (100.0%)
; DEoption: 21

0000:    DD 46 98                fld      qword ptr [esi - 0x68]
0003:    D8 C8                   fmul     st(0)
0005:    DD 46 90                fld      qword ptr [esi - 0x70]
0008:    D8 C8                   fmul     st(0)
000A:    DE C1                   faddp    st(1)
000C:    D9 FA                   fsqrt    ; sqrt(ST(0))
000E:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0011:    90                      nop
0012:    D9 C0                   fld      st(0)
0014:    D8 C8                   fmul     st(0)
0016:    DD 46 88                fld      qword ptr [esi - 0x78]
0019:    D8 C8                   fmul     st(0)
001B:    DE C1                   faddp    st(1)
001D:    D9 FA                   fsqrt    ; sqrt(ST(0))
001F:    90                      nop
0020:    D9 C1                   fld      st(1)
0022:    DD 46 88                fld      qword ptr [esi - 0x78]
0025:    D9 E0                   fchs    ; -ST(0)
0027:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0029:    DC 27                   fsub     qword ptr [edi]
002B:    D9 E1                   fabs    ; |ST(0)|
002D:    DD 46 90                fld      qword ptr [esi - 0x70]
0030:    DD 46 98                fld      qword ptr [esi - 0x68]
0033:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0035:    D9 E1                   fabs    ; |ST(0)|
0037:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
003B:    0F 85 A9 00 00 00       jne      0xea    ; Conditional jump
0041:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0044:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0047:    90                      nop
0048:    DD 47 B4                fld      qword ptr [edi - 0x4c]
004B:    D9 E4                   ftst
004D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004F:    D9 D0                   fnop
0051:    9E                      sahf
0052:    DD D8                   fstp     st(0)
0054:    74 36                   je       0x8c    ; Conditional jump
0056:    DD 5E 88                fstp     qword ptr [esi - 0x78]
0059:    DD 47 B4                fld      qword ptr [edi - 0x4c]
005C:    D8 C0                   fadd     st(0)
005E:    D8 C0                   fadd     st(0)
0060:    DD 46 88                fld      qword ptr [esi - 0x78]
0063:    90                      nop
0064:    D9 C0                   fld      st(0)
0066:    D8 F2                   fdiv     st(2)
0068:    90                      nop
0069:    D9 FC                   frndint    ; round(ST(0))
006B:    D8 CA                   fmul     st(2)
006D:    90                      nop
006E:    DE E9                   fsubp    st(1)
0070:    DD D9                   fstp     st(1)
0072:    D9 E0                   fchs    ; -ST(0)
0074:    D9 C0                   fld      st(0)
0076:    D9 E0                   fchs    ; -ST(0)
0078:    DC 47 B4                fadd     qword ptr [edi - 0x4c]
007B:    D9 E1                   fabs    ; |ST(0)|
007D:    D9 C1                   fld      st(1)
007F:    D9 E0                   fchs    ; -ST(0)
0081:    DC 67 B4                fsub     qword ptr [edi - 0x4c]
0084:    D9 E1                   fabs    ; |ST(0)|
0086:    D9 E0                   fchs    ; -ST(0)
0088:    DE C1                   faddp    st(1)
008A:    DE C1                   faddp    st(1)
008C:    DD 5E 88                fstp     qword ptr [esi - 0x78]
008F:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0092:    DC 67 D0                fsub     qword ptr [edi - 0x30]
0095:    90                      nop
0096:    DD 47 B4                fld      qword ptr [edi - 0x4c]
0099:    D9 E4                   ftst
009B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009D:    D9 D0                   fnop
009F:    9E                      sahf
00A0:    DD D8                   fstp     st(0)
00A2:    74 36                   je       0xda    ; Conditional jump
00A4:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00A7:    DD 47 B4                fld      qword ptr [edi - 0x4c]
00AA:    D8 C0                   fadd     st(0)
00AC:    D8 C0                   fadd     st(0)
00AE:    DD 46 90                fld      qword ptr [esi - 0x70]
00B1:    90                      nop
00B2:    D9 C0                   fld      st(0)
00B4:    D8 F2                   fdiv     st(2)
00B6:    90                      nop
00B7:    D9 FC                   frndint    ; round(ST(0))
00B9:    D8 CA                   fmul     st(2)
00BB:    90                      nop
00BC:    DE E9                   fsubp    st(1)
00BE:    DD D9                   fstp     st(1)
00C0:    D9 E0                   fchs    ; -ST(0)
00C2:    D9 C0                   fld      st(0)
00C4:    D9 E0                   fchs    ; -ST(0)
00C6:    DC 47 B4                fadd     qword ptr [edi - 0x4c]
00C9:    D9 E1                   fabs    ; |ST(0)|
00CB:    D9 C1                   fld      st(1)
00CD:    D9 E0                   fchs    ; -ST(0)
00CF:    DC 67 B4                fsub     qword ptr [edi - 0x4c]
00D2:    D9 E1                   fabs    ; |ST(0)|
00D4:    D9 E0                   fchs    ; -ST(0)
00D6:    DE C1                   faddp    st(1)
00D8:    DE C1                   faddp    st(1)
00DA:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00DD:    DC 4F C8                fmul     qword ptr [edi - 0x38]
00E0:    DC 67 C0                fsub     qword ptr [edi - 0x40]
00E3:    DD 5E 98                fstp     qword ptr [esi - 0x68]
00E6:    DD D8                   fstp     st(0)
00E8:    C3                      ret
00E9:    90                      nop
00EA:    DD 47 08                fld      qword ptr [edi + 8]
00ED:    DC C9                   fmul     st(1), st(0)
00EF:    DC CA                   fmul     st(2), st(0)
00F1:    DD D8                   fstp     st(0)
00F3:    E9 49 FF FF FF          jmp      0x41
