; Disassembly: _setw.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 217 bytes, decoded: 217 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 10 00 00 00       sub      esp, 0x10
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    83 7F F4 00             cmp      dword ptr [edi - 0xc], 0
001A:    0F 84 99 00 00 00       je       0xb9    ; Conditional jump
0020:    DD 47 EC                fld      qword ptr [edi - 0x14]
0023:    DD 03                   fld      qword ptr [ebx]
0025:    D9 E1                   fabs    ; |ST(0)|
0027:    D9 F1                   fyl2x
0029:    D9 E8                   fld1    ; Load 1.0
002B:    D9 C1                   fld      st(1)
002D:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
002F:    D9 F0                   f2xm1
0031:    DE C1                   faddp    st(1)
0033:    D9 FD                   fscale
0035:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0037:    DD D8                   fstp     st(0)
0039:    DC 4F E4                fmul     qword ptr [edi - 0x1c]
003C:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
003F:    DD 47 EC                fld      qword ptr [edi - 0x14]
0042:    DD 02                   fld      qword ptr [edx]
0044:    D9 E1                   fabs    ; |ST(0)|
0046:    D9 F1                   fyl2x
0048:    D9 E8                   fld1    ; Load 1.0
004A:    D9 C1                   fld      st(1)
004C:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
004E:    D9 F0                   f2xm1
0050:    DE C1                   faddp    st(1)
0052:    D9 FD                   fscale
0054:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0056:    DD D8                   fstp     st(0)
0058:    DC 4F DC                fmul     qword ptr [edi - 0x24]
005B:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
005E:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0061:    DD 47 EC                fld      qword ptr [edi - 0x14]
0064:    DD 01                   fld      qword ptr [ecx]
0066:    D9 E1                   fabs    ; |ST(0)|
0068:    D9 F1                   fyl2x
006A:    D9 E8                   fld1    ; Load 1.0
006C:    D9 C1                   fld      st(1)
006E:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
0070:    D9 F0                   f2xm1
0072:    DE C1                   faddp    st(1)
0074:    D9 FD                   fscale
0076:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0078:    DD D8                   fstp     st(0)
007A:    DC 4F D4                fmul     qword ptr [edi - 0x2c]
007D:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0080:    DD 47 E4                fld      qword ptr [edi - 0x1c]
0083:    DC 47 DC                fadd     qword ptr [edi - 0x24]
0086:    DC 47 D4                fadd     qword ptr [edi - 0x2c]
0089:    DE F9                   fdivp    st(1)
008B:    DD 47 EC                fld      qword ptr [edi - 0x14]
008E:    D9 E8                   fld1    ; Load 1.0
0090:    DE F9                   fdivp    st(1)
0092:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0094:    D9 F1                   fyl2x
0096:    D9 E8                   fld1    ; Load 1.0
0098:    D9 C1                   fld      st(1)
009A:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
009C:    D9 F0                   f2xm1
009E:    DE C1                   faddp    st(1)
00A0:    D9 FD                   fscale
00A2:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00A4:    DD D8                   fstp     st(0)
00A6:    DD 51 08                fst      qword ptr [ecx + 8]
00A9:    DD 5E C8                fstp     qword ptr [esi - 0x38]
00AC:    D9 D0                   fnop
00AE:    8B C3                   mov      eax, ebx
00B0:    5F                      pop      edi
00B1:    5E                      pop      esi
00B2:    5B                      pop      ebx
00B3:    8B E5                   mov      esp, ebp
00B5:    5D                      pop      ebp
00B6:    C2 08 00                ret      8
00B9:    DD 03                   fld      qword ptr [ebx]
00BB:    DC 4F E4                fmul     qword ptr [edi - 0x1c]
00BE:    DD 02                   fld      qword ptr [edx]
00C0:    DC 4F DC                fmul     qword ptr [edi - 0x24]
00C3:    DD 01                   fld      qword ptr [ecx]
00C5:    DC 4F D4                fmul     qword ptr [edi - 0x2c]
00C8:    DE C1                   faddp    st(1)
00CA:    DE C1                   faddp    st(1)
00CC:    DD 47 E4                fld      qword ptr [edi - 0x1c]
00CF:    DC 47 DC                fadd     qword ptr [edi - 0x24]
00D2:    DC 47 D4                fadd     qword ptr [edi - 0x2c]
00D5:    DE F9                   fdivp    st(1)
00D7:    EB CD                   jmp      0xa6
