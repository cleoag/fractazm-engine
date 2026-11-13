; Disassembly: _invbisp.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 168 bytes, decoded: 168 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 30 00 00 00       sub      esp, 0x30
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    90                      nop
0015:    DD 03                   fld      qword ptr [ebx]
0017:    D8 C8                   fmul     st(0)
0019:    DD 02                   fld      qword ptr [edx]
001B:    D8 C8                   fmul     st(0)
001D:    DD 01                   fld      qword ptr [ecx]
001F:    D8 C8                   fmul     st(0)
0021:    DE C1                   faddp    st(1)
0023:    DE C1                   faddp    st(1)
0025:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0028:    90                      nop
0029:    DD 47 F0                fld      qword ptr [edi - 0x10]
002C:    D8 C8                   fmul     st(0)
002E:    DE C1                   faddp    st(1)
0030:    D8 C8                   fmul     st(0)
0032:    DD 47 F0                fld      qword ptr [edi - 0x10]
0035:    DC 09                   fmul     qword ptr [ecx]
0037:    D8 C0                   fadd     st(0)
0039:    D8 C8                   fmul     st(0)
003B:    D9 E0                   fchs    ; -ST(0)
003D:    DE C1                   faddp    st(1)
003F:    D9 E1                   fabs    ; |ST(0)|
0041:    D9 FA                   fsqrt    ; sqrt(ST(0))
0043:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0046:    90                      nop
0047:    DD 45 F0                fld      qword ptr [ebp - 0x10]
004A:    DD 47 F0                fld      qword ptr [edi - 0x10]
004D:    D8 C8                   fmul     st(0)
004F:    D9 E0                   fchs    ; -ST(0)
0051:    DE C1                   faddp    st(1)
0053:    DD 47 F0                fld      qword ptr [edi - 0x10]
0056:    DC 09                   fmul     qword ptr [ecx]
0058:    D8 C0                   fadd     st(0)
005A:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
005D:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0060:    90                      nop
0061:    DD 02                   fld      qword ptr [edx]
0063:    DD 03                   fld      qword ptr [ebx]
0065:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0067:    DD 19                   fstp     qword ptr [ecx]
0069:    90                      nop
006A:    DD 45 E8                fld      qword ptr [ebp - 0x18]
006D:    D8 C8                   fmul     st(0)
006F:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0072:    D8 C8                   fmul     st(0)
0074:    DE E9                   fsubp    st(1)
0076:    D9 FA                   fsqrt    ; sqrt(ST(0))
0078:    DD 45 F0                fld      qword ptr [ebp - 0x10]
007B:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
007D:    DC 2F                   fsubr    qword ptr [edi]
007F:    DD 1B                   fstp     qword ptr [ebx]
0081:    90                      nop
0082:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0085:    DC 75 E8                fdiv     qword ptr [ebp - 0x18]
0088:    D9 C0                   fld      st(0)
008A:    D8 C8                   fmul     st(0)
008C:    D9 E8                   fld1    ; Load 1.0
008E:    DE C1                   faddp    st(1)
0090:    D9 FA                   fsqrt    ; sqrt(ST(0))
0092:    DE C1                   faddp    st(1)
0094:    D9 EA                   fldl2e    ; Load log2(e)
0096:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0098:    D9 F1                   fyl2x
009A:    DD 1A                   fstp     qword ptr [edx]
009C:    90                      nop
009D:    8B C3                   mov      eax, ebx
009F:    5F                      pop      edi
00A0:    5E                      pop      esi
00A1:    5B                      pop      ebx
00A2:    8B E5                   mov      esp, ebp
00A4:    5D                      pop      ebp
00A5:    C2 08 00                ret      8
