; Disassembly: Ruckerbulb.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 287 bytes, decoded: 287 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 40 00 00 00       sub      esp, 0x40
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 03                   fld      qword ptr [ebx]
0018:    D8 C8                   fmul     st(0)
001A:    DD 02                   fld      qword ptr [edx]
001C:    D8 C8                   fmul     st(0)
001E:    DD 01                   fld      qword ptr [ecx]
0020:    D8 C8                   fmul     st(0)
0022:    DE C1                   faddp    st(1)
0024:    DE C1                   faddp    st(1)
0026:    D9 FA                   fsqrt    ; sqrt(ST(0))
0028:    DD 55 C0                fst      qword ptr [ebp - 0x40]
002B:    DD 47 F0                fld      qword ptr [edi - 0x10]
002E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0030:    D9 E1                   fabs    ; |ST(0)|
0032:    D9 F1                   fyl2x
0034:    D9 E8                   fld1    ; Load 1.0
0036:    D9 C1                   fld      st(1)
0038:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
003A:    D9 F0                   f2xm1
003C:    DE C1                   faddp    st(1)
003E:    D9 FD                   fscale
0040:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0042:    DD D8                   fstp     st(0)
0044:    DD 5D F8                fstp     qword ptr [ebp - 8]
0047:    D9 D0                   fnop
0049:    83 7F DC 00             cmp      dword ptr [edi - 0x24], 0
004D:    75 08                   jne      0x57    ; Conditional jump
004F:    DD 45 F8                fld      qword ptr [ebp - 8]
0052:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0055:    EB 1F                   jmp      0x76
0057:    DD 45 C0                fld      qword ptr [ebp - 0x40]
005A:    DD 47 E8                fld      qword ptr [edi - 0x18]
005D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
005F:    D9 E1                   fabs    ; |ST(0)|
0061:    D9 F1                   fyl2x
0063:    D9 E8                   fld1    ; Load 1.0
0065:    D9 C1                   fld      st(1)
0067:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
0069:    D9 F0                   f2xm1
006B:    DE C1                   faddp    st(1)
006D:    D9 FD                   fscale
006F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0071:    DD D8                   fstp     st(0)
0073:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0076:    DD 03                   fld      qword ptr [ebx]
0078:    D9 E0                   fchs    ; -ST(0)
007A:    DD 01                   fld      qword ptr [ecx]
007C:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
007E:    83 7F D8 00             cmp      dword ptr [edi - 0x28], 0
0082:    75 2A                   jne      0xae    ; Conditional jump
0084:    DD 03                   fld      qword ptr [ebx]
0086:    90                      nop
0087:    D9 EE                   fldz    ; Load 0.0
0089:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
008B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
008D:    80 E4 41                and      ah, 0x41
0090:    7F 1C                   jg       0xae    ; Conditional jump
0092:    DD 01                   fld      qword ptr [ecx]
0094:    90                      nop
0095:    D9 EE                   fldz    ; Load 0.0
0097:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0099:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009B:    80 E4 41                and      ah, 0x41
009E:    7C 08                   jl       0xa8    ; Conditional jump
00A0:    D9 EB                   fldpi    ; Load pi
00A2:    DC 0F                   fmul     qword ptr [edi]
00A4:    DE E9                   fsubp    st(1)
00A6:    EB 06                   jmp      0xae
00A8:    D9 EB                   fldpi    ; Load pi
00AA:    DC 0F                   fmul     qword ptr [edi]
00AC:    DE C1                   faddp    st(1)
00AE:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00B1:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00B3:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00B6:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00B9:    DD 02                   fld      qword ptr [edx]
00BB:    DD 03                   fld      qword ptr [ebx]
00BD:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00BF:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00C2:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00C4:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00C7:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00CA:    D9 D0                   fnop
00CC:    DD 45 F8                fld      qword ptr [ebp - 8]
00CF:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00D2:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00D5:    D9 E0                   fchs    ; -ST(0)
00D7:    DC 46 18                fadd     qword ptr [esi + 0x18]
00DA:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00DD:    D9 D0                   fnop
00DF:    DD 45 F8                fld      qword ptr [ebp - 8]
00E2:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00E5:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00E8:    D9 E0                   fchs    ; -ST(0)
00EA:    DC 46 20                fadd     qword ptr [esi + 0x20]
00ED:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
00F0:    D9 D0                   fnop
00F2:    DD 45 C0                fld      qword ptr [ebp - 0x40]
00F5:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00F8:    DC 4F E0                fmul     qword ptr [edi - 0x20]
00FB:    DC 46 28                fadd     qword ptr [esi + 0x28]
00FE:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0101:    D9 D0                   fnop
0103:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0106:    DD 45 C8                fld      qword ptr [ebp - 0x38]
0109:    DD 45 C0                fld      qword ptr [ebp - 0x40]
010C:    DD 19                   fstp     qword ptr [ecx]
010E:    DD 1A                   fstp     qword ptr [edx]
0110:    DD 1B                   fstp     qword ptr [ebx]
0112:    D9 D0                   fnop
0114:    8B C3                   mov      eax, ebx
0116:    5F                      pop      edi
0117:    5E                      pop      esi
0118:    5B                      pop      ebx
0119:    8B E5                   mov      esp, ebp
011B:    5D                      pop      ebp
011C:    C2 08 00                ret      8
