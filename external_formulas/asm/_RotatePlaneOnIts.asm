; Disassembly: _RotatePlaneOnIts.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 209 bytes, decoded: 209 bytes (100.0%)
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
0065:    DC 4F CC                fmul     qword ptr [edi - 0x34]
0068:    DC 0F                   fmul     qword ptr [edi]
006A:    D9 FB                   fsincos    ; ST(0)=cos, push sin
006C:    D9 C0                   fld      st(0)
006E:    D8 CB                   fmul     st(3)
0070:    D9 C2                   fld      st(2)
0072:    D8 CD                   fmul     st(5)
0074:    DE C1                   faddp    st(1)
0076:    DD 1B                   fstp     qword ptr [ebx]
0078:    DE CB                   fmulp    st(3)
007A:    DE C9                   fmulp    st(1)
007C:    DE E9                   fsubp    st(1)
007E:    DD 1A                   fstp     qword ptr [edx]
0080:    90                      nop
0081:    D9 D0                   fnop
0083:    DD 01                   fld      qword ptr [ecx]
0085:    DD 02                   fld      qword ptr [edx]
0087:    DD 03                   fld      qword ptr [ebx]
0089:    D9 C0                   fld      st(0)
008B:    D8 4F EC                fmul     dword ptr [edi - 0x14]
008E:    D9 C2                   fld      st(2)
0090:    D8 4F E0                fmul     dword ptr [edi - 0x20]
0093:    DE C1                   faddp    st(1)
0095:    D9 C3                   fld      st(3)
0097:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
009A:    DE C1                   faddp    st(1)
009C:    D9 C1                   fld      st(1)
009E:    D8 4F F4                fmul     dword ptr [edi - 0xc]
00A1:    D9 C3                   fld      st(3)
00A3:    D8 4F E8                fmul     dword ptr [edi - 0x18]
00A6:    DE C1                   faddp    st(1)
00A8:    D9 C4                   fld      st(4)
00AA:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00AD:    DE C1                   faddp    st(1)
00AF:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00B1:    D8 4F F0                fmul     dword ptr [edi - 0x10]
00B4:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00B6:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
00B9:    DE C3                   faddp    st(3)
00BB:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00BD:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00C0:    DE C2                   faddp    st(2)
00C2:    DD 1B                   fstp     qword ptr [ebx]
00C4:    DD 1A                   fstp     qword ptr [edx]
00C6:    DD 19                   fstp     qword ptr [ecx]
00C8:    89 D8                   mov      eax, ebx
00CA:    5B                      pop      ebx
00CB:    5F                      pop      edi
00CC:    5E                      pop      esi
00CD:    5D                      pop      ebp
00CE:    C2 08 00                ret      8
