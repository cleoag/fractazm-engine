; Disassembly: _sphereinv.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 146 bytes, decoded: 146 bytes (100.0%)
; DEoption: 1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 01                   fld      qword ptr [ecx]
0010:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0013:    DD 02                   fld      qword ptr [edx]
0015:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0018:    DD 03                   fld      qword ptr [ebx]
001A:    DC 47 F0                fadd     qword ptr [edi - 0x10]
001D:    D9 C0                   fld      st(0)
001F:    D8 C8                   fmul     st(0)
0021:    D9 C2                   fld      st(2)
0023:    D8 C8                   fmul     st(0)
0025:    DE C1                   faddp    st(1)
0027:    D9 C3                   fld      st(3)
0029:    D8 C8                   fmul     st(0)
002B:    DE C1                   faddp    st(1)
002D:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0030:    83 7F C4 00             cmp      dword ptr [edi - 0x3c], 0
0034:    74 0B                   je       0x41    ; Conditional jump
0036:    D9 E8                   fld1    ; Load 1.0
0038:    D8 D9                   fcomp    st(1)
003A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
003C:    80 E4 41                and      ah, 0x41
003F:    77 47                   ja       0x88    ; Conditional jump
0041:    DC 57 D0                fcom     qword ptr [edi - 0x30]
0044:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0046:    80 E4 41                and      ah, 0x41
0049:    77 25                   ja       0x70    ; Conditional jump
004B:    D9 E8                   fld1    ; Load 1.0
004D:    DE F1                   fdivrp   st(1)
004F:    DC C9                   fmul     st(1), st(0)
0051:    DC CA                   fmul     st(2), st(0)
0053:    DC CB                   fmul     st(3), st(0)
0055:    DD D8                   fstp     st(0)
0057:    DC 67 F0                fsub     qword ptr [edi - 0x10]
005A:    DD 1B                   fstp     qword ptr [ebx]
005C:    DC 67 E8                fsub     qword ptr [edi - 0x18]
005F:    DD 1A                   fstp     qword ptr [edx]
0061:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0064:    DD 19                   fstp     qword ptr [ecx]
0066:    89 D8                   mov      eax, ebx
0068:    5B                      pop      ebx
0069:    5F                      pop      edi
006A:    5E                      pop      esi
006B:    5D                      pop      ebp
006C:    C2 08 00                ret      8
006F:    90                      nop
0070:    83 7F C4 02             cmp      dword ptr [edi - 0x3c], 2
0074:    74 07                   je       0x7d    ; Conditional jump
0076:    DD D8                   fstp     st(0)
0078:    DD 47 C8                fld      qword ptr [edi - 0x38]
007B:    EB D2                   jmp      0x4f
007D:    D9 E0                   fchs    ; -ST(0)
007F:    DC 47 D0                fadd     qword ptr [edi - 0x30]
0082:    DC 47 D0                fadd     qword ptr [edi - 0x30]
0085:    EB C4                   jmp      0x4b
0087:    90                      nop
0088:    DD D8                   fstp     st(0)
008A:    DD D8                   fstp     st(0)
008C:    DD D8                   fstp     st(0)
008E:    DD D8                   fstp     st(0)
0090:    EB D4                   jmp      0x66
