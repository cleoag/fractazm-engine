; Disassembly: _QuadrayTransform2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 156 bytes, decoded: 156 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 47 F0                fld      qword ptr [edi - 0x10]
000E:    DD 01                   fld      qword ptr [ecx]
0010:    DD 02                   fld      qword ptr [edx]
0012:    DD 00                   fld      qword ptr [eax]
0014:    D9 C0                   fld      st(0)
0016:    D8 C2                   fadd     st(2)
0018:    D8 C3                   fadd     st(3)
001A:    D9 E1                   fabs    ; |ST(0)|
001C:    DC 4F E8                fmul     qword ptr [edi - 0x18]
001F:    D8 C4                   fadd     st(4)
0021:    DD 18                   fstp     qword ptr [eax]
0023:    D9 C2                   fld      st(2)
0025:    D8 E1                   fsub     st(1)
0027:    D8 E2                   fsub     st(2)
0029:    D9 E1                   fabs    ; |ST(0)|
002B:    DC 4F E8                fmul     qword ptr [edi - 0x18]
002E:    D8 C4                   fadd     st(4)
0030:    DD 1A                   fstp     qword ptr [edx]
0032:    D9 C1                   fld      st(1)
0034:    D8 E1                   fsub     st(1)
0036:    D8 E3                   fsub     st(3)
0038:    D9 E1                   fabs    ; |ST(0)|
003A:    DC 4F E8                fmul     qword ptr [edi - 0x18]
003D:    D8 C4                   fadd     st(4)
003F:    DD 19                   fstp     qword ptr [ecx]
0041:    DE E1                   fsubrp   st(1)
0043:    DE E1                   fsubrp   st(1)
0045:    D9 E1                   fabs    ; |ST(0)|
0047:    DC 4F E8                fmul     qword ptr [edi - 0x18]
004A:    D8 C1                   fadd     st(1)
004C:    DD 59 08                fstp     qword ptr [ecx + 8]
004F:    DD 46 28                fld      qword ptr [esi + 0x28]
0052:    DD 46 20                fld      qword ptr [esi + 0x20]
0055:    DD 46 18                fld      qword ptr [esi + 0x18]
0058:    D9 C0                   fld      st(0)
005A:    D8 C2                   fadd     st(2)
005C:    D8 C3                   fadd     st(3)
005E:    D9 E1                   fabs    ; |ST(0)|
0060:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0063:    D8 C4                   fadd     st(4)
0065:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0068:    D9 C2                   fld      st(2)
006A:    D8 E1                   fsub     st(1)
006C:    D8 E2                   fsub     st(2)
006E:    D9 E1                   fabs    ; |ST(0)|
0070:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0073:    D8 C4                   fadd     st(4)
0075:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0078:    D9 C1                   fld      st(1)
007A:    D8 E1                   fsub     st(1)
007C:    D8 E3                   fsub     st(3)
007E:    D9 E1                   fabs    ; |ST(0)|
0080:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0083:    D8 C4                   fadd     st(4)
0085:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0088:    DE E1                   fsubrp   st(1)
008A:    DE E1                   fsubrp   st(1)
008C:    D9 E1                   fabs    ; |ST(0)|
008E:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0091:    DE C1                   faddp    st(1)
0093:    DD 5E C8                fstp     qword ptr [esi - 0x38]
0096:    5F                      pop      edi
0097:    5E                      pop      esi
0098:    5D                      pop      ebp
0099:    C2 08 00                ret      8
