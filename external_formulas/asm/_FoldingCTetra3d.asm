; Disassembly: _FoldingCTetra3d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 422 bytes, decoded: 422 bytes (100.0%)
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
0012:    74 71                   je       0x85    ; Conditional jump
0014:    90                      nop
0015:    90                      nop
0016:    90                      nop
0017:    90                      nop
0018:    DD 46 18                fld      qword ptr [esi + 0x18]
001B:    DC 46 20                fadd     qword ptr [esi + 0x20]
001E:    D9 EE                   fldz    ; Load 0.0
0020:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0022:    90                      nop
0023:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0025:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0027:    72 10                   jb       0x39    ; Conditional jump
0029:    DD 46 18                fld      qword ptr [esi + 0x18]
002C:    D9 E0                   fchs    ; -ST(0)
002E:    DD 46 20                fld      qword ptr [esi + 0x20]
0031:    D9 E0                   fchs    ; -ST(0)
0033:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0036:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0039:    DD 46 18                fld      qword ptr [esi + 0x18]
003C:    DC 46 28                fadd     qword ptr [esi + 0x28]
003F:    D9 EE                   fldz    ; Load 0.0
0041:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0043:    90                      nop
0044:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0046:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0048:    72 10                   jb       0x5a    ; Conditional jump
004A:    DD 46 18                fld      qword ptr [esi + 0x18]
004D:    D9 E0                   fchs    ; -ST(0)
004F:    DD 46 28                fld      qword ptr [esi + 0x28]
0052:    D9 E0                   fchs    ; -ST(0)
0054:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0057:    DD 5E 28                fstp     qword ptr [esi + 0x28]
005A:    DD 46 20                fld      qword ptr [esi + 0x20]
005D:    DC 46 28                fadd     qword ptr [esi + 0x28]
0060:    D9 EE                   fldz    ; Load 0.0
0062:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0064:    90                      nop
0065:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0067:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0069:    72 10                   jb       0x7b    ; Conditional jump
006B:    DD 46 20                fld      qword ptr [esi + 0x20]
006E:    D9 E0                   fchs    ; -ST(0)
0070:    DD 46 28                fld      qword ptr [esi + 0x28]
0073:    D9 E0                   fchs    ; -ST(0)
0075:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0078:    DD 5E 28                fstp     qword ptr [esi + 0x28]
007B:    83 7F F0 00             cmp      dword ptr [edi - 0x10], 0
007F:    74 67                   je       0xe8    ; Conditional jump
0081:    90                      nop
0082:    90                      nop
0083:    90                      nop
0084:    90                      nop
0085:    DD 46 18                fld      qword ptr [esi + 0x18]
0088:    DC 66 20                fsub     qword ptr [esi + 0x20]
008B:    D9 EE                   fldz    ; Load 0.0
008D:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
008F:    90                      nop
0090:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0092:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0094:    72 10                   jb       0xa6    ; Conditional jump
0096:    DD 46 18                fld      qword ptr [esi + 0x18]
0099:    90                      nop
009A:    90                      nop
009B:    DD 46 20                fld      qword ptr [esi + 0x20]
009E:    90                      nop
009F:    90                      nop
00A0:    DD 5E 18                fstp     qword ptr [esi + 0x18]
00A3:    DD 5E 20                fstp     qword ptr [esi + 0x20]
00A6:    DD 46 18                fld      qword ptr [esi + 0x18]
00A9:    DC 66 28                fsub     qword ptr [esi + 0x28]
00AC:    D9 EE                   fldz    ; Load 0.0
00AE:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00B0:    90                      nop
00B1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00B5:    72 10                   jb       0xc7    ; Conditional jump
00B7:    DD 46 18                fld      qword ptr [esi + 0x18]
00BA:    90                      nop
00BB:    90                      nop
00BC:    DD 46 28                fld      qword ptr [esi + 0x28]
00BF:    90                      nop
00C0:    90                      nop
00C1:    DD 5E 18                fstp     qword ptr [esi + 0x18]
00C4:    DD 5E 28                fstp     qword ptr [esi + 0x28]
00C7:    DD 46 20                fld      qword ptr [esi + 0x20]
00CA:    DC 66 28                fsub     qword ptr [esi + 0x28]
00CD:    D9 EE                   fldz    ; Load 0.0
00CF:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00D1:    90                      nop
00D2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00D4:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D6:    72 10                   jb       0xe8    ; Conditional jump
00D8:    DD 46 20                fld      qword ptr [esi + 0x20]
00DB:    90                      nop
00DC:    90                      nop
00DD:    DD 46 28                fld      qword ptr [esi + 0x28]
00E0:    90                      nop
00E1:    90                      nop
00E2:    DD 5E 20                fstp     qword ptr [esi + 0x20]
00E5:    DD 5E 28                fstp     qword ptr [esi + 0x28]
00E8:    83 7F EC 00             cmp      dword ptr [edi - 0x14], 0
00EC:    0F 84 AB 00 00 00       je       0x19d    ; Conditional jump
00F2:    83 7F F4 00             cmp      dword ptr [edi - 0xc], 0
00F6:    74 5F                   je       0x157    ; Conditional jump
00F8:    90                      nop
00F9:    90                      nop
00FA:    90                      nop
00FB:    90                      nop
00FC:    DD 03                   fld      qword ptr [ebx]
00FE:    DC 02                   fadd     qword ptr [edx]
0100:    D9 EE                   fldz    ; Load 0.0
0102:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0104:    90                      nop
0105:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0107:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0109:    72 0C                   jb       0x117    ; Conditional jump
010B:    DD 03                   fld      qword ptr [ebx]
010D:    D9 E0                   fchs    ; -ST(0)
010F:    DD 02                   fld      qword ptr [edx]
0111:    D9 E0                   fchs    ; -ST(0)
0113:    DD 1B                   fstp     qword ptr [ebx]
0115:    DD 1A                   fstp     qword ptr [edx]
0117:    DD 03                   fld      qword ptr [ebx]
0119:    DC 01                   fadd     qword ptr [ecx]
011B:    D9 EE                   fldz    ; Load 0.0
011D:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
011F:    90                      nop
0120:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0122:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0124:    72 0C                   jb       0x132    ; Conditional jump
0126:    DD 03                   fld      qword ptr [ebx]
0128:    D9 E0                   fchs    ; -ST(0)
012A:    DD 01                   fld      qword ptr [ecx]
012C:    D9 E0                   fchs    ; -ST(0)
012E:    DD 1B                   fstp     qword ptr [ebx]
0130:    DD 19                   fstp     qword ptr [ecx]
0132:    DD 01                   fld      qword ptr [ecx]
0134:    DC 02                   fadd     qword ptr [edx]
0136:    D9 EE                   fldz    ; Load 0.0
0138:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
013A:    90                      nop
013B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
013D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
013F:    72 0C                   jb       0x14d    ; Conditional jump
0141:    DD 01                   fld      qword ptr [ecx]
0143:    D9 E0                   fchs    ; -ST(0)
0145:    DD 02                   fld      qword ptr [edx]
0147:    D9 E0                   fchs    ; -ST(0)
0149:    DD 19                   fstp     qword ptr [ecx]
014B:    DD 1A                   fstp     qword ptr [edx]
014D:    83 7F F0 00             cmp      dword ptr [edi - 0x10], 0
0151:    74 49                   je       0x19c    ; Conditional jump
0153:    90                      nop
0154:    90                      nop
0155:    90                      nop
0156:    90                      nop
0157:    DD 03                   fld      qword ptr [ebx]
0159:    DC 22                   fsub     qword ptr [edx]
015B:    D9 EE                   fldz    ; Load 0.0
015D:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
015F:    90                      nop
0160:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0162:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0164:    72 08                   jb       0x16e    ; Conditional jump
0166:    DD 03                   fld      qword ptr [ebx]
0168:    DD 02                   fld      qword ptr [edx]
016A:    DD 1B                   fstp     qword ptr [ebx]
016C:    DD 1A                   fstp     qword ptr [edx]
016E:    DD 03                   fld      qword ptr [ebx]
0170:    DC 21                   fsub     qword ptr [ecx]
0172:    D9 EE                   fldz    ; Load 0.0
0174:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0176:    90                      nop
0177:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0179:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
017B:    72 08                   jb       0x185    ; Conditional jump
017D:    DD 03                   fld      qword ptr [ebx]
017F:    DD 01                   fld      qword ptr [ecx]
0181:    DD 1B                   fstp     qword ptr [ebx]
0183:    DD 19                   fstp     qword ptr [ecx]
0185:    DD 02                   fld      qword ptr [edx]
0187:    DC 21                   fsub     qword ptr [ecx]
0189:    D9 EE                   fldz    ; Load 0.0
018B:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
018D:    90                      nop
018E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0190:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0192:    72 08                   jb       0x19c    ; Conditional jump
0194:    DD 02                   fld      qword ptr [edx]
0196:    DD 01                   fld      qword ptr [ecx]
0198:    DD 1A                   fstp     qword ptr [edx]
019A:    DD 19                   fstp     qword ptr [ecx]
019C:    90                      nop
019D:    89 D8                   mov      eax, ebx
019F:    5B                      pop      ebx
01A0:    5F                      pop      edi
01A1:    5E                      pop      esi
01A2:    5D                      pop      ebp
01A3:    C2 08 00                ret      8
