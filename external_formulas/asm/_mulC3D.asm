; Disassembly: _mulC3D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 296 bytes, decoded: 296 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 48 00 00 00       sub      esp, 0x48
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    90                      nop
000D:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0010:    8B D8                   mov      ebx, eax
0012:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0015:    DD 03                   fld      qword ptr [ebx]
0017:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
001A:    90                      nop
001B:    DD 02                   fld      qword ptr [edx]
001D:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0020:    90                      nop
0021:    DD 01                   fld      qword ptr [ecx]
0023:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0026:    DD 46 28                fld      qword ptr [esi + 0x28]
0029:    DC 4F F0                fmul     qword ptr [edi - 0x10]
002C:    DD 45 E0                fld      qword ptr [ebp - 0x20]
002F:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0032:    DE C1                   faddp    st(1)
0034:    D9 D0                   fnop
0036:    DD 46 20                fld      qword ptr [esi + 0x20]
0039:    DC 4F F0                fmul     qword ptr [edi - 0x10]
003C:    DD 45 E8                fld      qword ptr [ebp - 0x18]
003F:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0042:    DE C1                   faddp    st(1)
0044:    D9 D0                   fnop
0046:    DD 46 18                fld      qword ptr [esi + 0x18]
0049:    DC 4F F0                fmul     qword ptr [edi - 0x10]
004C:    DD 45 F0                fld      qword ptr [ebp - 0x10]
004F:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0052:    DE C1                   faddp    st(1)
0054:    D9 D0                   fnop
0056:    D9 C0                   fld      st(0)
0058:    D8 4F B4                fmul     dword ptr [edi - 0x4c]
005B:    D9 C2                   fld      st(2)
005D:    D8 4F B0                fmul     dword ptr [edi - 0x50]
0060:    DE C1                   faddp    st(1)
0062:    D9 C3                   fld      st(3)
0064:    D8 4F AC                fmul     dword ptr [edi - 0x54]
0067:    DE C1                   faddp    st(1)
0069:    D9 C1                   fld      st(1)
006B:    D8 4F CC                fmul     dword ptr [edi - 0x34]
006E:    D9 C3                   fld      st(3)
0070:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0073:    DE C1                   faddp    st(1)
0075:    D9 C4                   fld      st(4)
0077:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
007A:    DE C1                   faddp    st(1)
007C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007E:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0081:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0083:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0086:    DE C3                   faddp    st(3)
0088:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008A:    D8 4F B8                fmul     dword ptr [edi - 0x48]
008D:    DE C2                   faddp    st(2)
008F:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0092:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0095:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0098:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
009B:    DC 47 D0                fadd     qword ptr [edi - 0x30]
009E:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
00A1:    90                      nop
00A2:    90                      nop
00A3:    DD 03                   fld      qword ptr [ebx]
00A5:    D8 C8                   fmul     st(0)
00A7:    DD 02                   fld      qword ptr [edx]
00A9:    D8 C8                   fmul     st(0)
00AB:    DE C1                   faddp    st(1)
00AD:    D9 FA                   fsqrt    ; sqrt(ST(0))
00AF:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00B2:    90                      nop
00B3:    DD 45 C8                fld      qword ptr [ebp - 0x38]
00B6:    D8 C8                   fmul     st(0)
00B8:    DD 45 C0                fld      qword ptr [ebp - 0x40]
00BB:    D8 C8                   fmul     st(0)
00BD:    DE C1                   faddp    st(1)
00BF:    D9 FA                   fsqrt    ; sqrt(ST(0))
00C1:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00C4:    90                      nop
00C5:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00C8:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
00CB:    DD 45 D0                fld      qword ptr [ebp - 0x30]
00CE:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00D1:    DE C1                   faddp    st(1)
00D3:    DC 4F A4                fmul     qword ptr [edi - 0x5c]
00D6:    DD 19                   fstp     qword ptr [ecx]
00D8:    90                      nop
00D9:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00DC:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
00DF:    DC 75 D8                fdiv     qword ptr [ebp - 0x28]
00E2:    DC 75 D0                fdiv     qword ptr [ebp - 0x30]
00E5:    D9 E0                   fchs    ; -ST(0)
00E7:    D9 E8                   fld1    ; Load 1.0
00E9:    DE C1                   faddp    st(1)
00EB:    DD 55 D8                fst      qword ptr [ebp - 0x28]
00EE:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00F1:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
00F4:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00F7:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
00FA:    DE E9                   fsubp    st(1)
00FC:    DE C9                   fmulp    st(1)
00FE:    90                      nop
00FF:    90                      nop
0100:    90                      nop
0101:    DD 1B                   fstp     qword ptr [ebx]
0103:    90                      nop
0104:    DD 45 C8                fld      qword ptr [ebp - 0x38]
0107:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
010A:    DD 45 F0                fld      qword ptr [ebp - 0x10]
010D:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0110:    DE C1                   faddp    st(1)
0112:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0115:    90                      nop
0116:    90                      nop
0117:    90                      nop
0118:    DD 1A                   fstp     qword ptr [edx]
011A:    90                      nop
011B:    D9 D0                   fnop
011D:    8B C3                   mov      eax, ebx
011F:    5F                      pop      edi
0120:    5E                      pop      esi
0121:    5B                      pop      ebx
0122:    89 EC                   mov      esp, ebp
0124:    5D                      pop      ebp
0125:    C2 08 00                ret      8
