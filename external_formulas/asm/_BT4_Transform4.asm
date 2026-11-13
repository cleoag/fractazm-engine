; Disassembly: _BT4_Transform4.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 203 bytes, decoded: 203 bytes (100.0%)
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
0029:    DD 02                   fld      qword ptr [edx]
002B:    DC 4F 10                fmul     qword ptr [edi + 0x10]
002E:    DD 1A                   fstp     qword ptr [edx]
0030:    90                      nop
0031:    DD 03                   fld      qword ptr [ebx]
0033:    DC 4F 08                fmul     qword ptr [edi + 8]
0036:    DD 01                   fld      qword ptr [ecx]
0038:    DC 0F                   fmul     qword ptr [edi]
003A:    DE C1                   faddp    st(1)
003C:    90                      nop
003D:    DD 45 F8                fld      qword ptr [ebp - 8]
0040:    DC 22                   fsub     qword ptr [edx]
0042:    90                      nop
0043:    DD 45 F8                fld      qword ptr [ebp - 8]
0046:    DC 02                   fadd     qword ptr [edx]
0048:    90                      nop
0049:    D8 C8                   fmul     st(0)
004B:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
004E:    D8 C8                   fmul     st(0)
0050:    DD 5D F8                fstp     qword ptr [ebp - 8]
0053:    D8 C8                   fmul     st(0)
0055:    DD 19                   fstp     qword ptr [ecx]
0057:    D9 D0                   fnop
0059:    DD 45 F0                fld      qword ptr [ebp - 0x10]
005C:    DC 01                   fadd     qword ptr [ecx]
005E:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0061:    D9 E1                   fabs    ; |ST(0)|
0063:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0066:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0069:    DD 1B                   fstp     qword ptr [ebx]
006B:    90                      nop
006C:    DD 45 F8                fld      qword ptr [ebp - 8]
006F:    DC 01                   fadd     qword ptr [ecx]
0071:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0074:    D9 E1                   fabs    ; |ST(0)|
0076:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0079:    DC 4F 10                fmul     qword ptr [edi + 0x10]
007C:    DD 1A                   fstp     qword ptr [edx]
007E:    90                      nop
007F:    DD 45 F8                fld      qword ptr [ebp - 8]
0082:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0085:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0088:    D9 E1                   fabs    ; |ST(0)|
008A:    DC 4F F0                fmul     qword ptr [edi - 0x10]
008D:    DD 19                   fstp     qword ptr [ecx]
008F:    90                      nop
0090:    DD 02                   fld      qword ptr [edx]
0092:    DC 03                   fadd     qword ptr [ebx]
0094:    DD 02                   fld      qword ptr [edx]
0096:    DC 23                   fsub     qword ptr [ebx]
0098:    DD 1A                   fstp     qword ptr [edx]
009A:    DD 5D F8                fstp     qword ptr [ebp - 8]
009D:    90                      nop
009E:    DD 45 F8                fld      qword ptr [ebp - 8]
00A1:    DC 0F                   fmul     qword ptr [edi]
00A3:    DD 01                   fld      qword ptr [ecx]
00A5:    DC 4F 08                fmul     qword ptr [edi + 8]
00A8:    DE C1                   faddp    st(1)
00AA:    DD 45 F8                fld      qword ptr [ebp - 8]
00AD:    DC 4F 08                fmul     qword ptr [edi + 8]
00B0:    D9 E0                   fchs    ; -ST(0)
00B2:    DD 01                   fld      qword ptr [ecx]
00B4:    DC 0F                   fmul     qword ptr [edi]
00B6:    90                      nop
00B7:    DE C1                   faddp    st(1)
00B9:    DD 19                   fstp     qword ptr [ecx]
00BB:    DD 1B                   fstp     qword ptr [ebx]
00BD:    90                      nop
00BE:    D9 D0                   fnop
00C0:    8B C3                   mov      eax, ebx
00C2:    5F                      pop      edi
00C3:    5E                      pop      esi
00C4:    5B                      pop      ebx
00C5:    89 EC                   mov      esp, ebp
00C7:    5D                      pop      ebp
00C8:    C2 08 00                ret      8
