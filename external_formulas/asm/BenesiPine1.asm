; Disassembly: BenesiPine1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 272 bytes, decoded: 272 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 30 00 00 00       sub      esp, 0x30
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 03                   fld      qword ptr [ebx]
0018:    DC 0F                   fmul     qword ptr [edi]
001A:    DD 01                   fld      qword ptr [ecx]
001C:    DC 4F 08                fmul     qword ptr [edi + 8]
001F:    D9 E0                   fchs    ; -ST(0)
0021:    DE C1                   faddp    st(1)
0023:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0026:    DD 5D F8                fstp     qword ptr [ebp - 8]
0029:    DD 03                   fld      qword ptr [ebx]
002B:    DC 4F 08                fmul     qword ptr [edi + 8]
002E:    DD 01                   fld      qword ptr [ecx]
0030:    DC 0F                   fmul     qword ptr [edi]
0032:    DE C1                   faddp    st(1)
0034:    D9 E1                   fabs    ; |ST(0)|
0036:    90                      nop
0037:    DD 45 F8                fld      qword ptr [ebp - 8]
003A:    DD 02                   fld      qword ptr [edx]
003C:    DC 4F 10                fmul     qword ptr [edi + 0x10]
003F:    D9 E0                   fchs    ; -ST(0)
0041:    DE C1                   faddp    st(1)
0043:    D9 E1                   fabs    ; |ST(0)|
0045:    90                      nop
0046:    DD 45 F8                fld      qword ptr [ebp - 8]
0049:    DD 02                   fld      qword ptr [edx]
004B:    DC 4F 10                fmul     qword ptr [edi + 0x10]
004E:    DE C1                   faddp    st(1)
0050:    D9 E1                   fabs    ; |ST(0)|
0052:    90                      nop
0053:    DD 1A                   fstp     qword ptr [edx]
0055:    DD 1B                   fstp     qword ptr [ebx]
0057:    DD 19                   fstp     qword ptr [ecx]
0059:    90                      nop
005A:    DD 03                   fld      qword ptr [ebx]
005C:    DC 02                   fadd     qword ptr [edx]
005E:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0061:    DD 5D F8                fstp     qword ptr [ebp - 8]
0064:    90                      nop
0065:    DD 03                   fld      qword ptr [ebx]
0067:    D9 E0                   fchs    ; -ST(0)
0069:    DC 02                   fadd     qword ptr [edx]
006B:    DC 4F 10                fmul     qword ptr [edi + 0x10]
006E:    90                      nop
006F:    DD 45 F8                fld      qword ptr [ebp - 8]
0072:    DC 0F                   fmul     qword ptr [edi]
0074:    DD 01                   fld      qword ptr [ecx]
0076:    DC 4F 08                fmul     qword ptr [edi + 8]
0079:    DE C1                   faddp    st(1)
007B:    90                      nop
007C:    DD 45 F8                fld      qword ptr [ebp - 8]
007F:    DC 4F 08                fmul     qword ptr [edi + 8]
0082:    D9 E0                   fchs    ; -ST(0)
0084:    DD 01                   fld      qword ptr [ecx]
0086:    DC 0F                   fmul     qword ptr [edi]
0088:    DE C1                   faddp    st(1)
008A:    90                      nop
008B:    DD 19                   fstp     qword ptr [ecx]
008D:    DD 1B                   fstp     qword ptr [ebx]
008F:    DD 1A                   fstp     qword ptr [edx]
0091:    90                      nop
0092:    DD 03                   fld      qword ptr [ebx]
0094:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0097:    DC 67 E8                fsub     qword ptr [edi - 0x18]
009A:    DD 1B                   fstp     qword ptr [ebx]
009C:    DD 02                   fld      qword ptr [edx]
009E:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00A1:    DD 1A                   fstp     qword ptr [edx]
00A3:    DD 01                   fld      qword ptr [ecx]
00A5:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00A8:    DD 19                   fstp     qword ptr [ecx]
00AA:    90                      nop
00AB:    DD 03                   fld      qword ptr [ebx]
00AD:    D8 C8                   fmul     st(0)
00AF:    DD 5D F8                fstp     qword ptr [ebp - 8]
00B2:    DD 02                   fld      qword ptr [edx]
00B4:    D8 C8                   fmul     st(0)
00B6:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00B9:    DD 01                   fld      qword ptr [ecx]
00BB:    D8 C8                   fmul     st(0)
00BD:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00C0:    DD 03                   fld      qword ptr [ebx]
00C2:    D8 C0                   fadd     st(0)
00C4:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00C7:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
00CA:    D9 FA                   fsqrt    ; sqrt(ST(0))
00CC:    DE F9                   fdivp    st(1)
00CE:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00D1:    90                      nop
00D2:    DD 45 F8                fld      qword ptr [ebp - 8]
00D5:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
00D8:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
00DB:    90                      nop
00DC:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00DF:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00E2:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
00E5:    DE C9                   fmulp    st(1)
00E7:    90                      nop
00E8:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00EB:    D8 C0                   fadd     st(0)
00ED:    DC 09                   fmul     qword ptr [ecx]
00EF:    DC 0A                   fmul     qword ptr [edx]
00F1:    90                      nop
00F2:    DC 46 20                fadd     qword ptr [esi + 0x20]
00F5:    DD 1A                   fstp     qword ptr [edx]
00F7:    90                      nop
00F8:    DC 46 28                fadd     qword ptr [esi + 0x28]
00FB:    DD 19                   fstp     qword ptr [ecx]
00FD:    90                      nop
00FE:    DC 46 18                fadd     qword ptr [esi + 0x18]
0101:    DD 1B                   fstp     qword ptr [ebx]
0103:    D9 D0                   fnop
0105:    8B C3                   mov      eax, ebx
0107:    5F                      pop      edi
0108:    5E                      pop      esi
0109:    5B                      pop      ebx
010A:    89 EC                   mov      esp, ebp
010C:    5D                      pop      ebp
010D:    C2 08 00                ret      8
