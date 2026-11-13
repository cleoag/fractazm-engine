; Disassembly: foldinghexIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 255 bytes, decoded: 255 bytes (100.0%)
; DEoption: 21

0000:    83 C4 E0                add      esp, -0x20
0003:    DD 46 90                fld      qword ptr [esi - 0x70]
0006:    DD 46 88                fld      qword ptr [esi - 0x78]
0009:    DD 1C 24                fstp     qword ptr [esp]
000C:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0010:    DD 04 24                fld      qword ptr [esp]
0013:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0016:    D9 E1                   fabs    ; |ST(0)|
0018:    DD 1C 24                fstp     qword ptr [esp]
001B:    DD 04 24                fld      qword ptr [esp]
001E:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
0021:    DC 0F                   fmul     qword ptr [edi]
0023:    DC 27                   fsub     qword ptr [edi]
0025:    D9 FC                   frndint    ; round(ST(0))
0027:    D8 C0                   fadd     st(0)
0029:    D9 E8                   fld1    ; Load 1.0
002B:    DE C1                   faddp    st(1)
002D:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0030:    D9 E0                   fchs    ; -ST(0)
0032:    DC 04 24                fadd     qword ptr [esp]
0035:    D9 E1                   fabs    ; |ST(0)|
0037:    DD 1C 24                fstp     qword ptr [esp]
003A:    DD 47 F0                fld      qword ptr [edi - 0x10]
003D:    DC 4F 08                fmul     qword ptr [edi + 8]
0040:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0044:    DD 44 24 08             fld      qword ptr [esp + 8]
0048:    DC 64 24 18             fsub     qword ptr [esp + 0x18]
004C:    D9 E1                   fabs    ; |ST(0)|
004E:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0052:    DD 44 24 08             fld      qword ptr [esp + 8]
0056:    DC 74 24 18             fdiv     qword ptr [esp + 0x18]
005A:    DC 0F                   fmul     qword ptr [edi]
005C:    DC 27                   fsub     qword ptr [edi]
005E:    D9 FC                   frndint    ; round(ST(0))
0060:    D8 C0                   fadd     st(0)
0062:    D9 E8                   fld1    ; Load 1.0
0064:    DE C1                   faddp    st(1)
0066:    DC 4C 24 18             fmul     qword ptr [esp + 0x18]
006A:    D9 E0                   fchs    ; -ST(0)
006C:    DC 44 24 08             fadd     qword ptr [esp + 8]
0070:    D9 E1                   fabs    ; |ST(0)|
0072:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0076:    DD 04 24                fld      qword ptr [esp]
0079:    DC 4F 10                fmul     qword ptr [edi + 0x10]
007C:    DD 44 24 08             fld      qword ptr [esp + 8]
0080:    DC 4F 18                fmul     qword ptr [edi + 0x18]
0083:    DE C1                   faddp    st(1)
0085:    DC 47 20                fadd     qword ptr [edi + 0x20]
0088:    D9 C0                   fld      st(0)
008A:    D9 E1                   fabs    ; |ST(0)|
008C:    D9 E0                   fchs    ; -ST(0)
008E:    DE C1                   faddp    st(1)
0090:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0094:    D9 C0                   fld      st(0)
0096:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0099:    D9 E0                   fchs    ; -ST(0)
009B:    DC 04 24                fadd     qword ptr [esp]
009E:    D9 E1                   fabs    ; |ST(0)|
00A0:    DD 1C 24                fstp     qword ptr [esp]
00A3:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00A6:    D9 E0                   fchs    ; -ST(0)
00A8:    DC 44 24 08             fadd     qword ptr [esp + 8]
00AC:    D9 E1                   fabs    ; |ST(0)|
00AE:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00B2:    DD 04 24                fld      qword ptr [esp]
00B5:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00B8:    DD 44 24 08             fld      qword ptr [esp + 8]
00BC:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00BF:    DE C1                   faddp    st(1)
00C1:    DC 47 20                fadd     qword ptr [edi + 0x20]
00C4:    D9 C0                   fld      st(0)
00C6:    D9 E1                   fabs    ; |ST(0)|
00C8:    D9 E0                   fchs    ; -ST(0)
00CA:    DE C1                   faddp    st(1)
00CC:    DD 54 24 18             fst      qword ptr [esp + 0x18]
00D0:    D9 C0                   fld      st(0)
00D2:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00D5:    D9 E0                   fchs    ; -ST(0)
00D7:    DC 04 24                fadd     qword ptr [esp]
00DA:    90                      nop
00DB:    90                      nop
00DC:    DD 1C 24                fstp     qword ptr [esp]
00DF:    DC 4F 18                fmul     qword ptr [edi + 0x18]
00E2:    D9 E0                   fchs    ; -ST(0)
00E4:    DC 44 24 08             fadd     qword ptr [esp + 8]
00E8:    90                      nop
00E9:    90                      nop
00EA:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00EE:    DD 04 24                fld      qword ptr [esp]
00F1:    DD 44 24 08             fld      qword ptr [esp + 8]
00F5:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00F8:    DD 5E 88                fstp     qword ptr [esi - 0x78]
00FB:    83 C4 20                add      esp, 0x20
00FE:    C3                      ret
