; Disassembly: ABoxMapSFold.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 228 bytes, decoded: 228 bytes (100.0%)
; DEoption: 11

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    83 C4 E8                add      esp, -0x18
0009:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
000C:    89 C3                   mov      ebx, eax
000E:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
0011:    DD 46 D8                fld      qword ptr [esi - 0x28]
0014:    DD 03                   fld      qword ptr [ebx]
0016:    D9 C0                   fld      st(0)
0018:    D8 E2                   fsub     st(2)
001A:    D9 E1                   fabs    ; |ST(0)|
001C:    D8 C1                   fadd     st(1)
001E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0020:    D8 C2                   fadd     st(2)
0022:    D9 E1                   fabs    ; |ST(0)|
0024:    DE E1                   fsubrp   st(1)
0026:    DD 14 24                fst      qword ptr [esp]
0029:    DD 02                   fld      qword ptr [edx]
002B:    D9 C0                   fld      st(0)
002D:    D8 E3                   fsub     st(3)
002F:    D9 E1                   fabs    ; |ST(0)|
0031:    D8 C1                   fadd     st(1)
0033:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0035:    D8 C3                   fadd     st(3)
0037:    D9 E1                   fabs    ; |ST(0)|
0039:    DE E1                   fsubrp   st(1)
003B:    DD 54 24 08             fst      qword ptr [esp + 8]
003F:    DD 01                   fld      qword ptr [ecx]
0041:    D9 C0                   fld      st(0)
0043:    D8 E4                   fsub     st(4)
0045:    D9 E1                   fabs    ; |ST(0)|
0047:    D8 C1                   fadd     st(1)
0049:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
004B:    D8 C4                   fadd     st(4)
004D:    D9 E1                   fabs    ; |ST(0)|
004F:    DE E1                   fsubrp   st(1)
0051:    DD 54 24 10             fst      qword ptr [esp + 0x10]
0055:    89 E0                   mov      eax, esp
0057:    89 E1                   mov      ecx, esp
0059:    8B 56 D4                mov      edx, dword ptr [esi - 0x2c]
005C:    FF 97 60 01 00 00       call     dword ptr [edi + 0x160]
0062:    D9 46 D0                fld      dword ptr [esi - 0x30]
0065:    D9 46 CC                fld      dword ptr [esi - 0x34]
0068:    DD 04 24                fld      qword ptr [esp]
006B:    D8 C2                   fadd     st(2)
006D:    D8 C9                   fmul     st(1)
006F:    D8 C5                   fadd     st(5)
0071:    D8 C8                   fmul     st(0)
0073:    DD 44 24 08             fld      qword ptr [esp + 8]
0077:    D8 C3                   fadd     st(3)
0079:    D8 CA                   fmul     st(2)
007B:    D8 C5                   fadd     st(5)
007D:    D8 C8                   fmul     st(0)
007F:    DE C1                   faddp    st(1)
0081:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0085:    D8 C3                   fadd     st(3)
0087:    D8 CA                   fmul     st(2)
0089:    D8 C4                   fadd     st(4)
008B:    D8 C8                   fmul     st(0)
008D:    DE C1                   faddp    st(1)
008F:    DC 56 E0                fcom     qword ptr [esi - 0x20]
0092:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0094:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0096:    73 07                   jae      0x9f    ; Conditional jump
0098:    DD D8                   fstp     st(0)
009A:    DD 46 E8                fld      qword ptr [esi - 0x18]
009D:    EB 16                   jmp      0xb5
009F:    D9 E8                   fld1    ; Load 1.0
00A1:    D8 D1                   fcom     st(1)
00A3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A5:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A7:    72 07                   jb       0xb0    ; Conditional jump
00A9:    DD D8                   fstp     st(0)
00AB:    DC 7E F0                fdivr    qword ptr [esi - 0x10]
00AE:    EB 05                   jmp      0xb5
00B0:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00B2:    DD 46 F0                fld      qword ptr [esi - 0x10]
00B5:    DD 43 18                fld      qword ptr [ebx + 0x18]
00B8:    D8 C9                   fmul     st(1)
00BA:    DD 5B 18                fstp     qword ptr [ebx + 0x18]
00BD:    DC CD                   fmul     st(5), st(0)
00BF:    DC CC                   fmul     st(4), st(0)
00C1:    DE CB                   fmulp    st(3)
00C3:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00C5:    DC 47 28                fadd     qword ptr [edi + 0x28]
00C8:    DD 5B 10                fstp     qword ptr [ebx + 0x10]
00CB:    DC 47 20                fadd     qword ptr [edi + 0x20]
00CE:    DD 5B 08                fstp     qword ptr [ebx + 8]
00D1:    DC 47 18                fadd     qword ptr [edi + 0x18]
00D4:    DD 1B                   fstp     qword ptr [ebx]
00D6:    DD D8                   fstp     st(0)
00D8:    89 D8                   mov      eax, ebx
00DA:    83 C4 18                add      esp, 0x18
00DD:    5B                      pop      ebx
00DE:    5F                      pop      edi
00DF:    5E                      pop      esi
00E0:    5D                      pop      ebp
00E1:    C2 08 00                ret      8
