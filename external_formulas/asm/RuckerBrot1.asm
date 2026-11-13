; Disassembly: RuckerBrot1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 190 bytes, decoded: 190 bytes (100.0%)

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
0012:    D8 C8                   fmul     st(0)
0014:    DD 02                   fld      qword ptr [edx]
0016:    D8 C8                   fmul     st(0)
0018:    DD 01                   fld      qword ptr [ecx]
001A:    D8 C8                   fmul     st(0)
001C:    DE C1                   faddp    st(1)
001E:    DE C1                   faddp    st(1)
0020:    DD 03                   fld      qword ptr [ebx]
0022:    D8 C8                   fmul     st(0)
0024:    DD 01                   fld      qword ptr [ecx]
0026:    D8 C8                   fmul     st(0)
0028:    DE C1                   faddp    st(1)
002A:    D9 FA                   fsqrt    ; sqrt(ST(0))
002C:    DC 07                   fadd     qword ptr [edi]
002E:    DE F9                   fdivp    st(1)
0030:    D9 D0                   fnop
0032:    DD 03                   fld      qword ptr [ebx]
0034:    D8 C8                   fmul     st(0)
0036:    DD 02                   fld      qword ptr [edx]
0038:    D8 C8                   fmul     st(0)
003A:    D9 E0                   fchs    ; -ST(0)
003C:    DE C1                   faddp    st(1)
003E:    DD 03                   fld      qword ptr [ebx]
0040:    D8 C8                   fmul     st(0)
0042:    DD 02                   fld      qword ptr [edx]
0044:    D8 C8                   fmul     st(0)
0046:    DE C1                   faddp    st(1)
0048:    DC 07                   fadd     qword ptr [edi]
004A:    DE F9                   fdivp    st(1)
004C:    D8 C9                   fmul     st(1)
004E:    DD 03                   fld      qword ptr [ebx]
0050:    D9 E1                   fabs    ; |ST(0)|
0052:    DE C9                   fmulp    st(1)
0054:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0057:    DC 46 18                fadd     qword ptr [esi + 0x18]
005A:    D9 D0                   fnop
005C:    DD 03                   fld      qword ptr [ebx]
005E:    DC 0A                   fmul     qword ptr [edx]
0060:    DD 03                   fld      qword ptr [ebx]
0062:    D9 E1                   fabs    ; |ST(0)|
0064:    DE C9                   fmulp    st(1)
0066:    DD 03                   fld      qword ptr [ebx]
0068:    D8 C8                   fmul     st(0)
006A:    DD 02                   fld      qword ptr [edx]
006C:    D8 C8                   fmul     st(0)
006E:    DE C1                   faddp    st(1)
0070:    DC 07                   fadd     qword ptr [edi]
0072:    DE F9                   fdivp    st(1)
0074:    D8 CA                   fmul     st(2)
0076:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0079:    DC 46 20                fadd     qword ptr [esi + 0x20]
007C:    D9 D0                   fnop
007E:    DD 01                   fld      qword ptr [ecx]
0080:    90                      nop
0081:    90                      nop
0082:    DD 03                   fld      qword ptr [ebx]
0084:    D8 C8                   fmul     st(0)
0086:    DD 02                   fld      qword ptr [edx]
0088:    D8 C8                   fmul     st(0)
008A:    DD 01                   fld      qword ptr [ecx]
008C:    D8 C8                   fmul     st(0)
008E:    DE C1                   faddp    st(1)
0090:    DE C1                   faddp    st(1)
0092:    D9 FA                   fsqrt    ; sqrt(ST(0))
0094:    DE C9                   fmulp    st(1)
0096:    90                      nop
0097:    90                      nop
0098:    DC 4F E0                fmul     qword ptr [edi - 0x20]
009B:    DD 46 28                fld      qword ptr [esi + 0x28]
009E:    D9 E1                   fabs    ; |ST(0)|
00A0:    90                      nop
00A1:    90                      nop
00A2:    90                      nop
00A3:    90                      nop
00A4:    DC 4F D8                fmul     qword ptr [edi - 0x28]
00A7:    DE C1                   faddp    st(1)
00A9:    D9 D0                   fnop
00AB:    DD 19                   fstp     qword ptr [ecx]
00AD:    DD 1A                   fstp     qword ptr [edx]
00AF:    DD 1B                   fstp     qword ptr [ebx]
00B1:    DD D8                   fstp     st(0)
00B3:    90                      nop
00B4:    8B C3                   mov      eax, ebx
00B6:    5F                      pop      edi
00B7:    5E                      pop      esi
00B8:    58                      pop      eax
00B9:    5B                      pop      ebx
00BA:    5D                      pop      ebp
00BB:    C2 08 00                ret      8
