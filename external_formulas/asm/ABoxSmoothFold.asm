; Disassembly: ABoxSmoothFold.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 299 bytes, decoded: 299 bytes (100.0%)
; DEoption: 11

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    89 C3                   mov      ebx, eax
000E:    83 C6 70                add      esi, 0x70
0011:    EB 4E                   jmp      0x61
0013:    90                      nop
0014:    90                      nop
0015:    90                      nop
0016:    D1 E8                   shr      eax, 1
0018:    74 16                   je       0x30    ; Conditional jump
001A:    73 04                   jae      0x20    ; Conditional jump
001C:    D9 C0                   fld      st(0)
001E:    EB 04                   jmp      0x24
0020:    D9 E8                   fld1    ; Load 1.0
0022:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0024:    D8 C8                   fmul     st(0)
0026:    D1 E8                   shr      eax, 1
0028:    73 FA                   jae      0x24    ; Conditional jump
002A:    DC C9                   fmul     st(1), st(0)
002C:    75 F6                   jne      0x24    ; Conditional jump
002E:    DD D8                   fstp     st(0)
0030:    C3                      ret
0031:    DD 00                   fld      qword ptr [eax]
0033:    D9 E1                   fabs    ; |ST(0)|
0035:    50                      push     eax
0036:    8B 47 CC                mov      eax, dword ptr [edi - 0x34]
0039:    D9 C0                   fld      st(0)
003B:    E8 D6 FF FF FF          call     0x16
0040:    58                      pop      eax
0041:    DC 4F C4                fmul     qword ptr [edi - 0x3c]
0044:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0046:    DD 47 D8                fld      qword ptr [edi - 0x28]
0049:    D8 C0                   fadd     st(0)
004B:    D8 E1                   fsub     st(1)
004D:    D8 CA                   fmul     st(2)
004F:    DE C1                   faddp    st(1)
0051:    D9 E8                   fld1    ; Load 1.0
0053:    DE C2                   faddp    st(2)
0055:    DE F1                   fdivrp   st(1)
0057:    F6 40 07 80             test     byte ptr [eax + 7], 0x80
005B:    79 02                   jns      0x5f    ; Conditional jump
005D:    D9 E0                   fchs    ; -ST(0)
005F:    C3                      ret
0060:    90                      nop
0061:    E8 CB FF FF FF          call     0x31
0066:    89 D0                   mov      eax, edx
0068:    E8 C4 FF FF FF          call     0x31
006D:    89 C8                   mov      eax, ecx
006F:    E8 BD FF FF FF          call     0x31
0074:    83 7E 60 00             cmp      dword ptr [esi + 0x60], 0
0078:    7F 09                   jg       0x83    ; Conditional jump
007A:    FF 46 60                inc      dword ptr [esi + 0x60]
007D:    DD 47 F0                fld      qword ptr [edi - 0x10]
0080:    DD 5E 58                fstp     qword ptr [esi + 0x58]
0083:    DD 46 58                fld      qword ptr [esi + 0x58]
0086:    D9 E8                   fld1    ; Load 1.0
0088:    D9 C1                   fld      st(1)
008A:    D9 E1                   fabs    ; |ST(0)|
008C:    DE E1                   fsubrp   st(1)
008E:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0091:    DE C1                   faddp    st(1)
0093:    DD 5E 58                fstp     qword ptr [esi + 0x58]
0096:    DD 47 E0                fld      qword ptr [edi - 0x20]
0099:    DC 1F                   fcomp    qword ptr [edi]
009B:    9B                      wait
009C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A0:    72 04                   jb       0xa6    ; Conditional jump
00A2:    D9 E8                   fld1    ; Load 1.0
00A4:    EB 5A                   jmp      0x100
00A6:    D9 C2                   fld      st(2)
00A8:    D8 C8                   fmul     st(0)
00AA:    D9 C2                   fld      st(2)
00AC:    D8 C8                   fmul     st(0)
00AE:    DE C1                   faddp    st(1)
00B0:    D9 C1                   fld      st(1)
00B2:    D8 C8                   fmul     st(0)
00B4:    DE C1                   faddp    st(1)
00B6:    D9 E8                   fld1    ; Load 1.0
00B8:    DC 47 E0                fadd     qword ptr [edi - 0x20]
00BB:    DC 4F F8                fmul     qword ptr [edi - 8]
00BE:    D9 E8                   fld1    ; Load 1.0
00C0:    DC 67 E0                fsub     qword ptr [edi - 0x20]
00C3:    DC 4F F8                fmul     qword ptr [edi - 8]
00C6:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00C8:    D8 E1                   fsub     st(1)
00CA:    D9 E4                   ftst
00CC:    9B                      wait
00CD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CF:    50                      push     eax
00D0:    D9 E1                   fabs    ; |ST(0)|
00D2:    D8 F2                   fdiv     st(2)
00D4:    D9 C0                   fld      st(0)
00D6:    8B 47 C0                mov      eax, dword ptr [edi - 0x40]
00D9:    D1 E8                   shr      eax, 1
00DB:    73 04                   jae      0xe1    ; Conditional jump
00DD:    D9 FA                   fsqrt    ; sqrt(ST(0))
00DF:    D1 D0                   rcl      eax, 1
00E1:    E8 30 FF FF FF          call     0x16
00E6:    DC 4F B8                fmul     qword ptr [edi - 0x48]
00E9:    D9 E8                   fld1    ; Load 1.0
00EB:    D8 C1                   fadd     st(1)
00ED:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00EF:    DE C1                   faddp    st(1)
00F1:    DE F1                   fdivrp   st(1)
00F3:    58                      pop      eax
00F4:    DE CA                   fmulp    st(2)
00F6:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00F8:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00FA:    73 02                   jae      0xfe    ; Conditional jump
00FC:    D9 E0                   fchs    ; -ST(0)
00FE:    DE C1                   faddp    st(1)
0100:    DD 46 58                fld      qword ptr [esi + 0x58]
0103:    DE F1                   fdivrp   st(1)
0105:    DD 41 08                fld      qword ptr [ecx + 8]
0108:    D8 C9                   fmul     st(1)
010A:    DD 59 08                fstp     qword ptr [ecx + 8]
010D:    DC CB                   fmul     st(3), st(0)
010F:    DC CA                   fmul     st(2), st(0)
0111:    DE C9                   fmulp    st(1)
0113:    DC 46 B8                fadd     qword ptr [esi - 0x48]
0116:    DD 19                   fstp     qword ptr [ecx]
0118:    DC 46 B0                fadd     qword ptr [esi - 0x50]
011B:    DD 1A                   fstp     qword ptr [edx]
011D:    DC 46 A8                fadd     qword ptr [esi - 0x58]
0120:    DD 1B                   fstp     qword ptr [ebx]
0122:    89 D8                   mov      eax, ebx
0124:    5B                      pop      ebx
0125:    5F                      pop      edi
0126:    5E                      pop      esi
0127:    5D                      pop      ebp
0128:    C2 08 00                ret      8
