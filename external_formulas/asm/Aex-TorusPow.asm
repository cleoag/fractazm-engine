; Disassembly: Aex-TorusPow.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 213 bytes, decoded: 213 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 60 00 00 00       sub      esp, 0x60
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 02                   fld      qword ptr [edx]
0018:    DD 03                   fld      qword ptr [ebx]
001A:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
001C:    DD 55 E8                fst      qword ptr [ebp - 0x18]
001F:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0022:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0024:    DD 5D F8                fstp     qword ptr [ebp - 8]
0027:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
002A:    DD 45 E8                fld      qword ptr [ebp - 0x18]
002D:    D9 FB                   fsincos    ; ST(0)=cos, push sin
002F:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0032:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0035:    DD 03                   fld      qword ptr [ebx]
0037:    D8 C8                   fmul     st(0)
0039:    DD 02                   fld      qword ptr [edx]
003B:    D8 C8                   fmul     st(0)
003D:    DE C1                   faddp    st(1)
003F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0041:    DD 01                   fld      qword ptr [ecx]
0043:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0045:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0048:    D9 FB                   fsincos    ; ST(0)=cos, push sin
004A:    DC 47 D8                fadd     qword ptr [edi - 0x28]
004D:    83 7F D4 00             cmp      dword ptr [edi - 0x2c], 0
0051:    74 02                   je       0x55    ; Conditional jump
0053:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0055:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0058:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
005B:    DD 03                   fld      qword ptr [ebx]
005D:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0060:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0063:    D9 E0                   fchs    ; -ST(0)
0065:    DE C1                   faddp    st(1)
0067:    D8 C8                   fmul     st(0)
0069:    DD 02                   fld      qword ptr [edx]
006B:    DD 45 E0                fld      qword ptr [ebp - 0x20]
006E:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0071:    D9 E0                   fchs    ; -ST(0)
0073:    DE C1                   faddp    st(1)
0075:    D8 C8                   fmul     st(0)
0077:    DE C1                   faddp    st(1)
0079:    DD 01                   fld      qword ptr [ecx]
007B:    D8 C8                   fmul     st(0)
007D:    DE C1                   faddp    st(1)
007F:    DD 12                   fst      qword ptr [edx]
0081:    DD 47 F0                fld      qword ptr [edi - 0x10]
0084:    DC 0F                   fmul     qword ptr [edi]
0086:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0088:    D9 E1                   fabs    ; |ST(0)|
008A:    D9 F1                   fyl2x
008C:    D9 E8                   fld1    ; Load 1.0
008E:    D9 C1                   fld      st(1)
0090:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
0092:    D9 F0                   f2xm1
0094:    DE C1                   faddp    st(1)
0096:    D9 FD                   fscale
0098:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
009A:    DD D8                   fstp     st(0)
009C:    DD 13                   fst      qword ptr [ebx]
009E:    DD 11                   fst      qword ptr [ecx]
00A0:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00A3:    DC 4D F8                fmul     qword ptr [ebp - 8]
00A6:    DC 46 18                fadd     qword ptr [esi + 0x18]
00A9:    90                      nop
00AA:    DD 01                   fld      qword ptr [ecx]
00AC:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00AF:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00B2:    DC 46 20                fadd     qword ptr [esi + 0x20]
00B5:    90                      nop
00B6:    DD 03                   fld      qword ptr [ebx]
00B8:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00BB:    DC 46 28                fadd     qword ptr [esi + 0x28]
00BE:    DC 4F E0                fmul     qword ptr [edi - 0x20]
00C1:    90                      nop
00C2:    DD 19                   fstp     qword ptr [ecx]
00C4:    DD 1A                   fstp     qword ptr [edx]
00C6:    DD 1B                   fstp     qword ptr [ebx]
00C8:    D9 D0                   fnop
00CA:    8B C3                   mov      eax, ebx
00CC:    5F                      pop      edi
00CD:    5E                      pop      esi
00CE:    5B                      pop      ebx
00CF:    89 EC                   mov      esp, ebp
00D1:    5D                      pop      ebp
00D2:    C2 08 00                ret      8
