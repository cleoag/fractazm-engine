; Disassembly: _skew.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 107 bytes, decoded: 107 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 47 F0                fld      qword ptr [edi - 0x10]
0011:    DD 03                   fld      qword ptr [ebx]
0013:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0016:    D8 E9                   fsubr    st(1)
0018:    DD 02                   fld      qword ptr [edx]
001A:    DC 4F E8                fmul     qword ptr [edi - 0x18]
001D:    D8 EA                   fsubr    st(2)
001F:    DD 01                   fld      qword ptr [ecx]
0021:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0024:    D8 EB                   fsubr    st(3)
0026:    D8 D1                   fcom     st(1)
0028:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
002A:    80 E4 41                and      ah, 0x41
002D:    7E 04                   jle      0x33    ; Conditional jump
002F:    DD D9                   fstp     st(1)
0031:    EB 02                   jmp      0x35
0033:    DD D8                   fstp     st(0)
0035:    D8 D1                   fcom     st(1)
0037:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0039:    80 E4 41                and      ah, 0x41
003C:    7E 04                   jle      0x42    ; Conditional jump
003E:    DD D9                   fstp     st(1)
0040:    EB 02                   jmp      0x44
0042:    DD D8                   fstp     st(0)
0044:    DD D9                   fstp     st(1)
0046:    D9 C0                   fld      st(0)
0048:    D9 E1                   fabs    ; |ST(0)|
004A:    DE C1                   faddp    st(1)
004C:    90                      nop
004D:    90                      nop
004E:    DD 03                   fld      qword ptr [ebx]
0050:    D8 E1                   fsub     st(1)
0052:    DD 02                   fld      qword ptr [edx]
0054:    D8 E2                   fsub     st(2)
0056:    DD 01                   fld      qword ptr [ecx]
0058:    D8 E3                   fsub     st(3)
005A:    DD 19                   fstp     qword ptr [ecx]
005C:    DD 1A                   fstp     qword ptr [edx]
005E:    DD 1B                   fstp     qword ptr [ebx]
0060:    DD D8                   fstp     st(0)
0062:    89 D8                   mov      eax, ebx
0064:    5B                      pop      ebx
0065:    5F                      pop      edi
0066:    5E                      pop      esi
0067:    5D                      pop      ebp
0068:    C2 08 00                ret      8
