; Disassembly: Barnsley4Dc2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 420 bytes, decoded: 420 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 30 00 00 00       sub      esp, 0x30
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    DD 03                   fld      qword ptr [ebx]
0016:    DC 61 08                fsub     qword ptr [ecx + 8]
0019:    90                      nop
001A:    DD 02                   fld      qword ptr [edx]
001C:    DC 01                   fadd     qword ptr [ecx]
001E:    90                      nop
001F:    90                      nop
0020:    DD 03                   fld      qword ptr [ebx]
0022:    DC 41 08                fadd     qword ptr [ecx + 8]
0025:    90                      nop
0026:    DD 02                   fld      qword ptr [edx]
0028:    DC 21                   fsub     qword ptr [ecx]
002A:    90                      nop
002B:    90                      nop
002C:    DD 59 08                fstp     qword ptr [ecx + 8]
002F:    DD 19                   fstp     qword ptr [ecx]
0031:    DD 1A                   fstp     qword ptr [edx]
0033:    DD 1B                   fstp     qword ptr [ebx]
0035:    90                      nop
0036:    90                      nop
0037:    DD 03                   fld      qword ptr [ebx]
0039:    DD 47 C8                fld      qword ptr [edi - 0x38]
003C:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
003E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0040:    80 E4 41                and      ah, 0x41
0043:    77 14                   ja       0x59    ; Conditional jump
0045:    90                      nop
0046:    90                      nop
0047:    90                      nop
0048:    90                      nop
0049:    DD 03                   fld      qword ptr [ebx]
004B:    DC 47 F0                fadd     qword ptr [edi - 0x10]
004E:    DD 1B                   fstp     qword ptr [ebx]
0050:    DD 02                   fld      qword ptr [edx]
0052:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0055:    DD 1A                   fstp     qword ptr [edx]
0057:    EB 0E                   jmp      0x67
0059:    DD 03                   fld      qword ptr [ebx]
005B:    DC 67 F0                fsub     qword ptr [edi - 0x10]
005E:    DD 1B                   fstp     qword ptr [ebx]
0060:    DD 02                   fld      qword ptr [edx]
0062:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0065:    DD 1A                   fstp     qword ptr [edx]
0067:    DD 01                   fld      qword ptr [ecx]
0069:    DD 47 C0                fld      qword ptr [edi - 0x40]
006C:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
006E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0070:    80 E4 41                and      ah, 0x41
0073:    77 16                   ja       0x8b    ; Conditional jump
0075:    90                      nop
0076:    90                      nop
0077:    90                      nop
0078:    90                      nop
0079:    DD 01                   fld      qword ptr [ecx]
007B:    DC 47 E0                fadd     qword ptr [edi - 0x20]
007E:    DD 19                   fstp     qword ptr [ecx]
0080:    DD 41 08                fld      qword ptr [ecx + 8]
0083:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0086:    DD 59 08                fstp     qword ptr [ecx + 8]
0089:    EB 10                   jmp      0x9b
008B:    DD 01                   fld      qword ptr [ecx]
008D:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0090:    DD 19                   fstp     qword ptr [ecx]
0092:    DD 41 08                fld      qword ptr [ecx + 8]
0095:    DC 67 D8                fsub     qword ptr [edi - 0x28]
0098:    DD 59 08                fstp     qword ptr [ecx + 8]
009B:    D9 D0                   fnop
009D:    DD 46 18                fld      qword ptr [esi + 0x18]
00A0:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00A3:    DD 46 20                fld      qword ptr [esi + 0x20]
00A6:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00A9:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
00AD:    74 0C                   je       0xbb    ; Conditional jump
00AF:    90                      nop
00B0:    90                      nop
00B1:    90                      nop
00B2:    90                      nop
00B3:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00B6:    D9 E0                   fchs    ; -ST(0)
00B8:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00BB:    83 7F B8 00             cmp      dword ptr [edi - 0x48], 0
00BF:    74 10                   je       0xd1    ; Conditional jump
00C1:    90                      nop
00C2:    90                      nop
00C3:    90                      nop
00C4:    90                      nop
00C5:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00C8:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00CB:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00CE:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00D1:    D9 D0                   fnop
00D3:    DD 03                   fld      qword ptr [ebx]
00D5:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00D8:    90                      nop
00D9:    DD 02                   fld      qword ptr [edx]
00DB:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00DE:    D9 E0                   fchs    ; -ST(0)
00E0:    DE C1                   faddp    st(1)
00E2:    DD 03                   fld      qword ptr [ebx]
00E4:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00E7:    90                      nop
00E8:    DD 02                   fld      qword ptr [edx]
00EA:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00ED:    90                      nop
00EE:    90                      nop
00EF:    DE C1                   faddp    st(1)
00F1:    DD 1A                   fstp     qword ptr [edx]
00F3:    DD 1B                   fstp     qword ptr [ebx]
00F5:    D9 D0                   fnop
00F7:    DD 46 28                fld      qword ptr [esi + 0x28]
00FA:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00FD:    DD 46 C8                fld      qword ptr [esi - 0x38]
0100:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0103:    83 7F BC 00             cmp      dword ptr [edi - 0x44], 0
0107:    74 0C                   je       0x115    ; Conditional jump
0109:    90                      nop
010A:    90                      nop
010B:    90                      nop
010C:    90                      nop
010D:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0110:    D9 E0                   fchs    ; -ST(0)
0112:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0115:    83 7F B8 00             cmp      dword ptr [edi - 0x48], 0
0119:    74 10                   je       0x12b    ; Conditional jump
011B:    90                      nop
011C:    90                      nop
011D:    90                      nop
011E:    90                      nop
011F:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0122:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0125:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0128:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
012B:    D9 D0                   fnop
012D:    DD 01                   fld      qword ptr [ecx]
012F:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0132:    DD 41 08                fld      qword ptr [ecx + 8]
0135:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0138:    D9 E0                   fchs    ; -ST(0)
013A:    DE C1                   faddp    st(1)
013C:    DD 03                   fld      qword ptr [ebx]
013E:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0141:    DD 41 08                fld      qword ptr [ecx + 8]
0144:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0147:    90                      nop
0148:    90                      nop
0149:    DE C1                   faddp    st(1)
014B:    DC 47 D0                fadd     qword ptr [edi - 0x30]
014E:    DD 59 08                fstp     qword ptr [ecx + 8]
0151:    DD 19                   fstp     qword ptr [ecx]
0153:    90                      nop
0154:    D9 D0                   fnop
0156:    DD 03                   fld      qword ptr [ebx]
0158:    DC 01                   fadd     qword ptr [ecx]
015A:    DC 0F                   fmul     qword ptr [edi]
015C:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
015F:    DD 02                   fld      qword ptr [edx]
0161:    DC 41 08                fadd     qword ptr [ecx + 8]
0164:    DC 0F                   fmul     qword ptr [edi]
0166:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0169:    DD 02                   fld      qword ptr [edx]
016B:    DC 61 08                fsub     qword ptr [ecx + 8]
016E:    DC 0F                   fmul     qword ptr [edi]
0170:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0173:    90                      nop
0174:    90                      nop
0175:    DD 03                   fld      qword ptr [ebx]
0177:    D9 E0                   fchs    ; -ST(0)
0179:    DC 01                   fadd     qword ptr [ecx]
017B:    90                      nop
017C:    DC 0F                   fmul     qword ptr [edi]
017E:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0181:    90                      nop
0182:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0185:    DD 59 08                fstp     qword ptr [ecx + 8]
0188:    DD 45 E0                fld      qword ptr [ebp - 0x20]
018B:    DD 19                   fstp     qword ptr [ecx]
018D:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0190:    DD 1A                   fstp     qword ptr [edx]
0192:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0195:    DD 1B                   fstp     qword ptr [ebx]
0197:    D9 D0                   fnop
0199:    8B C3                   mov      eax, ebx
019B:    5F                      pop      edi
019C:    5E                      pop      esi
019D:    5B                      pop      ebx
019E:    89 EC                   mov      esp, ebp
01A0:    5D                      pop      ebp
01A1:    C2 08 00                ret      8
