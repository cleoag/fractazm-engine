; Disassembly: Makin4D p2a.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 187 bytes, decoded: 187 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    57                      push     edi
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    8B D8                   mov      ebx, eax
000E:    DD 03                   fld      qword ptr [ebx]
0010:    DC 09                   fmul     qword ptr [ecx]
0012:    DD 02                   fld      qword ptr [edx]
0014:    DC 49 08                fmul     qword ptr [ecx + 8]
0017:    DE E9                   fsubp    st(1)
0019:    D8 C0                   fadd     st(0)
001B:    90                      nop
001C:    DD 03                   fld      qword ptr [ebx]
001E:    DC 49 08                fmul     qword ptr [ecx + 8]
0021:    DD 02                   fld      qword ptr [edx]
0023:    DC 09                   fmul     qword ptr [ecx]
0025:    DE C1                   faddp    st(1)
0027:    D8 C0                   fadd     st(0)
0029:    90                      nop
002A:    DD 03                   fld      qword ptr [ebx]
002C:    D8 C8                   fmul     st(0)
002E:    DD 02                   fld      qword ptr [edx]
0030:    D8 C8                   fmul     st(0)
0032:    DE C1                   faddp    st(1)
0034:    DD 01                   fld      qword ptr [ecx]
0036:    D8 C8                   fmul     st(0)
0038:    DD 41 08                fld      qword ptr [ecx + 8]
003B:    D8 C8                   fmul     st(0)
003D:    DE C1                   faddp    st(1)
003F:    D8 D1                   fcom     st(1)
0041:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0043:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0045:    73 25                   jae      0x6c    ; Conditional jump
0047:    DE F1                   fdivrp   st(1)
0049:    D9 E0                   fchs    ; -ST(0)
004B:    D9 E8                   fld1    ; Load 1.0
004D:    DE C1                   faddp    st(1)
004F:    DD 03                   fld      qword ptr [ebx]
0051:    D8 C8                   fmul     st(0)
0053:    DD 02                   fld      qword ptr [edx]
0055:    D8 C8                   fmul     st(0)
0057:    DE E9                   fsubp    st(1)
0059:    D8 C9                   fmul     st(1)
005B:    90                      nop
005C:    DD 03                   fld      qword ptr [ebx]
005E:    DC 0A                   fmul     qword ptr [edx]
0060:    D8 C0                   fadd     st(0)
0062:    D8 CA                   fmul     st(2)
0064:    DD 1A                   fstp     qword ptr [edx]
0066:    DD 1B                   fstp     qword ptr [ebx]
0068:    DD D8                   fstp     st(0)
006A:    EB 1C                   jmp      0x88
006C:    DE F9                   fdivp    st(1)
006E:    D9 E8                   fld1    ; Load 1.0
0070:    D9 E0                   fchs    ; -ST(0)
0072:    DE C1                   faddp    st(1)
0074:    DD 01                   fld      qword ptr [ecx]
0076:    D8 C8                   fmul     st(0)
0078:    DD 41 08                fld      qword ptr [ecx + 8]
007B:    D8 C8                   fmul     st(0)
007D:    DE E9                   fsubp    st(1)
007F:    D8 C9                   fmul     st(1)
0081:    DD 01                   fld      qword ptr [ecx]
0083:    DC 49 08                fmul     qword ptr [ecx + 8]
0086:    EB D8                   jmp      0x60
0088:    DD 59 08                fstp     qword ptr [ecx + 8]
008B:    DD 19                   fstp     qword ptr [ecx]
008D:    DD 46 18                fld      qword ptr [esi + 0x18]
0090:    DC 03                   fadd     qword ptr [ebx]
0092:    DD 1B                   fstp     qword ptr [ebx]
0094:    DD 46 20                fld      qword ptr [esi + 0x20]
0097:    DC 02                   fadd     qword ptr [edx]
0099:    DD 1A                   fstp     qword ptr [edx]
009B:    DD 46 28                fld      qword ptr [esi + 0x28]
009E:    DC 01                   fadd     qword ptr [ecx]
00A0:    DD 19                   fstp     qword ptr [ecx]
00A2:    DD 46 C8                fld      qword ptr [esi - 0x38]
00A5:    DC 47 F0                fadd     qword ptr [edi - 0x10]
00A8:    DC 41 08                fadd     qword ptr [ecx + 8]
00AB:    DD 59 08                fstp     qword ptr [ecx + 8]
00AE:    D9 D0                   fnop
00B0:    8B C3                   mov      eax, ebx
00B2:    5F                      pop      edi
00B3:    5E                      pop      esi
00B4:    5B                      pop      ebx
00B5:    89 EC                   mov      esp, ebp
00B7:    5D                      pop      ebp
00B8:    C2 08 00                ret      8
