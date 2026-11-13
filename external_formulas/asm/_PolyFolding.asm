; Disassembly: _PolyFolding.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 102 bytes, decoded: 102 bytes (100.0%)
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
0013:    DD 03                   fld      qword ptr [ebx]
0015:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0018:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
001A:    DD 47 D0                fld      qword ptr [edi - 0x30]
001D:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0020:    DC 4F 08                fmul     qword ptr [edi + 8]
0023:    DE C1                   faddp    st(1)
0025:    DC 37                   fdiv     qword ptr [edi]
0027:    DC 4F F0                fmul     qword ptr [edi - 0x10]
002A:    D9 FC                   frndint    ; round(ST(0))
002C:    D9 E0                   fchs    ; -ST(0)
002E:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
0031:    DC 0F                   fmul     qword ptr [edi]
0033:    DD 47 E8                fld      qword ptr [edi - 0x18]
0036:    DC 4F 08                fmul     qword ptr [edi + 8]
0039:    DE C1                   faddp    st(1)
003B:    D9 FB                   fsincos    ; ST(0)=cos, push sin
003D:    D9 D0                   fnop
003F:    DD 03                   fld      qword ptr [ebx]
0041:    D8 C9                   fmul     st(1)
0043:    DD 02                   fld      qword ptr [edx]
0045:    D8 CB                   fmul     st(3)
0047:    DE E1                   fsubrp   st(1)
0049:    D9 D0                   fnop
004B:    DD 03                   fld      qword ptr [ebx]
004D:    D8 CB                   fmul     st(3)
004F:    DD 02                   fld      qword ptr [edx]
0051:    D8 CB                   fmul     st(3)
0053:    DE C1                   faddp    st(1)
0055:    D9 D0                   fnop
0057:    DD 1A                   fstp     qword ptr [edx]
0059:    DD 1B                   fstp     qword ptr [ebx]
005B:    DD D8                   fstp     st(0)
005D:    DD D8                   fstp     st(0)
005F:    5B                      pop      ebx
0060:    5F                      pop      edi
0061:    5E                      pop      esi
0062:    5D                      pop      ebp
0063:    C2 08 00                ret      8
