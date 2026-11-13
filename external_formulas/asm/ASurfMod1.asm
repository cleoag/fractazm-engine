; Disassembly: ASurfMod1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 268 bytes, decoded: 268 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
000E:    81 C7 80 00 00 00       add      edi, 0x80
0014:    83 7F 50 00             cmp      dword ptr [edi + 0x50], 0
0018:    7F 09                   jg       0x23    ; Conditional jump
001A:    FF 47 50                inc      dword ptr [edi + 0x50]
001D:    DD 46 F0                fld      qword ptr [esi - 0x10]
0020:    DD 5F 48                fstp     qword ptr [edi + 0x48]
0023:    DD 47 48                fld      qword ptr [edi + 0x48]
0026:    D9 E8                   fld1    ; Load 1.0
0028:    D9 C1                   fld      st(1)
002A:    D9 E1                   fabs    ; |ST(0)|
002C:    DE E1                   fsubrp   st(1)
002E:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0031:    DE C1                   faddp    st(1)
0033:    DD 5F 48                fstp     qword ptr [edi + 0x48]
0036:    DD 46 D8                fld      qword ptr [esi - 0x28]
0039:    DD 03                   fld      qword ptr [ebx]
003B:    DC 46 C8                fadd     qword ptr [esi - 0x38]
003E:    D9 E1                   fabs    ; |ST(0)|
0040:    D8 E1                   fsub     st(1)
0042:    90                      nop
0043:    D9 E1                   fabs    ; |ST(0)|
0045:    D9 E0                   fchs    ; -ST(0)
0047:    D8 C1                   fadd     st(1)
0049:    DD 46 C8                fld      qword ptr [esi - 0x38]
004C:    D9 E1                   fabs    ; |ST(0)|
004E:    D9 E0                   fchs    ; -ST(0)
0050:    DE C1                   faddp    st(1)
0052:    DD 02                   fld      qword ptr [edx]
0054:    DC 46 C0                fadd     qword ptr [esi - 0x40]
0057:    D9 E1                   fabs    ; |ST(0)|
0059:    D8 E2                   fsub     st(2)
005B:    90                      nop
005C:    D9 E1                   fabs    ; |ST(0)|
005E:    D9 E0                   fchs    ; -ST(0)
0060:    D8 C2                   fadd     st(2)
0062:    DD 46 C0                fld      qword ptr [esi - 0x40]
0065:    D9 E1                   fabs    ; |ST(0)|
0067:    D9 E0                   fchs    ; -ST(0)
0069:    DE C1                   faddp    st(1)
006B:    DD 01                   fld      qword ptr [ecx]
006D:    DD DB                   fstp     st(3)
006F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0071:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0073:    D9 C0                   fld      st(0)
0075:    D8 C9                   fmul     st(1)
0077:    D9 C2                   fld      st(2)
0079:    D8 CB                   fmul     st(3)
007B:    DE C1                   faddp    st(1)
007D:    D9 C3                   fld      st(3)
007F:    D8 CC                   fmul     st(4)
0081:    DE C1                   faddp    st(1)
0083:    DC 4E B8                fmul     qword ptr [esi - 0x48]
0086:    DC 56 E0                fcom     qword ptr [esi - 0x20]
0089:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
008B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
008D:    73 07                   jae      0x96    ; Conditional jump
008F:    DD D8                   fstp     st(0)
0091:    DD 46 E8                fld      qword ptr [esi - 0x18]
0094:    EB 16                   jmp      0xac
0096:    D9 E8                   fld1    ; Load 1.0
0098:    D8 D1                   fcom     st(1)
009A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
009E:    72 07                   jb       0xa7    ; Conditional jump
00A0:    DD D8                   fstp     st(0)
00A2:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
00A5:    EB 05                   jmp      0xac
00A7:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00A9:    DD 47 48                fld      qword ptr [edi + 0x48]
00AC:    DD 41 08                fld      qword ptr [ecx + 8]
00AF:    D8 C9                   fmul     st(1)
00B1:    DD 59 08                fstp     qword ptr [ecx + 8]
00B4:    DC CB                   fmul     st(3), st(0)
00B6:    DC CA                   fmul     st(2), st(0)
00B8:    DE C9                   fmulp    st(1)
00BA:    D9 C0                   fld      st(0)
00BC:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
00BF:    D9 C2                   fld      st(2)
00C1:    D8 4E 98                fmul     dword ptr [esi - 0x68]
00C4:    DE C1                   faddp    st(1)
00C6:    D9 C3                   fld      st(3)
00C8:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
00CB:    DE C1                   faddp    st(1)
00CD:    D9 C1                   fld      st(1)
00CF:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
00D2:    D9 C3                   fld      st(3)
00D4:    D8 4E B0                fmul     dword ptr [esi - 0x50]
00D7:    DE C1                   faddp    st(1)
00D9:    D9 C4                   fld      st(4)
00DB:    D8 4E AC                fmul     dword ptr [esi - 0x54]
00DE:    DE C1                   faddp    st(1)
00E0:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00E2:    D8 4E A8                fmul     dword ptr [esi - 0x58]
00E5:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E7:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
00EA:    DE C3                   faddp    st(3)
00EC:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00EE:    D8 4E A0                fmul     dword ptr [esi - 0x60]
00F1:    DE C2                   faddp    st(2)
00F3:    90                      nop
00F4:    DC 47 A8                fadd     qword ptr [edi - 0x58]
00F7:    DD 19                   fstp     qword ptr [ecx]
00F9:    DC 47 98                fadd     qword ptr [edi - 0x68]
00FC:    DD 1A                   fstp     qword ptr [edx]
00FE:    DC 47 A0                fadd     qword ptr [edi - 0x60]
0101:    DD 1B                   fstp     qword ptr [ebx]
0103:    89 D8                   mov      eax, ebx
0105:    5B                      pop      ebx
0106:    5F                      pop      edi
0107:    5E                      pop      esi
0108:    5D                      pop      ebp
0109:    C2 08 00                ret      8
