; Disassembly: _BPolyhedronFromSphere.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 157 bytes, decoded: 157 bytes (100.0%)
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
0032:    DD 02                   fld      qword ptr [edx]
0034:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0036:    D9 E1                   fabs    ; |ST(0)|
0038:    DC 55 F8                fcom     qword ptr [ebp - 8]
003B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
003D:    80 E4 41                and      ah, 0x41
0040:    7F 05                   jg       0x47    ; Conditional jump
0042:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
0045:    EB F1                   jmp      0x38
0047:    D9 FF                   fcos    ; cos(ST(0))
0049:    DD 55 F0                fst      qword ptr [ebp - 0x10]
004C:    DC 0A                   fmul     qword ptr [edx]
004E:    DD 55 F8                fst      qword ptr [ebp - 8]
0051:    D8 C8                   fmul     st(0)
0053:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0056:    DC 09                   fmul     qword ptr [ecx]
0058:    DD 55 F0                fst      qword ptr [ebp - 0x10]
005B:    D8 C8                   fmul     st(0)
005D:    DE C1                   faddp    st(1)
005F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0061:    D9 D0                   fnop
0063:    DD 03                   fld      qword ptr [ebx]
0065:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0067:    D9 E1                   fabs    ; |ST(0)|
0069:    DC 55 E8                fcom     qword ptr [ebp - 0x18]
006C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006E:    80 E4 41                and      ah, 0x41
0071:    7F 05                   jg       0x78    ; Conditional jump
0073:    DC 65 E0                fsub     qword ptr [ebp - 0x20]
0076:    EB F1                   jmp      0x69
0078:    D9 FF                   fcos    ; cos(ST(0))
007A:    D9 C0                   fld      st(0)
007C:    D9 C0                   fld      st(0)
007E:    DC 4D F8                fmul     qword ptr [ebp - 8]
0081:    DD 1A                   fstp     qword ptr [edx]
0083:    D9 D0                   fnop
0085:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0088:    DD 19                   fstp     qword ptr [ecx]
008A:    D9 D0                   fnop
008C:    DC 0B                   fmul     qword ptr [ebx]
008E:    DD 1B                   fstp     qword ptr [ebx]
0090:    D9 D0                   fnop
0092:    8B C3                   mov      eax, ebx
0094:    5F                      pop      edi
0095:    5E                      pop      esi
0096:    5B                      pop      ebx
0097:    89 EC                   mov      esp, ebp
0099:    5D                      pop      ebp
009A:    C2 08 00                ret      8
