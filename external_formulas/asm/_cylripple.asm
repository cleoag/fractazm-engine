; Disassembly: _cylripple.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 283 bytes, decoded: 283 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 20 00 00 00       sub      esp, 0x20
0009:    56                      push     esi
000A:    57                      push     edi
000B:    53                      push     ebx
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    89 C3                   mov      ebx, eax
0011:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0014:    DD 01                   fld      qword ptr [ecx]
0016:    DD 02                   fld      qword ptr [edx]
0018:    DD 03                   fld      qword ptr [ebx]
001A:    D9 C0                   fld      st(0)
001C:    D8 4F DC                fmul     dword ptr [edi - 0x24]
001F:    D9 C2                   fld      st(2)
0021:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0024:    DE C1                   faddp    st(1)
0026:    D9 C3                   fld      st(3)
0028:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
002B:    DE C1                   faddp    st(1)
002D:    D9 C1                   fld      st(1)
002F:    D8 4F F4                fmul     dword ptr [edi - 0xc]
0032:    D9 C3                   fld      st(3)
0034:    D8 4F F0                fmul     dword ptr [edi - 0x10]
0037:    DE C1                   faddp    st(1)
0039:    D9 C4                   fld      st(4)
003B:    D8 4F EC                fmul     dword ptr [edi - 0x14]
003E:    DE C1                   faddp    st(1)
0040:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0042:    D8 4F E8                fmul     dword ptr [edi - 0x18]
0045:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0047:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
004A:    DE C3                   faddp    st(3)
004C:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004E:    D8 4F E0                fmul     dword ptr [edi - 0x20]
0051:    DE C2                   faddp    st(2)
0053:    DD 1B                   fstp     qword ptr [ebx]
0055:    DD 1A                   fstp     qword ptr [edx]
0057:    DD 19                   fstp     qword ptr [ecx]
0059:    D9 D0                   fnop
005B:    83 7F B0 01             cmp      dword ptr [edi - 0x50], 1
005F:    75 19                   jne      0x7a    ; Conditional jump
0061:    DD 03                   fld      qword ptr [ebx]
0063:    D8 C8                   fmul     st(0)
0065:    DD 02                   fld      qword ptr [edx]
0067:    D8 C8                   fmul     st(0)
0069:    DE C1                   faddp    st(1)
006B:    D9 FA                   fsqrt    ; sqrt(ST(0))
006D:    DD 55 F8                fst      qword ptr [ebp - 8]
0070:    DC 4F CC                fmul     qword ptr [edi - 0x34]
0073:    D9 FE                   fsin    ; sin(ST(0))
0075:    DC 4F C4                fmul     qword ptr [edi - 0x3c]
0078:    EB 0A                   jmp      0x84
007A:    DD 46 18                fld      qword ptr [esi + 0x18]
007D:    D8 C8                   fmul     st(0)
007F:    DD 46 20                fld      qword ptr [esi + 0x20]
0082:    EB E3                   jmp      0x67
0084:    83 7F AC 01             cmp      dword ptr [edi - 0x54], 1
0088:    75 13                   jne      0x9d    ; Conditional jump
008A:    DD 02                   fld      qword ptr [edx]
008C:    DD 03                   fld      qword ptr [ebx]
008E:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0090:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0093:    DC 4F BC                fmul     qword ptr [edi - 0x44]
0096:    D9 FE                   fsin    ; sin(ST(0))
0098:    DC 4F B4                fmul     qword ptr [edi - 0x4c]
009B:    EB 08                   jmp      0xa5
009D:    DD 46 20                fld      qword ptr [esi + 0x20]
00A0:    DD 46 18                fld      qword ptr [esi + 0x18]
00A3:    EB E9                   jmp      0x8e
00A5:    D9 E8                   fld1    ; Load 1.0
00A7:    DE C1                   faddp    st(1)
00A9:    DE C1                   faddp    st(1)
00AB:    DC 4D F8                fmul     qword ptr [ebp - 8]
00AE:    DD 1B                   fstp     qword ptr [ebx]
00B0:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00B3:    DD 1A                   fstp     qword ptr [edx]
00B5:    DD 03                   fld      qword ptr [ebx]
00B7:    DD 02                   fld      qword ptr [edx]
00B9:    D9 FF                   fcos    ; cos(ST(0))
00BB:    DE C9                   fmulp    st(1)
00BD:    DD 03                   fld      qword ptr [ebx]
00BF:    DD 02                   fld      qword ptr [edx]
00C1:    D9 FE                   fsin    ; sin(ST(0))
00C3:    DE C9                   fmulp    st(1)
00C5:    DD 1A                   fstp     qword ptr [edx]
00C7:    DD 1B                   fstp     qword ptr [ebx]
00C9:    D9 D0                   fnop
00CB:    DD 01                   fld      qword ptr [ecx]
00CD:    DD 02                   fld      qword ptr [edx]
00CF:    DD 03                   fld      qword ptr [ebx]
00D1:    D9 C0                   fld      st(0)
00D3:    D8 4F EC                fmul     dword ptr [edi - 0x14]
00D6:    D9 C2                   fld      st(2)
00D8:    D8 4F E0                fmul     dword ptr [edi - 0x20]
00DB:    DE C1                   faddp    st(1)
00DD:    D9 C3                   fld      st(3)
00DF:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00E2:    DE C1                   faddp    st(1)
00E4:    D9 C1                   fld      st(1)
00E6:    D8 4F F4                fmul     dword ptr [edi - 0xc]
00E9:    D9 C3                   fld      st(3)
00EB:    D8 4F E8                fmul     dword ptr [edi - 0x18]
00EE:    DE C1                   faddp    st(1)
00F0:    D9 C4                   fld      st(4)
00F2:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00F5:    DE C1                   faddp    st(1)
00F7:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00F9:    D8 4F F0                fmul     dword ptr [edi - 0x10]
00FC:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00FE:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
0101:    DE C3                   faddp    st(3)
0103:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0105:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0108:    DE C2                   faddp    st(2)
010A:    DD 1B                   fstp     qword ptr [ebx]
010C:    DD 1A                   fstp     qword ptr [edx]
010E:    DD 19                   fstp     qword ptr [ecx]
0110:    89 D8                   mov      eax, ebx
0112:    5B                      pop      ebx
0113:    5F                      pop      edi
0114:    5E                      pop      esi
0115:    89 EC                   mov      esp, ebp
0117:    5D                      pop      ebp
0118:    C2 08 00                ret      8
