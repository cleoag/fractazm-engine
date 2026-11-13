; Disassembly: _BPolygonToCircle.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 89 bytes, decoded: 89 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 30 00 00 00       sub      esp, 0x30
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 07                   fld      qword ptr [edi]
0018:    DC 77 F0                fdiv     qword ptr [edi - 0x10]
001B:    DD 55 F8                fst      qword ptr [ebp - 8]
001E:    D8 C0                   fadd     st(0)
0020:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0023:    DD 01                   fld      qword ptr [ecx]
0025:    DD 02                   fld      qword ptr [edx]
0027:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0029:    D9 E1                   fabs    ; |ST(0)|
002B:    DC 55 F8                fcom     qword ptr [ebp - 8]
002E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0030:    80 E4 41                and      ah, 0x41
0033:    7F 05                   jg       0x3a    ; Conditional jump
0035:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
0038:    EB F1                   jmp      0x2b
003A:    D9 FF                   fcos    ; cos(ST(0))
003C:    D9 E8                   fld1    ; Load 1.0
003E:    DE F1                   fdivrp   st(1)
0040:    D9 C0                   fld      st(0)
0042:    DC 0A                   fmul     qword ptr [edx]
0044:    DD 1A                   fstp     qword ptr [edx]
0046:    D9 D0                   fnop
0048:    DC 09                   fmul     qword ptr [ecx]
004A:    DD 19                   fstp     qword ptr [ecx]
004C:    D9 D0                   fnop
004E:    8B C3                   mov      eax, ebx
0050:    5F                      pop      edi
0051:    5E                      pop      esi
0052:    5B                      pop      ebx
0053:    89 EC                   mov      esp, ebp
0055:    5D                      pop      ebp
0056:    C2 08 00                ret      8
