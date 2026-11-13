; Disassembly: _helispiral.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 252 bytes, decoded: 252 bytes (100.0%)
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
0059:    83 7F BC 01             cmp      dword ptr [edi - 0x44], 1
005D:    75 0A                   jne      0x69    ; Conditional jump
005F:    DD 46 18                fld      qword ptr [esi + 0x18]
0062:    D8 C8                   fmul     st(0)
0064:    DD 46 20                fld      qword ptr [esi + 0x20]
0067:    EB 06                   jmp      0x6f
0069:    D9 C0                   fld      st(0)
006B:    D8 C8                   fmul     st(0)
006D:    DD 02                   fld      qword ptr [edx]
006F:    D8 C8                   fmul     st(0)
0071:    DE C1                   faddp    st(1)
0073:    D9 FA                   fsqrt    ; sqrt(ST(0))
0075:    DD 47 C4                fld      qword ptr [edi - 0x3c]
0078:    DE C9                   fmulp    st(1)
007A:    83 7F C0 01             cmp      dword ptr [edi - 0x40], 1
007E:    75 05                   jne      0x85    ; Conditional jump
0080:    DD 46 28                fld      qword ptr [esi + 0x28]
0083:    EB 02                   jmp      0x87
0085:    DD 01                   fld      qword ptr [ecx]
0087:    DD 47 CC                fld      qword ptr [edi - 0x34]
008A:    DE C9                   fmulp    st(1)
008C:    DE C1                   faddp    st(1)
008E:    DD 47 B4                fld      qword ptr [edi - 0x4c]
0091:    DC 0F                   fmul     qword ptr [edi]
0093:    90                      nop
0094:    DE C1                   faddp    st(1)
0096:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0098:    D9 C0                   fld      st(0)
009A:    D8 CB                   fmul     st(3)
009C:    D9 C2                   fld      st(2)
009E:    D8 CD                   fmul     st(5)
00A0:    DE C1                   faddp    st(1)
00A2:    DD 1B                   fstp     qword ptr [ebx]
00A4:    DE CB                   fmulp    st(3)
00A6:    DE C9                   fmulp    st(1)
00A8:    DE E9                   fsubp    st(1)
00AA:    DD 1A                   fstp     qword ptr [edx]
00AC:    D9 D0                   fnop
00AE:    DD 01                   fld      qword ptr [ecx]
00B0:    DD 02                   fld      qword ptr [edx]
00B2:    DD 03                   fld      qword ptr [ebx]
00B4:    D9 C0                   fld      st(0)
00B6:    D8 4F EC                fmul     dword ptr [edi - 0x14]
00B9:    D9 C2                   fld      st(2)
00BB:    D8 4F E0                fmul     dword ptr [edi - 0x20]
00BE:    DE C1                   faddp    st(1)
00C0:    D9 C3                   fld      st(3)
00C2:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00C5:    DE C1                   faddp    st(1)
00C7:    D9 C1                   fld      st(1)
00C9:    D8 4F F4                fmul     dword ptr [edi - 0xc]
00CC:    D9 C3                   fld      st(3)
00CE:    D8 4F E8                fmul     dword ptr [edi - 0x18]
00D1:    DE C1                   faddp    st(1)
00D3:    D9 C4                   fld      st(4)
00D5:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00D8:    DE C1                   faddp    st(1)
00DA:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00DC:    D8 4F F0                fmul     dword ptr [edi - 0x10]
00DF:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E1:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
00E4:    DE C3                   faddp    st(3)
00E6:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E8:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00EB:    DE C2                   faddp    st(2)
00ED:    DD 1B                   fstp     qword ptr [ebx]
00EF:    DD 1A                   fstp     qword ptr [edx]
00F1:    DD 19                   fstp     qword ptr [ecx]
00F3:    89 D8                   mov      eax, ebx
00F5:    5B                      pop      ebx
00F6:    5F                      pop      edi
00F7:    5E                      pop      esi
00F8:    5D                      pop      ebp
00F9:    C2 08 00                ret      8
