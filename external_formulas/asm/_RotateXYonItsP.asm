; Disassembly: _RotateXYonItsP.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 259 bytes, decoded: 259 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 01                   fld      qword ptr [ecx]
0010:    DD 02                   fld      qword ptr [edx]
0012:    DD 03                   fld      qword ptr [ebx]
0014:    D9 C0                   fld      st(0)
0016:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0019:    D9 C2                   fld      st(2)
001B:    D8 4F D8                fmul     dword ptr [edi - 0x28]
001E:    DE C1                   faddp    st(1)
0020:    D9 C3                   fld      st(3)
0022:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0025:    DE C1                   faddp    st(1)
0027:    D9 C1                   fld      st(1)
0029:    D8 4F F4                fmul     dword ptr [edi - 0xc]
002C:    D9 C3                   fld      st(3)
002E:    D8 4F F0                fmul     dword ptr [edi - 0x10]
0031:    DE C1                   faddp    st(1)
0033:    D9 C4                   fld      st(4)
0035:    D8 4F EC                fmul     dword ptr [edi - 0x14]
0038:    DE C1                   faddp    st(1)
003A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003C:    D8 4F E8                fmul     dword ptr [edi - 0x18]
003F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0041:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
0044:    DE C3                   faddp    st(3)
0046:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0048:    D8 4F E0                fmul     dword ptr [edi - 0x20]
004B:    DE C2                   faddp    st(2)
004D:    DD 1B                   fstp     qword ptr [ebx]
004F:    DD 1A                   fstp     qword ptr [edx]
0051:    DD 19                   fstp     qword ptr [ecx]
0053:    D9 D0                   fnop
0055:    DD 02                   fld      qword ptr [edx]
0057:    DD 03                   fld      qword ptr [ebx]
0059:    FF 86 D0 00 00 00       inc      dword ptr [esi + 0xd0]
005F:    DB 86 D0 00 00 00       fild     dword ptr [esi + 0xd0]
0065:    DC 67 C4                fsub     qword ptr [edi - 0x3c]
0068:    DD 47 BC                fld      qword ptr [edi - 0x44]
006B:    E8 6D 00 00 00          call     0xdd
0070:    90                      nop
0071:    DC 4F CC                fmul     qword ptr [edi - 0x34]
0074:    DC 0F                   fmul     qword ptr [edi]
0076:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0078:    D9 C0                   fld      st(0)
007A:    D8 CB                   fmul     st(3)
007C:    D9 C2                   fld      st(2)
007E:    D8 CD                   fmul     st(5)
0080:    DE C1                   faddp    st(1)
0082:    DD 1B                   fstp     qword ptr [ebx]
0084:    DE CB                   fmulp    st(3)
0086:    DE C9                   fmulp    st(1)
0088:    DE E9                   fsubp    st(1)
008A:    DD 1A                   fstp     qword ptr [edx]
008C:    90                      nop
008D:    D9 D0                   fnop
008F:    DD 01                   fld      qword ptr [ecx]
0091:    DD 02                   fld      qword ptr [edx]
0093:    DD 03                   fld      qword ptr [ebx]
0095:    D9 C0                   fld      st(0)
0097:    D8 4F EC                fmul     dword ptr [edi - 0x14]
009A:    D9 C2                   fld      st(2)
009C:    D8 4F E0                fmul     dword ptr [edi - 0x20]
009F:    DE C1                   faddp    st(1)
00A1:    D9 C3                   fld      st(3)
00A3:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00A6:    DE C1                   faddp    st(1)
00A8:    D9 C1                   fld      st(1)
00AA:    D8 4F F4                fmul     dword ptr [edi - 0xc]
00AD:    D9 C3                   fld      st(3)
00AF:    D8 4F E8                fmul     dword ptr [edi - 0x18]
00B2:    DE C1                   faddp    st(1)
00B4:    D9 C4                   fld      st(4)
00B6:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00B9:    DE C1                   faddp    st(1)
00BB:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00BD:    D8 4F F0                fmul     dword ptr [edi - 0x10]
00C0:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00C2:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
00C5:    DE C3                   faddp    st(3)
00C7:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00C9:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00CC:    DE C2                   faddp    st(2)
00CE:    DD 1B                   fstp     qword ptr [ebx]
00D0:    DD 1A                   fstp     qword ptr [edx]
00D2:    DD 19                   fstp     qword ptr [ecx]
00D4:    89 D8                   mov      eax, ebx
00D6:    5B                      pop      ebx
00D7:    5F                      pop      edi
00D8:    5E                      pop      esi
00D9:    5D                      pop      ebp
00DA:    C2 08 00                ret      8
00DD:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00DF:    D9 E1                   fabs    ; |ST(0)|
00E1:    D9 ED                   fldln2    ; Load ln(2)
00E3:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E5:    D9 F1                   fyl2x
00E7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E9:    DE C9                   fmulp    st(1)
00EB:    D9 EA                   fldl2e    ; Load log2(e)
00ED:    DE C9                   fmulp    st(1)
00EF:    D9 C0                   fld      st(0)
00F1:    D9 FC                   frndint    ; round(ST(0))
00F3:    DC E9                   fsub     st(1), st(0)
00F5:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00F7:    D9 F0                   f2xm1
00F9:    D9 E8                   fld1    ; Load 1.0
00FB:    DE C1                   faddp    st(1)
00FD:    D9 FD                   fscale
00FF:    DD D9                   fstp     st(1)
0101:    C3                      ret
0102:    90                      nop
