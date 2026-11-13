; Disassembly: _FoldingTetraKaleid.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 308 bytes, decoded: 308 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 10 00 00 00       sub      esp, 0x10
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    D9 EE                   fldz    ; Load 0.0
0018:    D9 55 F4                fst      dword ptr [ebp - 0xc]
001B:    D9 55 F8                fst      dword ptr [ebp - 8]
001E:    D9 5D FC                fstp     dword ptr [ebp - 4]
0021:    DD 03                   fld      qword ptr [ebx]
0023:    DC 02                   fadd     qword ptr [edx]
0025:    D9 EE                   fldz    ; Load 0.0
0027:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0029:    90                      nop
002A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
002C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
002E:    72 11                   jb       0x41    ; Conditional jump
0030:    DD 03                   fld      qword ptr [ebx]
0032:    D9 E0                   fchs    ; -ST(0)
0034:    DD 02                   fld      qword ptr [edx]
0036:    D9 E0                   fchs    ; -ST(0)
0038:    DD 1B                   fstp     qword ptr [ebx]
003A:    DD 1A                   fstp     qword ptr [edx]
003C:    D9 E8                   fld1    ; Load 1.0
003E:    D9 5D FC                fstp     dword ptr [ebp - 4]
0041:    DD 03                   fld      qword ptr [ebx]
0043:    DC 01                   fadd     qword ptr [ecx]
0045:    D9 EE                   fldz    ; Load 0.0
0047:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0049:    90                      nop
004A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
004E:    72 11                   jb       0x61    ; Conditional jump
0050:    DD 03                   fld      qword ptr [ebx]
0052:    D9 E0                   fchs    ; -ST(0)
0054:    DD 01                   fld      qword ptr [ecx]
0056:    D9 E0                   fchs    ; -ST(0)
0058:    DD 1B                   fstp     qword ptr [ebx]
005A:    DD 19                   fstp     qword ptr [ecx]
005C:    D9 E8                   fld1    ; Load 1.0
005E:    D9 5D F8                fstp     dword ptr [ebp - 8]
0061:    DD 01                   fld      qword ptr [ecx]
0063:    DC 02                   fadd     qword ptr [edx]
0065:    D9 EE                   fldz    ; Load 0.0
0067:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0069:    90                      nop
006A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006E:    72 11                   jb       0x81    ; Conditional jump
0070:    DD 01                   fld      qword ptr [ecx]
0072:    D9 E0                   fchs    ; -ST(0)
0074:    DD 02                   fld      qword ptr [edx]
0076:    D9 E0                   fchs    ; -ST(0)
0078:    DD 19                   fstp     qword ptr [ecx]
007A:    DD 1A                   fstp     qword ptr [edx]
007C:    D9 E8                   fld1    ; Load 1.0
007E:    D9 5D F4                fstp     dword ptr [ebp - 0xc]
0081:    83 7F EC 00             cmp      dword ptr [edi - 0x14], 0
0085:    74 05                   je       0x8c    ; Conditional jump
0087:    E8 36 00 00 00          call     0xc2
008C:    DD 03                   fld      qword ptr [ebx]
008E:    DD 02                   fld      qword ptr [edx]
0090:    DE C1                   faddp    st(1)
0092:    DD 01                   fld      qword ptr [ecx]
0094:    90                      nop
0095:    DE C1                   faddp    st(1)
0097:    DD 47 F0                fld      qword ptr [edi - 0x10]
009A:    DE E9                   fsubp    st(1)
009C:    D9 C0                   fld      st(0)
009E:    90                      nop
009F:    D9 E4                   ftst
00A1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A3:    9E                      sahf
00A4:    DD D8                   fstp     st(0)
00A6:    72 14                   jb       0xbc    ; Conditional jump
00A8:    DC 0F                   fmul     qword ptr [edi]
00AA:    DD 03                   fld      qword ptr [ebx]
00AC:    D8 E1                   fsub     st(1)
00AE:    DD 1B                   fstp     qword ptr [ebx]
00B0:    DD 02                   fld      qword ptr [edx]
00B2:    D8 E1                   fsub     st(1)
00B4:    DD 1A                   fstp     qword ptr [edx]
00B6:    DD 01                   fld      qword ptr [ecx]
00B8:    D8 E1                   fsub     st(1)
00BA:    DD 19                   fstp     qword ptr [ecx]
00BC:    DD D8                   fstp     st(0)
00BE:    EB 2B                   jmp      0xeb
00C0:    90                      nop
00C1:    90                      nop
00C2:    DD 03                   fld      qword ptr [ebx]
00C4:    DD 02                   fld      qword ptr [edx]
00C6:    DE C1                   faddp    st(1)
00C8:    DD 01                   fld      qword ptr [ecx]
00CA:    DE C1                   faddp    st(1)
00CC:    DC 0F                   fmul     qword ptr [edi]
00CE:    D9 E0                   fchs    ; -ST(0)
00D0:    DD 03                   fld      qword ptr [ebx]
00D2:    D8 C1                   fadd     st(1)
00D4:    D9 E0                   fchs    ; -ST(0)
00D6:    DD 1B                   fstp     qword ptr [ebx]
00D8:    DD 02                   fld      qword ptr [edx]
00DA:    D8 C1                   fadd     st(1)
00DC:    D9 E0                   fchs    ; -ST(0)
00DE:    DD 1A                   fstp     qword ptr [edx]
00E0:    DD 01                   fld      qword ptr [ecx]
00E2:    D8 C1                   fadd     st(1)
00E4:    D9 E0                   fchs    ; -ST(0)
00E6:    DD 19                   fstp     qword ptr [ecx]
00E8:    DD D8                   fstp     st(0)
00EA:    C3                      ret
00EB:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
00EF:    74 38                   je       0x129    ; Conditional jump
00F1:    83 7D F4 00             cmp      dword ptr [ebp - 0xc], 0
00F5:    75 0C                   jne      0x103    ; Conditional jump
00F7:    DD 01                   fld      qword ptr [ecx]
00F9:    D9 E0                   fchs    ; -ST(0)
00FB:    DD 02                   fld      qword ptr [edx]
00FD:    D9 E0                   fchs    ; -ST(0)
00FF:    DD 19                   fstp     qword ptr [ecx]
0101:    DD 1A                   fstp     qword ptr [edx]
0103:    83 7D F8 00             cmp      dword ptr [ebp - 8], 0
0107:    75 0C                   jne      0x115    ; Conditional jump
0109:    DD 03                   fld      qword ptr [ebx]
010B:    D9 E0                   fchs    ; -ST(0)
010D:    DD 01                   fld      qword ptr [ecx]
010F:    D9 E0                   fchs    ; -ST(0)
0111:    DD 1B                   fstp     qword ptr [ebx]
0113:    DD 19                   fstp     qword ptr [ecx]
0115:    83 7D FC 00             cmp      dword ptr [ebp - 4], 0
0119:    75 0C                   jne      0x127    ; Conditional jump
011B:    DD 03                   fld      qword ptr [ebx]
011D:    D9 E0                   fchs    ; -ST(0)
011F:    DD 02                   fld      qword ptr [edx]
0121:    D9 E0                   fchs    ; -ST(0)
0123:    DD 1B                   fstp     qword ptr [ebx]
0125:    DD 1A                   fstp     qword ptr [edx]
0127:    D9 D0                   fnop
0129:    8B C3                   mov      eax, ebx
012B:    5F                      pop      edi
012C:    5E                      pop      esi
012D:    5B                      pop      ebx
012E:    8B E5                   mov      esp, ebp
0130:    5D                      pop      ebp
0131:    C2 08 00                ret      8
