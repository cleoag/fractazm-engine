; Disassembly: _DonutTransform.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 211 bytes, decoded: 211 bytes (100.0%)
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
0014:    90                      nop
0015:    DD 03                   fld      qword ptr [ebx]
0017:    DD 02                   fld      qword ptr [edx]
0019:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
001B:    DC 4F E0                fmul     qword ptr [edi - 0x20]
001E:    D9 5D F0                fstp     dword ptr [ebp - 0x10]
0021:    90                      nop
0022:    DD 03                   fld      qword ptr [ebx]
0024:    D8 C8                   fmul     st(0)
0026:    DD 02                   fld      qword ptr [edx]
0028:    D8 C8                   fmul     st(0)
002A:    DE C1                   faddp    st(1)
002C:    D9 FA                   fsqrt    ; sqrt(ST(0))
002E:    D9 55 E8                fst      dword ptr [ebp - 0x18]
0031:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0034:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
0037:    90                      nop
0038:    D9 45 F0                fld      dword ptr [ebp - 0x10]
003B:    D9 FB                   fsincos    ; ST(0)=cos, push sin
003D:    90                      nop
003E:    D9 45 E8                fld      dword ptr [ebp - 0x18]
0041:    DC CA                   fmul     st(2), st(0)
0043:    DE C9                   fmulp    st(1)
0045:    DD 47 E8                fld      qword ptr [edi - 0x18]
0048:    DC EA                   fsub     st(2), st(0)
004A:    DE E9                   fsubp    st(1)
004C:    DD 1A                   fstp     qword ptr [edx]
004E:    DD 1B                   fstp     qword ptr [ebx]
0050:    DD 01                   fld      qword ptr [ecx]
0052:    D8 C8                   fmul     st(0)
0054:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0057:    D9 45 E0                fld      dword ptr [ebp - 0x20]
005A:    D8 C8                   fmul     st(0)
005C:    DE C1                   faddp    st(1)
005E:    DD 59 08                fstp     qword ptr [ecx + 8]
0061:    90                      nop
0062:    DD 01                   fld      qword ptr [ecx]
0064:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0067:    DD 19                   fstp     qword ptr [ecx]
0069:    90                      nop
006A:    DD 46 18                fld      qword ptr [esi + 0x18]
006D:    DD 46 20                fld      qword ptr [esi + 0x20]
0070:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0072:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0075:    D9 5D F0                fstp     dword ptr [ebp - 0x10]
0078:    90                      nop
0079:    DD 46 18                fld      qword ptr [esi + 0x18]
007C:    D8 C8                   fmul     st(0)
007E:    DD 46 20                fld      qword ptr [esi + 0x20]
0081:    D8 C8                   fmul     st(0)
0083:    DE C1                   faddp    st(1)
0085:    D9 FA                   fsqrt    ; sqrt(ST(0))
0087:    D9 55 E8                fst      dword ptr [ebp - 0x18]
008A:    DC 67 F0                fsub     qword ptr [edi - 0x10]
008D:    D9 5D E0                fstp     dword ptr [ebp - 0x20]
0090:    90                      nop
0091:    D9 45 F0                fld      dword ptr [ebp - 0x10]
0094:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0096:    90                      nop
0097:    D9 45 E8                fld      dword ptr [ebp - 0x18]
009A:    DC CA                   fmul     st(2), st(0)
009C:    DE C9                   fmulp    st(1)
009E:    DD 47 E8                fld      qword ptr [edi - 0x18]
00A1:    DC EA                   fsub     st(2), st(0)
00A3:    DE E9                   fsubp    st(1)
00A5:    DD 5E 20                fstp     qword ptr [esi + 0x20]
00A8:    DD 5E 18                fstp     qword ptr [esi + 0x18]
00AB:    DD 46 28                fld      qword ptr [esi + 0x28]
00AE:    D8 C8                   fmul     st(0)
00B0:    DC 67 E8                fsub     qword ptr [edi - 0x18]
00B3:    D9 45 E0                fld      dword ptr [ebp - 0x20]
00B6:    D8 C8                   fmul     st(0)
00B8:    DE C1                   faddp    st(1)
00BA:    DD 5E C8                fstp     qword ptr [esi - 0x38]
00BD:    90                      nop
00BE:    DD 46 28                fld      qword ptr [esi + 0x28]
00C1:    DC 67 E8                fsub     qword ptr [edi - 0x18]
00C4:    DD 5E 28                fstp     qword ptr [esi + 0x28]
00C7:    90                      nop
00C8:    8B C3                   mov      eax, ebx
00CA:    5F                      pop      edi
00CB:    5E                      pop      esi
00CC:    5B                      pop      ebx
00CD:    8B E5                   mov      esp, ebp
00CF:    5D                      pop      ebp
00D0:    C2 08 00                ret      8
