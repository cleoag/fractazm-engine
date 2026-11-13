; Disassembly: TrifoxComplexAngles.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 203 bytes, decoded: 203 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    D9 E8                   fld1    ; Load 1.0
000D:    8B 47 F4                mov      eax, dword ptr [edi - 0xc]
0010:    DD 46 38                fld      qword ptr [esi + 0x38]
0013:    D9 FA                   fsqrt    ; sqrt(ST(0))
0015:    EB 02                   jmp      0x19
0017:    D8 C8                   fmul     st(0)
0019:    D1 E8                   shr      eax, 1
001B:    73 FA                   jae      0x17    ; Conditional jump
001D:    DC C9                   fmul     st(1), st(0)
001F:    75 F6                   jne      0x17    ; Conditional jump
0021:    DD D8                   fstp     st(0)
0023:    D9 EE                   fldz    ; Load 0.0
0025:    D9 E8                   fld1    ; Load 1.0
0027:    DD 42 F8                fld      qword ptr [edx - 8]
002A:    DD 02                   fld      qword ptr [edx]
002C:    DD 01                   fld      qword ptr [ecx]
002E:    D9 C2                   fld      st(2)
0030:    D8 C8                   fmul     st(0)
0032:    D9 C2                   fld      st(2)
0034:    D8 C8                   fmul     st(0)
0036:    DE C1                   faddp    st(1)
0038:    D9 FA                   fsqrt    ; sqrt(ST(0))
003A:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
003C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003E:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0040:    D9 47 F0                fld      dword ptr [edi - 0x10]
0043:    DC CA                   fmul     st(2), st(0)
0045:    DE C9                   fmulp    st(1)
0047:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
004B:    74 02                   je       0x4f    ; Conditional jump
004D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
004F:    83 7F E0 00             cmp      dword ptr [edi - 0x20], 0
0053:    74 15                   je       0x6a    ; Conditional jump
0055:    D9 C1                   fld      st(1)
0057:    D8 C8                   fmul     st(0)
0059:    D9 C1                   fld      st(1)
005B:    D8 C8                   fmul     st(0)
005D:    DE C1                   faddp    st(1)
005F:    D8 47 DC                fadd     dword ptr [edi - 0x24]
0062:    D9 E8                   fld1    ; Load 1.0
0064:    DE F1                   fdivrp   st(1)
0066:    DC CA                   fmul     st(2), st(0)
0068:    DE C9                   fmulp    st(1)
006A:    8B 47 E4                mov      eax, dword ptr [edi - 0x1c]
006D:    EB 10                   jmp      0x7f
006F:    D9 C1                   fld      st(1)
0071:    D8 C9                   fmul     st(1)
0073:    D8 C0                   fadd     st(0)
0075:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0077:    D8 C8                   fmul     st(0)
0079:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
007B:    D8 C8                   fmul     st(0)
007D:    DE E1                   fsubrp   st(1)
007F:    D1 E8                   shr      eax, 1
0081:    73 EC                   jae      0x6f    ; Conditional jump
0083:    D9 C2                   fld      st(2)
0085:    D8 C9                   fmul     st(1)
0087:    D9 C4                   fld      st(4)
0089:    D8 CB                   fmul     st(3)
008B:    DE E9                   fsubp    st(1)
008D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008F:    D8 CA                   fmul     st(2)
0091:    D9 C1                   fld      st(1)
0093:    DE CD                   fmulp    st(5)
0095:    DE C4                   faddp    st(4)
0097:    75 D6                   jne      0x6f    ; Conditional jump
0099:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
009B:    D9 47 EC                fld      dword ptr [edi - 0x14]
009E:    DC CA                   fmul     st(2), st(0)
00A0:    DE C9                   fmulp    st(1)
00A2:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00A4:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00A6:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00A8:    D8 CB                   fmul     st(3)
00AA:    D8 CC                   fmul     st(4)
00AC:    DC 46 18                fadd     qword ptr [esi + 0x18]
00AF:    DD 5A F8                fstp     qword ptr [edx - 8]
00B2:    DE CA                   fmulp    st(2)
00B4:    D8 CA                   fmul     st(2)
00B6:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00B9:    DC 46 28                fadd     qword ptr [esi + 0x28]
00BC:    DD 19                   fstp     qword ptr [ecx]
00BE:    DE C9                   fmulp    st(1)
00C0:    DC 46 20                fadd     qword ptr [esi + 0x20]
00C3:    DD 1A                   fstp     qword ptr [edx]
00C5:    5F                      pop      edi
00C6:    5E                      pop      esi
00C7:    5D                      pop      ebp
00C8:    C2 08 00                ret      8
