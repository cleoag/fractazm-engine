; Disassembly: Barnsley2_4Dnc.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 316 bytes, decoded: 316 bytes (100.0%)
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
0012:    DD 46 20                fld      qword ptr [esi + 0x20]
0015:    DE C9                   fmulp    st(1)
0017:    DC 4F C8                fmul     qword ptr [edi - 0x38]
001A:    DD 02                   fld      qword ptr [edx]
001C:    DD 46 18                fld      qword ptr [esi + 0x18]
001F:    DE C9                   fmulp    st(1)
0021:    DC 4F C0                fmul     qword ptr [edi - 0x40]
0024:    DE C1                   faddp    st(1)
0026:    90                      nop
0027:    DD 47 B8                fld      qword ptr [edi - 0x48]
002A:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
002C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
002E:    80 E4 41                and      ah, 0x41
0031:    90                      nop
0032:    90                      nop
0033:    90                      nop
0034:    90                      nop
0035:    77 2B                   ja       0x62    ; Conditional jump
0037:    90                      nop
0038:    90                      nop
0039:    90                      nop
003A:    90                      nop
003B:    DD 03                   fld      qword ptr [ebx]
003D:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0040:    DD 1B                   fstp     qword ptr [ebx]
0042:    DD 02                   fld      qword ptr [edx]
0044:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0047:    DD 1A                   fstp     qword ptr [edx]
0049:    DD 01                   fld      qword ptr [ecx]
004B:    DC 47 E0                fadd     qword ptr [edi - 0x20]
004E:    DD 19                   fstp     qword ptr [ecx]
0050:    DD 41 08                fld      qword ptr [ecx + 8]
0053:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0056:    DC 47 D0                fadd     qword ptr [edi - 0x30]
0059:    DD 59 08                fstp     qword ptr [ecx + 8]
005C:    EB 29                   jmp      0x87
005E:    90                      nop
005F:    90                      nop
0060:    90                      nop
0061:    90                      nop
0062:    DD 03                   fld      qword ptr [ebx]
0064:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0067:    DD 1B                   fstp     qword ptr [ebx]
0069:    DD 02                   fld      qword ptr [edx]
006B:    DC 67 E8                fsub     qword ptr [edi - 0x18]
006E:    DD 1A                   fstp     qword ptr [edx]
0070:    DD 01                   fld      qword ptr [ecx]
0072:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0075:    DD 19                   fstp     qword ptr [ecx]
0077:    DD 41 08                fld      qword ptr [ecx + 8]
007A:    DC 67 D8                fsub     qword ptr [edi - 0x28]
007D:    DC 47 D0                fadd     qword ptr [edi - 0x30]
0080:    DD 59 08                fstp     qword ptr [ecx + 8]
0083:    90                      nop
0084:    90                      nop
0085:    90                      nop
0086:    90                      nop
0087:    DD 03                   fld      qword ptr [ebx]
0089:    90                      nop
008A:    DC 4E 18                fmul     qword ptr [esi + 0x18]
008D:    90                      nop
008E:    90                      nop
008F:    DD 02                   fld      qword ptr [edx]
0091:    90                      nop
0092:    DC 4E 20                fmul     qword ptr [esi + 0x20]
0095:    D9 E0                   fchs    ; -ST(0)
0097:    DE C1                   faddp    st(1)
0099:    DD 01                   fld      qword ptr [ecx]
009B:    90                      nop
009C:    DC 4E 28                fmul     qword ptr [esi + 0x28]
009F:    D9 E0                   fchs    ; -ST(0)
00A1:    DE C1                   faddp    st(1)
00A3:    DD 41 08                fld      qword ptr [ecx + 8]
00A6:    DC 4E C8                fmul     qword ptr [esi - 0x38]
00A9:    D9 E0                   fchs    ; -ST(0)
00AB:    DE C1                   faddp    st(1)
00AD:    D9 D0                   fnop
00AF:    DD 03                   fld      qword ptr [ebx]
00B1:    90                      nop
00B2:    DC 4E 20                fmul     qword ptr [esi + 0x20]
00B5:    90                      nop
00B6:    90                      nop
00B7:    DD 02                   fld      qword ptr [edx]
00B9:    90                      nop
00BA:    DC 4E 18                fmul     qword ptr [esi + 0x18]
00BD:    90                      nop
00BE:    90                      nop
00BF:    DE C1                   faddp    st(1)
00C1:    DD 01                   fld      qword ptr [ecx]
00C3:    90                      nop
00C4:    DC 4E C8                fmul     qword ptr [esi - 0x38]
00C7:    90                      nop
00C8:    90                      nop
00C9:    DE C1                   faddp    st(1)
00CB:    DD 41 08                fld      qword ptr [ecx + 8]
00CE:    DC 4E 28                fmul     qword ptr [esi + 0x28]
00D1:    D9 E0                   fchs    ; -ST(0)
00D3:    DE C1                   faddp    st(1)
00D5:    D9 D0                   fnop
00D7:    DD 03                   fld      qword ptr [ebx]
00D9:    90                      nop
00DA:    DC 4E 28                fmul     qword ptr [esi + 0x28]
00DD:    90                      nop
00DE:    90                      nop
00DF:    DD 02                   fld      qword ptr [edx]
00E1:    90                      nop
00E2:    DC 4E C8                fmul     qword ptr [esi - 0x38]
00E5:    D9 E0                   fchs    ; -ST(0)
00E7:    DE C1                   faddp    st(1)
00E9:    DD 01                   fld      qword ptr [ecx]
00EB:    90                      nop
00EC:    DC 4E 18                fmul     qword ptr [esi + 0x18]
00EF:    90                      nop
00F0:    90                      nop
00F1:    DE C1                   faddp    st(1)
00F3:    DD 41 08                fld      qword ptr [ecx + 8]
00F6:    DC 4E 20                fmul     qword ptr [esi + 0x20]
00F9:    90                      nop
00FA:    90                      nop
00FB:    DE C1                   faddp    st(1)
00FD:    D9 D0                   fnop
00FF:    DD 03                   fld      qword ptr [ebx]
0101:    90                      nop
0102:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0105:    90                      nop
0106:    90                      nop
0107:    DD 02                   fld      qword ptr [edx]
0109:    90                      nop
010A:    DC 4E 28                fmul     qword ptr [esi + 0x28]
010D:    90                      nop
010E:    90                      nop
010F:    DE C1                   faddp    st(1)
0111:    DD 01                   fld      qword ptr [ecx]
0113:    90                      nop
0114:    DC 4E 20                fmul     qword ptr [esi + 0x20]
0117:    D9 E0                   fchs    ; -ST(0)
0119:    DE C1                   faddp    st(1)
011B:    DD 41 08                fld      qword ptr [ecx + 8]
011E:    DC 4E 18                fmul     qword ptr [esi + 0x18]
0121:    90                      nop
0122:    90                      nop
0123:    DE C1                   faddp    st(1)
0125:    D9 D0                   fnop
0127:    90                      nop
0128:    DD 59 08                fstp     qword ptr [ecx + 8]
012B:    DD 19                   fstp     qword ptr [ecx]
012D:    DD 1A                   fstp     qword ptr [edx]
012F:    DD 1B                   fstp     qword ptr [ebx]
0131:    90                      nop
0132:    8B C3                   mov      eax, ebx
0134:    5F                      pop      edi
0135:    5E                      pop      esi
0136:    58                      pop      eax
0137:    5B                      pop      ebx
0138:    5D                      pop      ebp
0139:    C2 08 00                ret      8
