; Disassembly: Fuzzy.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 129 bytes, decoded: 129 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    D9 E8                   fld1    ; Load 1.0
000D:    DA 46 40                fiadd    dword ptr [esi + 0x40]
0010:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0013:    DD 46 38                fld      qword ptr [esi + 0x38]
0016:    D9 C0                   fld      st(0)
0018:    D9 FA                   fsqrt    ; sqrt(ST(0))
001A:    DC 39                   fdivr    qword ptr [ecx]
001C:    D9 E8                   fld1    ; Load 1.0
001E:    D9 C1                   fld      st(1)
0020:    D8 C8                   fmul     st(0)
0022:    DE E9                   fsubp    st(1)
0024:    D9 FA                   fsqrt    ; sqrt(ST(0))
0026:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0028:    D8 CA                   fmul     st(2)
002A:    DD 02                   fld      qword ptr [edx]
002C:    DD 00                   fld      qword ptr [eax]
002E:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0030:    DE CB                   fmulp    st(3)
0032:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0034:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0036:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0038:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
003A:    D9 E8                   fld1    ; Load 1.0
003C:    DC 6F F0                fsubr    qword ptr [edi - 0x10]
003F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0041:    D9 ED                   fldln2    ; Load ln(2)
0043:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0045:    D9 F1                   fyl2x
0047:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0049:    DE C9                   fmulp    st(1)
004B:    D9 EA                   fldl2e    ; Load log2(e)
004D:    DE C9                   fmulp    st(1)
004F:    D9 C0                   fld      st(0)
0051:    D9 FC                   frndint    ; round(ST(0))
0053:    DC E9                   fsub     st(1), st(0)
0055:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0057:    D9 F0                   f2xm1
0059:    D9 E8                   fld1    ; Load 1.0
005B:    DE C1                   faddp    st(1)
005D:    D9 FD                   fscale
005F:    DD D9                   fstp     st(1)
0061:    DC CA                   fmul     st(2), st(0)
0063:    DE CC                   fmulp    st(4)
0065:    D8 CB                   fmul     st(3)
0067:    DC 4F E8                fmul     qword ptr [edi - 0x18]
006A:    DC 46 20                fadd     qword ptr [esi + 0x20]
006D:    DD 1A                   fstp     qword ptr [edx]
006F:    DC 6E 28                fsubr    qword ptr [esi + 0x28]
0072:    DD 19                   fstp     qword ptr [ecx]
0074:    DE C9                   fmulp    st(1)
0076:    DC 46 18                fadd     qword ptr [esi + 0x18]
0079:    DD 18                   fstp     qword ptr [eax]
007B:    5F                      pop      edi
007C:    5E                      pop      esi
007D:    5D                      pop      ebp
007E:    C2 08 00                ret      8
