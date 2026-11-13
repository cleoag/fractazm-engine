; Disassembly: _gnarl3Dfast.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 235 bytes, decoded: 235 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    83 C4 E0                add      esp, -0x20
0011:    DD 01                   fld      qword ptr [ecx]
0013:    DD 47 C8                fld      qword ptr [edi - 0x38]
0016:    DE C9                   fmulp    st(1)
0018:    E8 9F 00 00 00          call     0xbc
001D:    DD 01                   fld      qword ptr [ecx]
001F:    DE C1                   faddp    st(1)
0021:    DD 47 D0                fld      qword ptr [edi - 0x30]
0024:    DE C9                   fmulp    st(1)
0026:    E8 91 00 00 00          call     0xbc
002B:    DD 01                   fld      qword ptr [ecx]
002D:    DE C1                   faddp    st(1)
002F:    E8 88 00 00 00          call     0xbc
0034:    DD 47 E8                fld      qword ptr [edi - 0x18]
0037:    DE C9                   fmulp    st(1)
0039:    D9 E0                   fchs    ; -ST(0)
003B:    DD 03                   fld      qword ptr [ebx]
003D:    DE C1                   faddp    st(1)
003F:    DD 47 F0                fld      qword ptr [edi - 0x10]
0042:    DE C9                   fmulp    st(1)
0044:    DD 03                   fld      qword ptr [ebx]
0046:    DD 47 C8                fld      qword ptr [edi - 0x38]
0049:    DE C9                   fmulp    st(1)
004B:    E8 6C 00 00 00          call     0xbc
0050:    DD 03                   fld      qword ptr [ebx]
0052:    DE C1                   faddp    st(1)
0054:    DD 47 D0                fld      qword ptr [edi - 0x30]
0057:    DE C9                   fmulp    st(1)
0059:    E8 5E 00 00 00          call     0xbc
005E:    DD 03                   fld      qword ptr [ebx]
0060:    DE C1                   faddp    st(1)
0062:    E8 55 00 00 00          call     0xbc
0067:    DD 47 E0                fld      qword ptr [edi - 0x20]
006A:    DE C9                   fmulp    st(1)
006C:    D9 E0                   fchs    ; -ST(0)
006E:    DD 02                   fld      qword ptr [edx]
0070:    DE C1                   faddp    st(1)
0072:    DD 47 F0                fld      qword ptr [edi - 0x10]
0075:    DE C9                   fmulp    st(1)
0077:    DD 02                   fld      qword ptr [edx]
0079:    DD 47 C8                fld      qword ptr [edi - 0x38]
007C:    DE C9                   fmulp    st(1)
007E:    E8 39 00 00 00          call     0xbc
0083:    DD 02                   fld      qword ptr [edx]
0085:    DE C1                   faddp    st(1)
0087:    DD 47 D0                fld      qword ptr [edi - 0x30]
008A:    DE C9                   fmulp    st(1)
008C:    E8 2B 00 00 00          call     0xbc
0091:    DD 02                   fld      qword ptr [edx]
0093:    DE C1                   faddp    st(1)
0095:    E8 22 00 00 00          call     0xbc
009A:    DD 47 D8                fld      qword ptr [edi - 0x28]
009D:    DE C9                   fmulp    st(1)
009F:    D9 E0                   fchs    ; -ST(0)
00A1:    DD 01                   fld      qword ptr [ecx]
00A3:    DE C1                   faddp    st(1)
00A5:    DD 47 F0                fld      qword ptr [edi - 0x10]
00A8:    DE C9                   fmulp    st(1)
00AA:    DD 19                   fstp     qword ptr [ecx]
00AC:    DD 1A                   fstp     qword ptr [edx]
00AE:    DD 1B                   fstp     qword ptr [ebx]
00B0:    83 C4 20                add      esp, 0x20
00B3:    89 D8                   mov      eax, ebx
00B5:    5B                      pop      ebx
00B6:    5F                      pop      edi
00B7:    5E                      pop      esi
00B8:    5D                      pop      ebp
00B9:    C2 08 00                ret      8
00BC:    DC 0F                   fmul     qword ptr [edi]
00BE:    DB 54 24 18             fist     dword ptr [esp + 0x18]
00C2:    DB 44 24 18             fild     dword ptr [esp + 0x18]
00C6:    DE E9                   fsubp    st(1)
00C8:    D9 C0                   fld      st(0)
00CA:    D8 C8                   fmul     st(0)
00CC:    DD 47 20                fld      qword ptr [edi + 0x20]
00CF:    D8 C9                   fmul     st(1)
00D1:    DC 47 18                fadd     qword ptr [edi + 0x18]
00D4:    D8 C9                   fmul     st(1)
00D6:    DC 47 10                fadd     qword ptr [edi + 0x10]
00D9:    DE C9                   fmulp    st(1)
00DB:    DC 47 08                fadd     qword ptr [edi + 8]
00DE:    DE C9                   fmulp    st(1)
00E0:    8A 44 24 18             mov      al, byte ptr [esp + 0x18]
00E4:    24 01                   and      al, 1
00E6:    74 02                   je       0xea    ; Conditional jump
00E8:    D9 E0                   fchs    ; -ST(0)
00EA:    C3                      ret
