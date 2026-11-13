; Disassembly: _dynamicfast.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 190 bytes, decoded: 190 bytes (100.0%)
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
0013:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0016:    E8 71 00 00 00          call     0x8c
001B:    DC 4F E8                fmul     qword ptr [edi - 0x18]
001E:    DD 01                   fld      qword ptr [ecx]
0020:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0023:    E8 67 00 00 00          call     0x8f
0028:    DC 4F E0                fmul     qword ptr [edi - 0x20]
002B:    DE C1                   faddp    st(1)
002D:    D9 E0                   fchs    ; -ST(0)
002F:    DC 03                   fadd     qword ptr [ebx]
0031:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0034:    DD 01                   fld      qword ptr [ecx]
0036:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0039:    E8 4E 00 00 00          call     0x8c
003E:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0041:    DD 03                   fld      qword ptr [ebx]
0043:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0046:    E8 44 00 00 00          call     0x8f
004B:    DC 4F E0                fmul     qword ptr [edi - 0x20]
004E:    DE C1                   faddp    st(1)
0050:    D9 E0                   fchs    ; -ST(0)
0052:    DC 02                   fadd     qword ptr [edx]
0054:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0057:    DD 03                   fld      qword ptr [ebx]
0059:    DC 4F D8                fmul     qword ptr [edi - 0x28]
005C:    E8 2B 00 00 00          call     0x8c
0061:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0064:    DD 02                   fld      qword ptr [edx]
0066:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0069:    E8 21 00 00 00          call     0x8f
006E:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0071:    DE C1                   faddp    st(1)
0073:    D9 E0                   fchs    ; -ST(0)
0075:    DC 01                   fadd     qword ptr [ecx]
0077:    DC 4F F0                fmul     qword ptr [edi - 0x10]
007A:    DD 19                   fstp     qword ptr [ecx]
007C:    DD 1A                   fstp     qword ptr [edx]
007E:    DD 1B                   fstp     qword ptr [ebx]
0080:    83 C4 20                add      esp, 0x20
0083:    89 D8                   mov      eax, ebx
0085:    5B                      pop      ebx
0086:    5F                      pop      edi
0087:    5E                      pop      esi
0088:    5D                      pop      ebp
0089:    C2 08 00                ret      8
008C:    DC 47 28                fadd     qword ptr [edi + 0x28]
008F:    DC 0F                   fmul     qword ptr [edi]
0091:    DB 54 24 18             fist     dword ptr [esp + 0x18]
0095:    DB 44 24 18             fild     dword ptr [esp + 0x18]
0099:    DE E9                   fsubp    st(1)
009B:    D9 C0                   fld      st(0)
009D:    D8 C8                   fmul     st(0)
009F:    DD 47 20                fld      qword ptr [edi + 0x20]
00A2:    D8 C9                   fmul     st(1)
00A4:    DC 47 18                fadd     qword ptr [edi + 0x18]
00A7:    D8 C9                   fmul     st(1)
00A9:    DC 47 10                fadd     qword ptr [edi + 0x10]
00AC:    DE C9                   fmulp    st(1)
00AE:    DC 47 08                fadd     qword ptr [edi + 8]
00B1:    DE C9                   fmulp    st(1)
00B3:    8A 44 24 18             mov      al, byte ptr [esp + 0x18]
00B7:    24 01                   and      al, 1
00B9:    74 02                   je       0xbd    ; Conditional jump
00BB:    D9 E0                   fchs    ; -ST(0)
00BD:    C3                      ret
