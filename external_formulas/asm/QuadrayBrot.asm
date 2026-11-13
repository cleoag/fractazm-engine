; Disassembly: QuadrayBrot.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 169 bytes, decoded: 169 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    90                      nop
0009:    90                      nop
000A:    90                      nop
000B:    DD 46 18                fld      qword ptr [esi + 0x18]
000E:    DC 46 20                fadd     qword ptr [esi + 0x20]
0011:    DC 46 28                fadd     qword ptr [esi + 0x28]
0014:    D9 E1                   fabs    ; |ST(0)|
0016:    DD 00                   fld      qword ptr [eax]
0018:    D8 C8                   fmul     st(0)
001A:    DE C1                   faddp    st(1)
001C:    DD 02                   fld      qword ptr [edx]
001E:    D8 C8                   fmul     st(0)
0020:    DE E9                   fsubp    st(1)
0022:    DD 41 08                fld      qword ptr [ecx + 8]
0025:    DC 09                   fmul     qword ptr [ecx]
0027:    D8 C0                   fadd     st(0)
0029:    DE C1                   faddp    st(1)
002B:    90                      nop
002C:    DD 46 18                fld      qword ptr [esi + 0x18]
002F:    D9 E0                   fchs    ; -ST(0)
0031:    DC 66 20                fsub     qword ptr [esi + 0x20]
0034:    DC 46 28                fadd     qword ptr [esi + 0x28]
0037:    D9 E1                   fabs    ; |ST(0)|
0039:    DD 02                   fld      qword ptr [edx]
003B:    D8 C8                   fmul     st(0)
003D:    DE C1                   faddp    st(1)
003F:    DD 00                   fld      qword ptr [eax]
0041:    D8 C8                   fmul     st(0)
0043:    DE E9                   fsubp    st(1)
0045:    DD 41 08                fld      qword ptr [ecx + 8]
0048:    DC 09                   fmul     qword ptr [ecx]
004A:    D8 C0                   fadd     st(0)
004C:    DE C1                   faddp    st(1)
004E:    90                      nop
004F:    DD 46 18                fld      qword ptr [esi + 0x18]
0052:    D9 E0                   fchs    ; -ST(0)
0054:    DC 46 20                fadd     qword ptr [esi + 0x20]
0057:    DC 66 28                fsub     qword ptr [esi + 0x28]
005A:    D9 E1                   fabs    ; |ST(0)|
005C:    DD 01                   fld      qword ptr [ecx]
005E:    D8 C8                   fmul     st(0)
0060:    DE C1                   faddp    st(1)
0062:    DD 41 08                fld      qword ptr [ecx + 8]
0065:    D8 C8                   fmul     st(0)
0067:    DE E9                   fsubp    st(1)
0069:    DD 00                   fld      qword ptr [eax]
006B:    90                      nop
006C:    DC 0A                   fmul     qword ptr [edx]
006E:    D8 C0                   fadd     st(0)
0070:    DE C1                   faddp    st(1)
0072:    90                      nop
0073:    90                      nop
0074:    DD 46 18                fld      qword ptr [esi + 0x18]
0077:    DC 66 20                fsub     qword ptr [esi + 0x20]
007A:    DC 66 28                fsub     qword ptr [esi + 0x28]
007D:    D9 E1                   fabs    ; |ST(0)|
007F:    DD 01                   fld      qword ptr [ecx]
0081:    D8 C8                   fmul     st(0)
0083:    DE E9                   fsubp    st(1)
0085:    DD 41 08                fld      qword ptr [ecx + 8]
0088:    D8 C8                   fmul     st(0)
008A:    DE C1                   faddp    st(1)
008C:    DD 00                   fld      qword ptr [eax]
008E:    90                      nop
008F:    DC 0A                   fmul     qword ptr [edx]
0091:    D8 C0                   fadd     st(0)
0093:    DE C1                   faddp    st(1)
0095:    90                      nop
0096:    DD 59 08                fstp     qword ptr [ecx + 8]
0099:    DD 19                   fstp     qword ptr [ecx]
009B:    DD 1A                   fstp     qword ptr [edx]
009D:    DD 18                   fstp     qword ptr [eax]
009F:    90                      nop
00A0:    5F                      pop      edi
00A1:    5E                      pop      esi
00A2:    90                      nop
00A3:    8B E5                   mov      esp, ebp
00A5:    5D                      pop      ebp
00A6:    C2 08 00                ret      8
