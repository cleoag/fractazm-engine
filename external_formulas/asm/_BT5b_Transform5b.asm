; Disassembly: _BT5b_Transform5b.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 227 bytes, decoded: 227 bytes (100.0%)
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
004B:    D8 C8                   fmul     st(0)
004D:    D8 C8                   fmul     st(0)
004F:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0052:    D8 C8                   fmul     st(0)
0054:    D8 C8                   fmul     st(0)
0056:    D8 C8                   fmul     st(0)
0058:    DD 5D F8                fstp     qword ptr [ebp - 8]
005B:    D8 C8                   fmul     st(0)
005D:    D8 C8                   fmul     st(0)
005F:    D8 C8                   fmul     st(0)
0061:    DD 19                   fstp     qword ptr [ecx]
0063:    D9 D0                   fnop
0065:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0068:    DC 01                   fadd     qword ptr [ecx]
006A:    D9 FA                   fsqrt    ; sqrt(ST(0))
006C:    D9 FA                   fsqrt    ; sqrt(ST(0))
006E:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0071:    D9 E1                   fabs    ; |ST(0)|
0073:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0076:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0079:    DD 1B                   fstp     qword ptr [ebx]
007B:    90                      nop
007C:    DD 45 F8                fld      qword ptr [ebp - 8]
007F:    DC 01                   fadd     qword ptr [ecx]
0081:    D9 FA                   fsqrt    ; sqrt(ST(0))
0083:    D9 FA                   fsqrt    ; sqrt(ST(0))
0085:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0088:    D9 E1                   fabs    ; |ST(0)|
008A:    DC 4F F0                fmul     qword ptr [edi - 0x10]
008D:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0090:    DD 1A                   fstp     qword ptr [edx]
0092:    90                      nop
0093:    DD 45 F8                fld      qword ptr [ebp - 8]
0096:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0099:    D9 FA                   fsqrt    ; sqrt(ST(0))
009B:    D9 FA                   fsqrt    ; sqrt(ST(0))
009D:    DC 67 E8                fsub     qword ptr [edi - 0x18]
00A0:    D9 E1                   fabs    ; |ST(0)|
00A2:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00A5:    DD 19                   fstp     qword ptr [ecx]
00A7:    90                      nop
00A8:    DD 02                   fld      qword ptr [edx]
00AA:    DC 03                   fadd     qword ptr [ebx]
00AC:    DD 02                   fld      qword ptr [edx]
00AE:    DC 23                   fsub     qword ptr [ebx]
00B0:    DD 1A                   fstp     qword ptr [edx]
00B2:    DD 5D F8                fstp     qword ptr [ebp - 8]
00B5:    90                      nop
00B6:    DD 45 F8                fld      qword ptr [ebp - 8]
00B9:    DC 0F                   fmul     qword ptr [edi]
00BB:    DD 01                   fld      qword ptr [ecx]
00BD:    DC 4F 08                fmul     qword ptr [edi + 8]
00C0:    DE C1                   faddp    st(1)
00C2:    DD 45 F8                fld      qword ptr [ebp - 8]
00C5:    DC 4F 08                fmul     qword ptr [edi + 8]
00C8:    D9 E0                   fchs    ; -ST(0)
00CA:    DD 01                   fld      qword ptr [ecx]
00CC:    DC 0F                   fmul     qword ptr [edi]
00CE:    90                      nop
00CF:    DE C1                   faddp    st(1)
00D1:    DD 19                   fstp     qword ptr [ecx]
00D3:    DD 1B                   fstp     qword ptr [ebx]
00D5:    90                      nop
00D6:    D9 D0                   fnop
00D8:    8B C3                   mov      eax, ebx
00DA:    5F                      pop      edi
00DB:    5E                      pop      esi
00DC:    5B                      pop      ebx
00DD:    89 EC                   mov      esp, ebp
00DF:    5D                      pop      ebp
00E0:    C2 08 00                ret      8
