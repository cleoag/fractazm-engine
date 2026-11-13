; Disassembly: _BenesiT1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 184 bytes, decoded: 184 bytes (100.0%)
; DEoption: -1

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
00AB:    D9 D0                   fnop
00AD:    8B C3                   mov      eax, ebx
00AF:    5F                      pop      edi
00B0:    5E                      pop      esi
00B1:    5B                      pop      ebx
00B2:    89 EC                   mov      esp, ebp
00B4:    5D                      pop      ebp
00B5:    C2 08 00                ret      8
