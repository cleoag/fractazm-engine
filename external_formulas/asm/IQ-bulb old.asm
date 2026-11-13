; Disassembly: IQ-bulb old.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 322 bytes, decoded: 322 bytes (100.0%)

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
0078:    D8 C8                   fmul     st(0)
007A:    DD 01                   fld      qword ptr [ecx]
007C:    D8 C8                   fmul     st(0)
007E:    DE C1                   faddp    st(1)
0080:    90                      nop
0081:    90                      nop
0082:    D9 FA                   fsqrt    ; sqrt(ST(0))
0084:    DD 03                   fld      qword ptr [ebx]
0086:    D8 C8                   fmul     st(0)
0088:    DD 02                   fld      qword ptr [edx]
008A:    D8 C8                   fmul     st(0)
008C:    DD 01                   fld      qword ptr [ecx]
008E:    D8 C8                   fmul     st(0)
0090:    DE C1                   faddp    st(1)
0092:    DE C1                   faddp    st(1)
0094:    D9 FA                   fsqrt    ; sqrt(ST(0))
0096:    DC 02                   fadd     qword ptr [edx]
0098:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
009A:    D8 C0                   fadd     st(0)
009C:    DC 4F E8                fmul     qword ptr [edi - 0x18]
009F:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00A1:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00A4:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00A7:    DD 01                   fld      qword ptr [ecx]
00A9:    DD 03                   fld      qword ptr [ebx]
00AB:    90                      nop
00AC:    90                      nop
00AD:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00AF:    83 7F D8 00             cmp      dword ptr [edi - 0x28], 0
00B3:    75 2A                   jne      0xdf    ; Conditional jump
00B5:    DD 03                   fld      qword ptr [ebx]
00B7:    90                      nop
00B8:    D9 EE                   fldz    ; Load 0.0
00BA:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00BC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00BE:    80 E4 41                and      ah, 0x41
00C1:    77 1C                   ja       0xdf    ; Conditional jump
00C3:    DD 01                   fld      qword ptr [ecx]
00C5:    90                      nop
00C6:    D9 EE                   fldz    ; Load 0.0
00C8:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00CA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CC:    80 E4 41                and      ah, 0x41
00CF:    77 08                   ja       0xd9    ; Conditional jump
00D1:    D9 EB                   fldpi    ; Load pi
00D3:    DC 0F                   fmul     qword ptr [edi]
00D5:    DE C1                   faddp    st(1)
00D7:    EB 06                   jmp      0xdf
00D9:    D9 EB                   fldpi    ; Load pi
00DB:    DC 0F                   fmul     qword ptr [edi]
00DD:    DE E9                   fsubp    st(1)
00DF:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00E2:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00E4:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00E7:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00EA:    D9 D0                   fnop
00EC:    DD 45 F8                fld      qword ptr [ebp - 8]
00EF:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00F2:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00F5:    90                      nop
00F6:    90                      nop
00F7:    DC 46 18                fadd     qword ptr [esi + 0x18]
00FA:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00FD:    D9 D0                   fnop
00FF:    DD 45 F8                fld      qword ptr [ebp - 8]
0102:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0105:    90                      nop
0106:    90                      nop
0107:    90                      nop
0108:    90                      nop
0109:    90                      nop
010A:    DC 46 20                fadd     qword ptr [esi + 0x20]
010D:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0110:    D9 D0                   fnop
0112:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0115:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0118:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
011B:    DC 4F E0                fmul     qword ptr [edi - 0x20]
011E:    DC 46 28                fadd     qword ptr [esi + 0x28]
0121:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0124:    D9 D0                   fnop
0126:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0129:    DD 45 C8                fld      qword ptr [ebp - 0x38]
012C:    DD 45 C0                fld      qword ptr [ebp - 0x40]
012F:    DD 19                   fstp     qword ptr [ecx]
0131:    DD 1A                   fstp     qword ptr [edx]
0133:    DD 1B                   fstp     qword ptr [ebx]
0135:    D9 D0                   fnop
0137:    8B C3                   mov      eax, ebx
0139:    5F                      pop      edi
013A:    5E                      pop      esi
013B:    5B                      pop      ebx
013C:    8B E5                   mov      esp, ebp
013E:    5D                      pop      ebp
013F:    C2 08 00                ret      8
