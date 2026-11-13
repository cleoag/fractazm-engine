; Disassembly: _DEComb_ISph_Plane.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 141 bytes, decoded: 141 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    83 BE D0 00 00 00 00    cmp      dword ptr [esi + 0xd0], 0
0015:    75 56                   jne      0x6d    ; Conditional jump
0017:    FF 86 D0 00 00 00       inc      dword ptr [esi + 0xd0]
001D:    DD 01                   fld      qword ptr [ecx]
001F:    DD 02                   fld      qword ptr [edx]
0021:    DD 03                   fld      qword ptr [ebx]
0023:    DC 67 D0                fsub     qword ptr [edi - 0x30]
0026:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0028:    DC 67 C8                fsub     qword ptr [edi - 0x38]
002B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
002D:    DC 67 C0                fsub     qword ptr [edi - 0x40]
0030:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0032:    D9 C0                   fld      st(0)
0034:    D8 C8                   fmul     st(0)
0036:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0038:    D9 C0                   fld      st(0)
003A:    D8 C8                   fmul     st(0)
003C:    DE C4                   faddp    st(4)
003E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0040:    D9 C0                   fld      st(0)
0042:    D8 C8                   fmul     st(0)
0044:    DE C4                   faddp    st(4)
0046:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0048:    DD 47 D8                fld      qword ptr [edi - 0x28]
004B:    DE F1                   fdivrp   st(1)
004D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
004F:    D8 C9                   fmul     st(1)
0051:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0054:    DD 1B                   fstp     qword ptr [ebx]
0056:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0058:    D8 C9                   fmul     st(1)
005A:    DC 67 E8                fsub     qword ptr [edi - 0x18]
005D:    DD 1A                   fstp     qword ptr [edx]
005F:    DE C9                   fmulp    st(1)
0061:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0064:    DD 19                   fstp     qword ptr [ecx]
0066:    EB 1C                   jmp      0x84
0068:    90                      nop
0069:    90                      nop
006A:    90                      nop
006B:    90                      nop
006C:    90                      nop
006D:    DD 01                   fld      qword ptr [ecx]
006F:    DD 02                   fld      qword ptr [edx]
0071:    DD 03                   fld      qword ptr [ebx]
0073:    DD 46 38                fld      qword ptr [esi + 0x38]
0076:    D9 FA                   fsqrt    ; sqrt(ST(0))
0078:    DC CB                   fmul     st(3), st(0)
007A:    DC CA                   fmul     st(2), st(0)
007C:    DE C9                   fmulp    st(1)
007E:    DD 1B                   fstp     qword ptr [ebx]
0080:    DD 1A                   fstp     qword ptr [edx]
0082:    DD 19                   fstp     qword ptr [ecx]
0084:    89 D8                   mov      eax, ebx
0086:    5B                      pop      ebx
0087:    5F                      pop      edi
0088:    5E                      pop      esi
0089:    5D                      pop      ebp
008A:    C2 08 00                ret      8
