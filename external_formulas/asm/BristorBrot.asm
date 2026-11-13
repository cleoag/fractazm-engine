; Disassembly: BristorBrot.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 88 bytes, decoded: 88 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 00                   fld      qword ptr [eax]
000D:    D8 C8                   fmul     st(0)
000F:    DD 02                   fld      qword ptr [edx]
0011:    D8 C8                   fmul     st(0)
0013:    D9 E0                   fchs    ; -ST(0)
0015:    DE C1                   faddp    st(1)
0017:    DD 01                   fld      qword ptr [ecx]
0019:    D8 C8                   fmul     st(0)
001B:    D9 E0                   fchs    ; -ST(0)
001D:    DE C1                   faddp    st(1)
001F:    DC 46 18                fadd     qword ptr [esi + 0x18]
0022:    D9 D0                   fnop
0024:    DD 00                   fld      qword ptr [eax]
0026:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0029:    DD 01                   fld      qword ptr [ecx]
002B:    DC 4F E0                fmul     qword ptr [edi - 0x20]
002E:    DE C1                   faddp    st(1)
0030:    DC 0A                   fmul     qword ptr [edx]
0032:    DC 46 20                fadd     qword ptr [esi + 0x20]
0035:    D9 D0                   fnop
0037:    DD 00                   fld      qword ptr [eax]
0039:    DC 4F F0                fmul     qword ptr [edi - 0x10]
003C:    DD 02                   fld      qword ptr [edx]
003E:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0041:    DE C1                   faddp    st(1)
0043:    DC 09                   fmul     qword ptr [ecx]
0045:    DC 46 28                fadd     qword ptr [esi + 0x28]
0048:    D9 D0                   fnop
004A:    DD 19                   fstp     qword ptr [ecx]
004C:    DD 1A                   fstp     qword ptr [edx]
004E:    DD 18                   fstp     qword ptr [eax]
0050:    90                      nop
0051:    90                      nop
0052:    5F                      pop      edi
0053:    5E                      pop      esi
0054:    5D                      pop      ebp
0055:    C2 08 00                ret      8
