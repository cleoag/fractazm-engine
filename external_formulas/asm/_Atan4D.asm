; Disassembly: _Atan4D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 285 bytes, decoded: 285 bytes (100.0%)
; DEoption: 4

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
0012:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0015:    DD 1B                   fstp     qword ptr [ebx]
0017:    DD 02                   fld      qword ptr [edx]
0019:    DC 47 E8                fadd     qword ptr [edi - 0x18]
001C:    DD 1A                   fstp     qword ptr [edx]
001E:    DD 01                   fld      qword ptr [ecx]
0020:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0023:    DD 19                   fstp     qword ptr [ecx]
0025:    DD 41 08                fld      qword ptr [ecx + 8]
0028:    DC 47 D8                fadd     qword ptr [edi - 0x28]
002B:    DD 59 08                fstp     qword ptr [ecx + 8]
002E:    90                      nop
002F:    DD 41 08                fld      qword ptr [ecx + 8]
0032:    D8 C8                   fmul     st(0)
0034:    DD 02                   fld      qword ptr [edx]
0036:    D8 C8                   fmul     st(0)
0038:    DE C1                   faddp    st(1)
003A:    DD 01                   fld      qword ptr [ecx]
003C:    D8 C8                   fmul     st(0)
003E:    DE C1                   faddp    st(1)
0040:    D9 FA                   fsqrt    ; sqrt(ST(0))
0042:    D9 C0                   fld      st(0)
0044:    DD 47 08                fld      qword ptr [edi + 8]
0047:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0049:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
004D:    90                      nop
004E:    90                      nop
004F:    0F 83 BC 00 00 00       jae      0x111    ; Conditional jump
0055:    DD 03                   fld      qword ptr [ebx]
0057:    D9 E4                   ftst
0059:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005B:    80 E4 50                and      ah, 0x50
005E:    9E                      sahf
005F:    DD D8                   fstp     st(0)
0061:    90                      nop
0062:    90                      nop
0063:    90                      nop
0064:    90                      nop
0065:    75 1F                   jne      0x86    ; Conditional jump
0067:    90                      nop
0068:    90                      nop
0069:    90                      nop
006A:    90                      nop
006B:    D9 C0                   fld      st(0)
006D:    D9 E8                   fld1    ; Load 1.0
006F:    D9 E0                   fchs    ; -ST(0)
0071:    DE C1                   faddp    st(1)
0073:    D9 E1                   fabs    ; |ST(0)|
0075:    DD 47 08                fld      qword ptr [edi + 8]
0078:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
007A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
007C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
007E:    90                      nop
007F:    90                      nop
0080:    0F 83 8B 00 00 00       jae      0x111    ; Conditional jump
0086:    D9 C0                   fld      st(0)
0088:    D9 E0                   fchs    ; -ST(0)
008A:    D9 E8                   fld1    ; Load 1.0
008C:    DE C1                   faddp    st(1)
008E:    DD 03                   fld      qword ptr [ebx]
0090:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0092:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0094:    D9 C1                   fld      st(1)
0096:    D9 E8                   fld1    ; Load 1.0
0098:    DE C1                   faddp    st(1)
009A:    DD 03                   fld      qword ptr [ebx]
009C:    D9 E0                   fchs    ; -ST(0)
009E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00A0:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00A2:    D9 E0                   fchs    ; -ST(0)
00A4:    DE C1                   faddp    st(1)
00A6:    DC 0F                   fmul     qword ptr [edi]
00A8:    90                      nop
00A9:    D9 D0                   fnop
00AB:    90                      nop
00AC:    D9 C1                   fld      st(1)
00AE:    D9 E8                   fld1    ; Load 1.0
00B0:    DE C1                   faddp    st(1)
00B2:    D8 C8                   fmul     st(0)
00B4:    DD 03                   fld      qword ptr [ebx]
00B6:    D8 C8                   fmul     st(0)
00B8:    DE C1                   faddp    st(1)
00BA:    D9 C2                   fld      st(2)
00BC:    D9 E0                   fchs    ; -ST(0)
00BE:    D9 E8                   fld1    ; Load 1.0
00C0:    DE C1                   faddp    st(1)
00C2:    D8 C8                   fmul     st(0)
00C4:    DD 03                   fld      qword ptr [ebx]
00C6:    D8 C8                   fmul     st(0)
00C8:    DE C1                   faddp    st(1)
00CA:    DE F9                   fdivp    st(1)
00CC:    D9 ED                   fldln2    ; Load ln(2)
00CE:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D0:    D9 F1                   fyl2x
00D2:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00D5:    D8 F2                   fdiv     st(2)
00D7:    D9 D0                   fnop
00D9:    D9 C1                   fld      st(1)
00DB:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00DE:    DD 1B                   fstp     qword ptr [ebx]
00E0:    DD 02                   fld      qword ptr [edx]
00E2:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00E5:    D8 C9                   fmul     st(1)
00E7:    DD 1A                   fstp     qword ptr [edx]
00E9:    DD 01                   fld      qword ptr [ecx]
00EB:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00EE:    D8 C9                   fmul     st(1)
00F0:    DD 19                   fstp     qword ptr [ecx]
00F2:    DD 41 08                fld      qword ptr [ecx + 8]
00F5:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00F8:    D8 C9                   fmul     st(1)
00FA:    DD 59 08                fstp     qword ptr [ecx + 8]
00FD:    DD D8                   fstp     st(0)
00FF:    DD D8                   fstp     st(0)
0101:    DD D8                   fstp     st(0)
0103:    DD D8                   fstp     st(0)
0105:    90                      nop
0106:    90                      nop
0107:    8B C3                   mov      eax, ebx
0109:    5F                      pop      edi
010A:    5E                      pop      esi
010B:    58                      pop      eax
010C:    5B                      pop      ebx
010D:    5D                      pop      ebp
010E:    C2 08 00                ret      8
0111:    DD D8                   fstp     st(0)
0113:    DD 03                   fld      qword ptr [ebx]
0115:    D9 E8                   fld1    ; Load 1.0
0117:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0119:    DD 1B                   fstp     qword ptr [ebx]
011B:    EB E8                   jmp      0x105
