; Disassembly: _gnarl2Dfast.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 182 bytes, decoded: 182 bytes (100.0%)
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
0011:    DD 02                   fld      qword ptr [edx]
0013:    DD 47 D0                fld      qword ptr [edi - 0x30]
0016:    DE C9                   fmulp    st(1)
0018:    E8 6A 00 00 00          call     0x87
001D:    DD 02                   fld      qword ptr [edx]
001F:    DE C1                   faddp    st(1)
0021:    DD 47 D8                fld      qword ptr [edi - 0x28]
0024:    DE C9                   fmulp    st(1)
0026:    E8 5C 00 00 00          call     0x87
002B:    DD 02                   fld      qword ptr [edx]
002D:    DE C1                   faddp    st(1)
002F:    E8 53 00 00 00          call     0x87
0034:    DD 47 E8                fld      qword ptr [edi - 0x18]
0037:    DE C9                   fmulp    st(1)
0039:    D9 E0                   fchs    ; -ST(0)
003B:    DD 03                   fld      qword ptr [ebx]
003D:    DE C1                   faddp    st(1)
003F:    DD 47 F0                fld      qword ptr [edi - 0x10]
0042:    DE C9                   fmulp    st(1)
0044:    DD 03                   fld      qword ptr [ebx]
0046:    DD 47 D0                fld      qword ptr [edi - 0x30]
0049:    DE C9                   fmulp    st(1)
004B:    E8 37 00 00 00          call     0x87
0050:    DD 03                   fld      qword ptr [ebx]
0052:    DE C1                   faddp    st(1)
0054:    DD 47 D8                fld      qword ptr [edi - 0x28]
0057:    DE C9                   fmulp    st(1)
0059:    E8 29 00 00 00          call     0x87
005E:    DD 03                   fld      qword ptr [ebx]
0060:    DE C1                   faddp    st(1)
0062:    E8 20 00 00 00          call     0x87
0067:    DD 47 E0                fld      qword ptr [edi - 0x20]
006A:    DE C9                   fmulp    st(1)
006C:    D9 E0                   fchs    ; -ST(0)
006E:    DD 02                   fld      qword ptr [edx]
0070:    DE C1                   faddp    st(1)
0072:    DD 47 F0                fld      qword ptr [edi - 0x10]
0075:    DE C9                   fmulp    st(1)
0077:    DD 1A                   fstp     qword ptr [edx]
0079:    DD 1B                   fstp     qword ptr [ebx]
007B:    83 C4 20                add      esp, 0x20
007E:    89 D8                   mov      eax, ebx
0080:    5B                      pop      ebx
0081:    5F                      pop      edi
0082:    5E                      pop      esi
0083:    5D                      pop      ebp
0084:    C2 08 00                ret      8
0087:    DC 0F                   fmul     qword ptr [edi]
0089:    DB 54 24 18             fist     dword ptr [esp + 0x18]
008D:    DB 44 24 18             fild     dword ptr [esp + 0x18]
0091:    DE E9                   fsubp    st(1)
0093:    D9 C0                   fld      st(0)
0095:    D8 C8                   fmul     st(0)
0097:    DD 47 20                fld      qword ptr [edi + 0x20]
009A:    D8 C9                   fmul     st(1)
009C:    DC 47 18                fadd     qword ptr [edi + 0x18]
009F:    D8 C9                   fmul     st(1)
00A1:    DC 47 10                fadd     qword ptr [edi + 0x10]
00A4:    DE C9                   fmulp    st(1)
00A6:    DC 47 08                fadd     qword ptr [edi + 8]
00A9:    DE C9                   fmulp    st(1)
00AB:    8A 44 24 18             mov      al, byte ptr [esp + 0x18]
00AF:    24 01                   and      al, 1
00B1:    74 02                   je       0xb5    ; Conditional jump
00B3:    D9 E0                   fchs    ; -ST(0)
00B5:    C3                      ret
