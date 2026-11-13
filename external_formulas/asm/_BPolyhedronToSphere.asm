; Disassembly: _BPolyhedronToSphere.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 158 bytes, decoded: 158 bytes (100.0%)
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
0023:    DD 07                   fld      qword ptr [edi]
0025:    DC 77 E8                fdiv     qword ptr [edi - 0x18]
0028:    DD 55 E8                fst      qword ptr [ebp - 0x18]
002B:    D8 C0                   fadd     st(0)
002D:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0030:    DD 01                   fld      qword ptr [ecx]
0032:    D8 C8                   fmul     st(0)
0034:    DD 02                   fld      qword ptr [edx]
0036:    D8 C8                   fmul     st(0)
0038:    DE C1                   faddp    st(1)
003A:    D9 FA                   fsqrt    ; sqrt(ST(0))
003C:    DD 03                   fld      qword ptr [ebx]
003E:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0040:    D9 E1                   fabs    ; |ST(0)|
0042:    DC 55 E8                fcom     qword ptr [ebp - 0x18]
0045:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0047:    80 E4 41                and      ah, 0x41
004A:    7F 05                   jg       0x51    ; Conditional jump
004C:    DC 65 E0                fsub     qword ptr [ebp - 0x20]
004F:    EB F1                   jmp      0x42
0051:    D9 FF                   fcos    ; cos(ST(0))
0053:    D9 E8                   fld1    ; Load 1.0
0055:    DE F1                   fdivrp   st(1)
0057:    D9 C0                   fld      st(0)
0059:    D9 C0                   fld      st(0)
005B:    DC 0B                   fmul     qword ptr [ebx]
005D:    DD 1B                   fstp     qword ptr [ebx]
005F:    DC 0A                   fmul     qword ptr [edx]
0061:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0064:    DC 09                   fmul     qword ptr [ecx]
0066:    DD 55 E0                fst      qword ptr [ebp - 0x20]
0069:    DD 45 E8                fld      qword ptr [ebp - 0x18]
006C:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
006E:    D9 E1                   fabs    ; |ST(0)|
0070:    DC 55 F8                fcom     qword ptr [ebp - 8]
0073:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0075:    80 E4 41                and      ah, 0x41
0078:    7F 05                   jg       0x7f    ; Conditional jump
007A:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
007D:    EB F1                   jmp      0x70
007F:    D9 FF                   fcos    ; cos(ST(0))
0081:    D9 E8                   fld1    ; Load 1.0
0083:    DE F1                   fdivrp   st(1)
0085:    D9 C0                   fld      st(0)
0087:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
008A:    DD 1A                   fstp     qword ptr [edx]
008C:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
008F:    DD 19                   fstp     qword ptr [ecx]
0091:    D9 D0                   fnop
0093:    8B C3                   mov      eax, ebx
0095:    5F                      pop      edi
0096:    5E                      pop      esi
0097:    5B                      pop      ebx
0098:    89 EC                   mov      esp, ebp
009A:    5D                      pop      ebp
009B:    C2 08 00                ret      8
