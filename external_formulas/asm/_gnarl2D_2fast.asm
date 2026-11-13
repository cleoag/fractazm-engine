; Disassembly: _gnarl2D_2fast.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 174 bytes, decoded: 174 bytes (100.0%)
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
0011:    DD 03                   fld      qword ptr [ebx]
0013:    D9 C0                   fld      st(0)
0015:    E8 62 00 00 00          call     0x7c
001A:    DC 4F D0                fmul     qword ptr [edi - 0x30]
001D:    DE C1                   faddp    st(1)
001F:    E8 5B 00 00 00          call     0x7f
0024:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0027:    DD 02                   fld      qword ptr [edx]
0029:    DC 03                   fadd     qword ptr [ebx]
002B:    DC 4F C8                fmul     qword ptr [edi - 0x38]
002E:    DE E1                   fsubrp   st(1)
0030:    E8 4A 00 00 00          call     0x7f
0035:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0038:    DC 02                   fadd     qword ptr [edx]
003A:    90                      nop
003B:    DD 02                   fld      qword ptr [edx]
003D:    D9 C0                   fld      st(0)
003F:    E8 38 00 00 00          call     0x7c
0044:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0047:    DE C1                   faddp    st(1)
0049:    DC 4F D8                fmul     qword ptr [edi - 0x28]
004C:    E8 2E 00 00 00          call     0x7f
0051:    DD 02                   fld      qword ptr [edx]
0053:    DC 2B                   fsubr    qword ptr [ebx]
0055:    DC 4F C8                fmul     qword ptr [edi - 0x38]
0058:    DE C1                   faddp    st(1)
005A:    E8 20 00 00 00          call     0x7f
005F:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0062:    DC 03                   fadd     qword ptr [ebx]
0064:    90                      nop
0065:    DD 47 F0                fld      qword ptr [edi - 0x10]
0068:    DC C9                   fmul     st(1), st(0)
006A:    DE CA                   fmulp    st(2)
006C:    DD 1A                   fstp     qword ptr [edx]
006E:    DD 1B                   fstp     qword ptr [ebx]
0070:    83 C4 20                add      esp, 0x20
0073:    89 D8                   mov      eax, ebx
0075:    5B                      pop      ebx
0076:    5F                      pop      edi
0077:    5E                      pop      esi
0078:    5D                      pop      ebp
0079:    C2 08 00                ret      8
007C:    DC 47 28                fadd     qword ptr [edi + 0x28]
007F:    DC 0F                   fmul     qword ptr [edi]
0081:    DB 54 24 18             fist     dword ptr [esp + 0x18]
0085:    DB 44 24 18             fild     dword ptr [esp + 0x18]
0089:    DE E9                   fsubp    st(1)
008B:    D9 C0                   fld      st(0)
008D:    D8 C8                   fmul     st(0)
008F:    DD 47 20                fld      qword ptr [edi + 0x20]
0092:    D8 C9                   fmul     st(1)
0094:    DC 47 18                fadd     qword ptr [edi + 0x18]
0097:    D8 C9                   fmul     st(1)
0099:    DC 47 10                fadd     qword ptr [edi + 0x10]
009C:    DE C9                   fmulp    st(1)
009E:    DC 47 08                fadd     qword ptr [edi + 8]
00A1:    DE C9                   fmulp    st(1)
00A3:    8A 44 24 18             mov      al, byte ptr [esp + 0x18]
00A7:    24 01                   and      al, 1
00A9:    74 02                   je       0xad    ; Conditional jump
00AB:    D9 E0                   fchs    ; -ST(0)
00AD:    C3                      ret
