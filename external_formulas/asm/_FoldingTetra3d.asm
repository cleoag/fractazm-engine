; Disassembly: _FoldingTetra3d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 194 bytes, decoded: 194 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    83 7F F4 00             cmp      dword ptr [edi - 0xc], 0
0012:    74 5F                   je       0x73    ; Conditional jump
0014:    90                      nop
0015:    90                      nop
0016:    90                      nop
0017:    90                      nop
0018:    DD 03                   fld      qword ptr [ebx]
001A:    DC 02                   fadd     qword ptr [edx]
001C:    D9 EE                   fldz    ; Load 0.0
001E:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0020:    90                      nop
0021:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0023:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0025:    72 0C                   jb       0x33    ; Conditional jump
0027:    DD 03                   fld      qword ptr [ebx]
0029:    D9 E0                   fchs    ; -ST(0)
002B:    DD 02                   fld      qword ptr [edx]
002D:    D9 E0                   fchs    ; -ST(0)
002F:    DD 1B                   fstp     qword ptr [ebx]
0031:    DD 1A                   fstp     qword ptr [edx]
0033:    DD 03                   fld      qword ptr [ebx]
0035:    DC 01                   fadd     qword ptr [ecx]
0037:    D9 EE                   fldz    ; Load 0.0
0039:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
003B:    90                      nop
003C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
003E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0040:    72 0C                   jb       0x4e    ; Conditional jump
0042:    DD 03                   fld      qword ptr [ebx]
0044:    D9 E0                   fchs    ; -ST(0)
0046:    DD 01                   fld      qword ptr [ecx]
0048:    D9 E0                   fchs    ; -ST(0)
004A:    DD 1B                   fstp     qword ptr [ebx]
004C:    DD 19                   fstp     qword ptr [ecx]
004E:    DD 01                   fld      qword ptr [ecx]
0050:    DC 02                   fadd     qword ptr [edx]
0052:    D9 EE                   fldz    ; Load 0.0
0054:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0056:    90                      nop
0057:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0059:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005B:    72 0C                   jb       0x69    ; Conditional jump
005D:    DD 01                   fld      qword ptr [ecx]
005F:    D9 E0                   fchs    ; -ST(0)
0061:    DD 02                   fld      qword ptr [edx]
0063:    D9 E0                   fchs    ; -ST(0)
0065:    DD 19                   fstp     qword ptr [ecx]
0067:    DD 1A                   fstp     qword ptr [edx]
0069:    83 7F F0 00             cmp      dword ptr [edi - 0x10], 0
006D:    74 49                   je       0xb8    ; Conditional jump
006F:    90                      nop
0070:    90                      nop
0071:    90                      nop
0072:    90                      nop
0073:    DD 03                   fld      qword ptr [ebx]
0075:    DC 22                   fsub     qword ptr [edx]
0077:    D9 EE                   fldz    ; Load 0.0
0079:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
007B:    90                      nop
007C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
007E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0080:    72 08                   jb       0x8a    ; Conditional jump
0082:    DD 03                   fld      qword ptr [ebx]
0084:    DD 02                   fld      qword ptr [edx]
0086:    DD 1B                   fstp     qword ptr [ebx]
0088:    DD 1A                   fstp     qword ptr [edx]
008A:    DD 03                   fld      qword ptr [ebx]
008C:    DC 21                   fsub     qword ptr [ecx]
008E:    D9 EE                   fldz    ; Load 0.0
0090:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0092:    90                      nop
0093:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0095:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0097:    72 08                   jb       0xa1    ; Conditional jump
0099:    DD 03                   fld      qword ptr [ebx]
009B:    DD 01                   fld      qword ptr [ecx]
009D:    DD 1B                   fstp     qword ptr [ebx]
009F:    DD 19                   fstp     qword ptr [ecx]
00A1:    DD 02                   fld      qword ptr [edx]
00A3:    DC 21                   fsub     qword ptr [ecx]
00A5:    D9 EE                   fldz    ; Load 0.0
00A7:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00A9:    90                      nop
00AA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00AE:    72 08                   jb       0xb8    ; Conditional jump
00B0:    DD 02                   fld      qword ptr [edx]
00B2:    DD 01                   fld      qword ptr [ecx]
00B4:    DD 1A                   fstp     qword ptr [edx]
00B6:    DD 19                   fstp     qword ptr [ecx]
00B8:    90                      nop
00B9:    89 D8                   mov      eax, ebx
00BB:    5B                      pop      ebx
00BC:    5F                      pop      edi
00BD:    5E                      pop      esi
00BE:    5D                      pop      ebp
00BF:    C2 08 00                ret      8
