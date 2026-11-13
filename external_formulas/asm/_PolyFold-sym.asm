; Disassembly: _PolyFold-sym.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 131 bytes, decoded: 131 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 02                   fld      qword ptr [edx]
0010:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0013:    DD 12                   fst      qword ptr [edx]
0015:    DD 03                   fld      qword ptr [ebx]
0017:    DC 47 E0                fadd     qword ptr [edi - 0x20]
001A:    DD 13                   fst      qword ptr [ebx]
001C:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
001E:    DD 47 D0                fld      qword ptr [edi - 0x30]
0021:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0024:    DC 4F 08                fmul     qword ptr [edi + 8]
0027:    DE C1                   faddp    st(1)
0029:    DC 37                   fdiv     qword ptr [edi]
002B:    DC 4F F0                fmul     qword ptr [edi - 0x10]
002E:    83 C4 F0                add      esp, -0x10
0031:    90                      nop
0032:    DB 1C 24                fistp    dword ptr [esp]
0035:    DB 04 24                fild     dword ptr [esp]
0038:    D9 E0                   fchs    ; -ST(0)
003A:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
003D:    DC 0F                   fmul     qword ptr [edi]
003F:    DD 47 E8                fld      qword ptr [edi - 0x18]
0042:    DC 4F 08                fmul     qword ptr [edi + 8]
0045:    DE C1                   faddp    st(1)
0047:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0049:    D9 D0                   fnop
004B:    DD 03                   fld      qword ptr [ebx]
004D:    D8 C9                   fmul     st(1)
004F:    DD 02                   fld      qword ptr [edx]
0051:    D8 CB                   fmul     st(3)
0053:    DE E1                   fsubrp   st(1)
0055:    D9 D0                   fnop
0057:    DD 03                   fld      qword ptr [ebx]
0059:    D8 CB                   fmul     st(3)
005B:    DD 02                   fld      qword ptr [edx]
005D:    D8 CB                   fmul     st(3)
005F:    DE C1                   faddp    st(1)
0061:    8B 04 24                mov      eax, dword ptr [esp]
0064:    83 E0 01                and      eax, 1
0067:    75 02                   jne      0x6b    ; Conditional jump
0069:    D9 E0                   fchs    ; -ST(0)
006B:    83 C4 10                add      esp, 0x10
006E:    DC 67 D8                fsub     qword ptr [edi - 0x28]
0071:    DD 1A                   fstp     qword ptr [edx]
0073:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0076:    DD 1B                   fstp     qword ptr [ebx]
0078:    DD D8                   fstp     st(0)
007A:    DD D8                   fstp     st(0)
007C:    5B                      pop      ebx
007D:    5F                      pop      edi
007E:    5E                      pop      esi
007F:    5D                      pop      ebp
0080:    C2 08 00                ret      8
