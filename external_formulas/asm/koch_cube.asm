; Disassembly: koch_cube.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 370 bytes, decoded: 370 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 20 00 00 00       sub      esp, 0x20
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    DD 03                   fld      qword ptr [ebx]
0016:    D9 E1                   fabs    ; |ST(0)|
0018:    DC 4F 08                fmul     qword ptr [edi + 8]
001B:    DD 1B                   fstp     qword ptr [ebx]
001D:    90                      nop
001E:    DD 02                   fld      qword ptr [edx]
0020:    D9 E1                   fabs    ; |ST(0)|
0022:    DC 4F 08                fmul     qword ptr [edi + 8]
0025:    DD 1A                   fstp     qword ptr [edx]
0027:    90                      nop
0028:    DD 01                   fld      qword ptr [ecx]
002A:    D9 E1                   fabs    ; |ST(0)|
002C:    DC 4F 08                fmul     qword ptr [edi + 8]
002F:    DD 19                   fstp     qword ptr [ecx]
0031:    DD 41 08                fld      qword ptr [ecx + 8]
0034:    DC 4F 08                fmul     qword ptr [edi + 8]
0037:    DC 77 E8                fdiv     qword ptr [edi - 0x18]
003A:    DC 4F F0                fmul     qword ptr [edi - 0x10]
003D:    DD 59 08                fstp     qword ptr [ecx + 8]
0040:    90                      nop
0041:    D9 D0                   fnop
0043:    DD 03                   fld      qword ptr [ebx]
0045:    DD 02                   fld      qword ptr [edx]
0047:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0049:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004B:    80 E4 41                and      ah, 0x41
004E:    77 08                   ja       0x58    ; Conditional jump
0050:    DD 03                   fld      qword ptr [ebx]
0052:    DD 02                   fld      qword ptr [edx]
0054:    DD 1B                   fstp     qword ptr [ebx]
0056:    DD 1A                   fstp     qword ptr [edx]
0058:    DD 03                   fld      qword ptr [ebx]
005A:    DD 01                   fld      qword ptr [ecx]
005C:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
005E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0060:    80 E4 41                and      ah, 0x41
0063:    77 08                   ja       0x6d    ; Conditional jump
0065:    DD 03                   fld      qword ptr [ebx]
0067:    DD 01                   fld      qword ptr [ecx]
0069:    DD 1B                   fstp     qword ptr [ebx]
006B:    DD 19                   fstp     qword ptr [ecx]
006D:    DD 01                   fld      qword ptr [ecx]
006F:    DD 02                   fld      qword ptr [edx]
0071:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0073:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0075:    80 E4 41                and      ah, 0x41
0078:    76 08                   jbe      0x82    ; Conditional jump
007A:    DD 01                   fld      qword ptr [ecx]
007C:    DD 02                   fld      qword ptr [edx]
007E:    DD 19                   fstp     qword ptr [ecx]
0080:    DD 1A                   fstp     qword ptr [edx]
0082:    D9 D0                   fnop
0084:    DD 01                   fld      qword ptr [ecx]
0086:    DC 47 C8                fadd     qword ptr [edi - 0x38]
0089:    DD 02                   fld      qword ptr [edx]
008B:    DC 47 D0                fadd     qword ptr [edi - 0x30]
008E:    DD 03                   fld      qword ptr [ebx]
0090:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0093:    D9 C0                   fld      st(0)
0095:    D8 4F AC                fmul     dword ptr [edi - 0x54]
0098:    D9 C2                   fld      st(2)
009A:    D8 4F A8                fmul     dword ptr [edi - 0x58]
009D:    DE C1                   faddp    st(1)
009F:    D9 C3                   fld      st(3)
00A1:    D8 4F A4                fmul     dword ptr [edi - 0x5c]
00A4:    DE C1                   faddp    st(1)
00A6:    D9 C1                   fld      st(1)
00A8:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
00AB:    D9 C3                   fld      st(3)
00AD:    D8 4F C0                fmul     dword ptr [edi - 0x40]
00B0:    DE C1                   faddp    st(1)
00B2:    D9 C4                   fld      st(4)
00B4:    D8 4F BC                fmul     dword ptr [edi - 0x44]
00B7:    DE C1                   faddp    st(1)
00B9:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00BB:    D8 4F B8                fmul     dword ptr [edi - 0x48]
00BE:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00C0:    D8 4F B4                fmul     dword ptr [edi - 0x4c]
00C3:    DE C3                   faddp    st(3)
00C5:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00C7:    D8 4F B0                fmul     dword ptr [edi - 0x50]
00CA:    DE C2                   faddp    st(2)
00CC:    DD 1B                   fstp     qword ptr [ebx]
00CE:    DD 1A                   fstp     qword ptr [edx]
00D0:    DD 19                   fstp     qword ptr [ecx]
00D2:    90                      nop
00D3:    DD 47 E0                fld      qword ptr [edi - 0x20]
00D6:    DC 21                   fsub     qword ptr [ecx]
00D8:    D9 E1                   fabs    ; |ST(0)|
00DA:    D9 E0                   fchs    ; -ST(0)
00DC:    DC 47 E0                fadd     qword ptr [edi - 0x20]
00DF:    DD 19                   fstp     qword ptr [ecx]
00E1:    90                      nop
00E2:    DD 47 08                fld      qword ptr [edi + 8]
00E5:    D9 C0                   fld      st(0)
00E7:    DD 47 E8                fld      qword ptr [edi - 0x18]
00EA:    DE C1                   faddp    st(1)
00EC:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00EF:    DD 47 E8                fld      qword ptr [edi - 0x18]
00F2:    DE E9                   fsubp    st(1)
00F4:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00F7:    90                      nop
00F8:    DD 03                   fld      qword ptr [ebx]
00FA:    DC 65 E0                fsub     qword ptr [ebp - 0x20]
00FD:    DD 5D F8                fstp     qword ptr [ebp - 8]
0100:    90                      nop
0101:    DD 03                   fld      qword ptr [ebx]
0103:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
0106:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0109:    90                      nop
010A:    DD 45 F8                fld      qword ptr [ebp - 8]
010D:    DD 02                   fld      qword ptr [edx]
010F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0111:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0113:    80 E4 41                and      ah, 0x41
0116:    77 12                   ja       0x12a    ; Conditional jump
0118:    DD 45 F8                fld      qword ptr [ebp - 8]
011B:    DD 1B                   fstp     qword ptr [ebx]
011D:    DD 02                   fld      qword ptr [edx]
011F:    DC 65 E0                fsub     qword ptr [ebp - 0x20]
0122:    DD 1A                   fstp     qword ptr [edx]
0124:    EB 26                   jmp      0x14c
0126:    90                      nop
0127:    90                      nop
0128:    90                      nop
0129:    90                      nop
012A:    DD 45 F0                fld      qword ptr [ebp - 0x10]
012D:    DD 02                   fld      qword ptr [edx]
012F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0131:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0133:    80 E4 41                and      ah, 0x41
0136:    76 09                   jbe      0x141    ; Conditional jump
0138:    90                      nop
0139:    DD 45 F0                fld      qword ptr [ebp - 0x10]
013C:    DD 1B                   fstp     qword ptr [ebx]
013E:    EB 0C                   jmp      0x14c
0140:    90                      nop
0141:    DD 02                   fld      qword ptr [edx]
0143:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0146:    DD 1A                   fstp     qword ptr [edx]
0148:    DD 1B                   fstp     qword ptr [ebx]
014A:    D9 D0                   fnop
014C:    DD 03                   fld      qword ptr [ebx]
014E:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0151:    DC 77 E8                fdiv     qword ptr [edi - 0x18]
0154:    DD 1B                   fstp     qword ptr [ebx]
0156:    DD 02                   fld      qword ptr [edx]
0158:    DC 4F F0                fmul     qword ptr [edi - 0x10]
015B:    DC 77 E8                fdiv     qword ptr [edi - 0x18]
015E:    DD 1A                   fstp     qword ptr [edx]
0160:    DD 01                   fld      qword ptr [ecx]
0162:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0165:    DD 19                   fstp     qword ptr [ecx]
0167:    8B C3                   mov      eax, ebx
0169:    5F                      pop      edi
016A:    5E                      pop      esi
016B:    5B                      pop      ebx
016C:    89 EC                   mov      esp, ebp
016E:    5D                      pop      ebp
016F:    C2 08 00                ret      8
