; Disassembly: _CelticMode.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 104 bytes, decoded: 104 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    50                      push     eax
0005:    56                      push     esi
0006:    57                      push     edi
0007:    90                      nop
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B D8                   mov      ebx, eax
000D:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0010:    DD 03                   fld      qword ptr [ebx]
0012:    DD 03                   fld      qword ptr [ebx]
0014:    DD 46 18                fld      qword ptr [esi + 0x18]
0017:    DC 4F F0                fmul     qword ptr [edi - 0x10]
001A:    DE C1                   faddp    st(1)
001C:    D9 E1                   fabs    ; |ST(0)|
001E:    DD 46 18                fld      qword ptr [esi + 0x18]
0021:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0024:    D9 E0                   fchs    ; -ST(0)
0026:    DE C1                   faddp    st(1)
0028:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002A:    DC 4F E8                fmul     qword ptr [edi - 0x18]
002D:    DE C1                   faddp    st(1)
002F:    DD 1B                   fstp     qword ptr [ebx]
0031:    DD 47 E0                fld      qword ptr [edi - 0x20]
0034:    D9 EE                   fldz    ; Load 0.0
0036:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0038:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
003A:    80 E4 41                and      ah, 0x41
003D:    80 F4 40                xor      ah, 0x40
0040:    74 18                   je       0x5a    ; Conditional jump
0042:    DD 01                   fld      qword ptr [ecx]
0044:    DD 46 28                fld      qword ptr [esi + 0x28]
0047:    DC 4F E0                fmul     qword ptr [edi - 0x20]
004A:    DE C1                   faddp    st(1)
004C:    D9 E1                   fabs    ; |ST(0)|
004E:    DD 46 28                fld      qword ptr [esi + 0x28]
0051:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0054:    D9 E0                   fchs    ; -ST(0)
0056:    DE C1                   faddp    st(1)
0058:    DD 19                   fstp     qword ptr [ecx]
005A:    D9 D0                   fnop
005C:    8B C3                   mov      eax, ebx
005E:    5F                      pop      edi
005F:    5E                      pop      esi
0060:    58                      pop      eax
0061:    5B                      pop      ebx
0062:    8B E5                   mov      esp, ebp
0064:    5D                      pop      ebp
0065:    C2 08 00                ret      8
