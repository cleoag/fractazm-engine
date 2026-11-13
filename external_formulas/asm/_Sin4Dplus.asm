; Disassembly: _Sin4Dplus.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 258 bytes, decoded: 258 bytes (100.0%)
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
004B:    80 E4 41                and      ah, 0x41
004E:    72 7F                   jb       0xcf    ; Conditional jump
0050:    90                      nop
0051:    90                      nop
0052:    90                      nop
0053:    90                      nop
0054:    90                      nop
0055:    90                      nop
0056:    90                      nop
0057:    90                      nop
0058:    90                      nop
0059:    90                      nop
005A:    90                      nop
005B:    DD 03                   fld      qword ptr [ebx]
005D:    D9 FB                   fsincos    ; ST(0)=cos, push sin
005F:    D9 D0                   fnop
0061:    D9 C2                   fld      st(2)
0063:    D9 EA                   fldl2e    ; Load log2(e)
0065:    DE C9                   fmulp    st(1)
0067:    D9 E8                   fld1    ; Load 1.0
0069:    D9 C1                   fld      st(1)
006B:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
006D:    D9 F0                   f2xm1
006F:    DE C1                   faddp    st(1)
0071:    D9 FD                   fscale
0073:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0075:    DD D8                   fstp     st(0)
0077:    90                      nop
0078:    D9 C0                   fld      st(0)
007A:    D9 E8                   fld1    ; Load 1.0
007C:    D9 C1                   fld      st(1)
007E:    DE F9                   fdivp    st(1)
0080:    D9 E0                   fchs    ; -ST(0)
0082:    DE C1                   faddp    st(1)
0084:    DC 0F                   fmul     qword ptr [edi]
0086:    90                      nop
0087:    90                      nop
0088:    90                      nop
0089:    D9 C1                   fld      st(1)
008B:    D9 E8                   fld1    ; Load 1.0
008D:    D9 C1                   fld      st(1)
008F:    DE F9                   fdivp    st(1)
0091:    D9 D0                   fnop
0093:    DE C1                   faddp    st(1)
0095:    DC 0F                   fmul     qword ptr [edi]
0097:    90                      nop
0098:    90                      nop
0099:    90                      nop
009A:    DD DA                   fstp     st(2)
009C:    D9 C3                   fld      st(3)
009E:    D8 CA                   fmul     st(2)
00A0:    DD 1B                   fstp     qword ptr [ebx]
00A2:    D9 C2                   fld      st(2)
00A4:    D8 C9                   fmul     st(1)
00A6:    D8 F5                   fdiv     st(5)
00A8:    DD D9                   fstp     st(1)
00AA:    DD D9                   fstp     st(1)
00AC:    DD D9                   fstp     st(1)
00AE:    DD D9                   fstp     st(1)
00B0:    DD D9                   fstp     st(1)
00B2:    90                      nop
00B3:    90                      nop
00B4:    DD 02                   fld      qword ptr [edx]
00B6:    D8 C9                   fmul     st(1)
00B8:    DD 1A                   fstp     qword ptr [edx]
00BA:    DD 01                   fld      qword ptr [ecx]
00BC:    D8 C9                   fmul     st(1)
00BE:    DD 19                   fstp     qword ptr [ecx]
00C0:    DD 41 08                fld      qword ptr [ecx + 8]
00C3:    D8 C9                   fmul     st(1)
00C5:    DD 59 08                fstp     qword ptr [ecx + 8]
00C8:    DD D8                   fstp     st(0)
00CA:    DD D8                   fstp     st(0)
00CC:    90                      nop
00CD:    EB 09                   jmp      0xd8
00CF:    DD 03                   fld      qword ptr [ebx]
00D1:    D9 FE                   fsin    ; sin(ST(0))
00D3:    DD 1B                   fstp     qword ptr [ebx]
00D5:    DD D8                   fstp     st(0)
00D7:    90                      nop
00D8:    DD 03                   fld      qword ptr [ebx]
00DA:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00DD:    DD 1B                   fstp     qword ptr [ebx]
00DF:    DD 02                   fld      qword ptr [edx]
00E1:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00E4:    DD 1A                   fstp     qword ptr [edx]
00E6:    DD 01                   fld      qword ptr [ecx]
00E8:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00EB:    DD 19                   fstp     qword ptr [ecx]
00ED:    DD 41 08                fld      qword ptr [ecx + 8]
00F0:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00F3:    DD 59 08                fstp     qword ptr [ecx + 8]
00F6:    90                      nop
00F7:    90                      nop
00F8:    8B C3                   mov      eax, ebx
00FA:    5F                      pop      edi
00FB:    5E                      pop      esi
00FC:    58                      pop      eax
00FD:    5B                      pop      ebx
00FE:    5D                      pop      ebp
00FF:    C2 08 00                ret      8
