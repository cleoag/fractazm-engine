; Disassembly: _ngon.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 151 bytes, decoded: 151 bytes (100.0%)
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
0013:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0016:    DD 12                   fst      qword ptr [edx]
0018:    DD 03                   fld      qword ptr [ebx]
001A:    DC 47 E0                fadd     qword ptr [edi - 0x20]
001D:    DD 13                   fst      qword ptr [ebx]
001F:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0021:    DD 47 D0                fld      qword ptr [edi - 0x30]
0024:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0027:    DC 4F 08                fmul     qword ptr [edi + 8]
002A:    DE C1                   faddp    st(1)
002C:    DC 37                   fdiv     qword ptr [edi]
002E:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0031:    DD 54 24 0C             fst      qword ptr [esp + 0xc]
0035:    90                      nop
0036:    90                      nop
0037:    DB 1C 24                fistp    dword ptr [esp]
003A:    DB 04 24                fild     dword ptr [esp]
003D:    D9 E0                   fchs    ; -ST(0)
003F:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
0042:    DC 0F                   fmul     qword ptr [edi]
0044:    DD 47 E8                fld      qword ptr [edi - 0x18]
0047:    DC 4F 08                fmul     qword ptr [edi + 8]
004A:    DE C1                   faddp    st(1)
004C:    D9 FB                   fsincos    ; ST(0)=cos, push sin
004E:    D9 D0                   fnop
0050:    DD 03                   fld      qword ptr [ebx]
0052:    D8 C9                   fmul     st(1)
0054:    DD 02                   fld      qword ptr [edx]
0056:    D8 CB                   fmul     st(3)
0058:    DE E1                   fsubrp   st(1)
005A:    90                      nop
005B:    DD 44 24 0C             fld      qword ptr [esp + 0xc]
005F:    8B 47 CC                mov      eax, dword ptr [edi - 0x34]
0062:    85 C0                   test     eax, eax
0064:    75 11                   jne      0x77    ; Conditional jump
0066:    DB 04 24                fild     dword ptr [esp]
0069:    D9 E0                   fchs    ; -ST(0)
006B:    DE C1                   faddp    st(1)
006D:    8B 04 24                mov      eax, dword ptr [esp]
0070:    83 E0 01                and      eax, 1
0073:    75 02                   jne      0x77    ; Conditional jump
0075:    D9 E0                   fchs    ; -ST(0)
0077:    90                      nop
0078:    90                      nop
0079:    90                      nop
007A:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
007D:    DC 0F                   fmul     qword ptr [edi]
007F:    DC 67 D8                fsub     qword ptr [edi - 0x28]
0082:    DD 1A                   fstp     qword ptr [edx]
0084:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0087:    DD 1B                   fstp     qword ptr [ebx]
0089:    DD D8                   fstp     st(0)
008B:    DD D8                   fstp     st(0)
008D:    83 C4 20                add      esp, 0x20
0090:    5B                      pop      ebx
0091:    5F                      pop      edi
0092:    5E                      pop      esi
0093:    5D                      pop      ebp
0094:    C2 08 00                ret      8
