; Disassembly: _BStellahedron_F_Sphere.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 221 bytes, decoded: 221 bytes (100.0%)
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
0023:    DD 45 F8                fld      qword ptr [ebp - 8]
0026:    DC 4F 08                fmul     qword ptr [edi + 8]
0029:    D9 C0                   fld      st(0)
002B:    DC 67 E0                fsub     qword ptr [edi - 0x20]
002E:    D9 FF                   fcos    ; cos(ST(0))
0030:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0033:    D9 FF                   fcos    ; cos(ST(0))
0035:    DC 75 D8                fdiv     qword ptr [ebp - 0x28]
0038:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
003B:    DD 07                   fld      qword ptr [edi]
003D:    DC 77 E8                fdiv     qword ptr [edi - 0x18]
0040:    DD 55 E8                fst      qword ptr [ebp - 0x18]
0043:    D8 C0                   fadd     st(0)
0045:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0048:    DD 45 E8                fld      qword ptr [ebp - 0x18]
004B:    DC 4F 08                fmul     qword ptr [edi + 8]
004E:    D9 C0                   fld      st(0)
0050:    DC 67 D8                fsub     qword ptr [edi - 0x28]
0053:    D9 FF                   fcos    ; cos(ST(0))
0055:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0058:    D9 FF                   fcos    ; cos(ST(0))
005A:    DC 75 D0                fdiv     qword ptr [ebp - 0x30]
005D:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0060:    DD 01                   fld      qword ptr [ecx]
0062:    DD 02                   fld      qword ptr [edx]
0064:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0066:    D9 E1                   fabs    ; |ST(0)|
0068:    DC 55 F8                fcom     qword ptr [ebp - 8]
006B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006D:    80 E4 41                and      ah, 0x41
0070:    7F 05                   jg       0x77    ; Conditional jump
0072:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
0075:    EB F1                   jmp      0x68
0077:    D9 E1                   fabs    ; |ST(0)|
0079:    DC 67 E0                fsub     qword ptr [edi - 0x20]
007C:    D9 FF                   fcos    ; cos(ST(0))
007E:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0081:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0084:    DC 0A                   fmul     qword ptr [edx]
0086:    DD 55 F8                fst      qword ptr [ebp - 8]
0089:    D8 C8                   fmul     st(0)
008B:    DD 45 F0                fld      qword ptr [ebp - 0x10]
008E:    DC 09                   fmul     qword ptr [ecx]
0090:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0093:    D8 C8                   fmul     st(0)
0095:    DE C1                   faddp    st(1)
0097:    D9 FA                   fsqrt    ; sqrt(ST(0))
0099:    D9 D0                   fnop
009B:    DD 03                   fld      qword ptr [ebx]
009D:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
009F:    D9 E1                   fabs    ; |ST(0)|
00A1:    DC 55 E8                fcom     qword ptr [ebp - 0x18]
00A4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A6:    80 E4 41                and      ah, 0x41
00A9:    7F 05                   jg       0xb0    ; Conditional jump
00AB:    DC 65 E0                fsub     qword ptr [ebp - 0x20]
00AE:    EB F1                   jmp      0xa1
00B0:    D9 E1                   fabs    ; |ST(0)|
00B2:    DC 67 D8                fsub     qword ptr [edi - 0x28]
00B5:    D9 FF                   fcos    ; cos(ST(0))
00B7:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00BA:    D9 C0                   fld      st(0)
00BC:    D9 C0                   fld      st(0)
00BE:    DC 4D F8                fmul     qword ptr [ebp - 8]
00C1:    DD 1A                   fstp     qword ptr [edx]
00C3:    D9 D0                   fnop
00C5:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00C8:    DD 19                   fstp     qword ptr [ecx]
00CA:    D9 D0                   fnop
00CC:    DC 0B                   fmul     qword ptr [ebx]
00CE:    DD 1B                   fstp     qword ptr [ebx]
00D0:    D9 D0                   fnop
00D2:    8B C3                   mov      eax, ebx
00D4:    5F                      pop      edi
00D5:    5E                      pop      esi
00D6:    5B                      pop      ebx
00D7:    89 EC                   mov      esp, ebp
00D9:    5D                      pop      ebp
00DA:    C2 08 00                ret      8
