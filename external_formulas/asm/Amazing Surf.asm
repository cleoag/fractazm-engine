; Disassembly: Amazing Surf.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 264 bytes, decoded: 264 bytes (100.0%)
; DEoption: 11

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
002E:    DC 4E AC                fmul     qword ptr [esi - 0x54]
0031:    DE C1                   faddp    st(1)
0033:    DD 5F 48                fstp     qword ptr [edi + 0x48]
0036:    DD 46 D8                fld      qword ptr [esi - 0x28]
0039:    DD 03                   fld      qword ptr [ebx]
003B:    D9 C0                   fld      st(0)
003D:    D8 E2                   fsub     st(2)
003F:    D9 E1                   fabs    ; |ST(0)|
0041:    D8 C1                   fadd     st(1)
0043:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0045:    D8 C2                   fadd     st(2)
0047:    D9 E1                   fabs    ; |ST(0)|
0049:    DE E1                   fsubrp   st(1)
004B:    DD 02                   fld      qword ptr [edx]
004D:    D9 C0                   fld      st(0)
004F:    D8 E3                   fsub     st(3)
0051:    D9 E1                   fabs    ; |ST(0)|
0053:    D8 C1                   fadd     st(1)
0055:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0057:    D8 C3                   fadd     st(3)
0059:    D9 E1                   fabs    ; |ST(0)|
005B:    DE E1                   fsubrp   st(1)
005D:    DD 12                   fst      qword ptr [edx]
005F:    D8 C8                   fmul     st(0)
0061:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0063:    DD 13                   fst      qword ptr [ebx]
0065:    D8 C8                   fmul     st(0)
0067:    DE C1                   faddp    st(1)
0069:    DD D9                   fstp     st(1)
006B:    83 7E A8 00             cmp      dword ptr [esi - 0x58], 0
006F:    75 06                   jne      0x77    ; Conditional jump
0071:    DD 01                   fld      qword ptr [ecx]
0073:    D8 C8                   fmul     st(0)
0075:    DE C1                   faddp    st(1)
0077:    DC 56 E0                fcom     qword ptr [esi - 0x20]
007A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
007C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
007E:    73 07                   jae      0x87    ; Conditional jump
0080:    DD D8                   fstp     st(0)
0082:    DD 46 E8                fld      qword ptr [esi - 0x18]
0085:    EB 16                   jmp      0x9d
0087:    D9 E8                   fld1    ; Load 1.0
0089:    D8 D1                   fcom     st(1)
008B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
008D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
008F:    72 07                   jb       0x98    ; Conditional jump
0091:    DD D8                   fstp     st(0)
0093:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
0096:    EB 05                   jmp      0x9d
0098:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
009A:    DD 47 48                fld      qword ptr [edi + 0x48]
009D:    DD 41 08                fld      qword ptr [ecx + 8]
00A0:    D8 C9                   fmul     st(1)
00A2:    DD 59 08                fstp     qword ptr [ecx + 8]
00A5:    DD 01                   fld      qword ptr [ecx]
00A7:    D8 C9                   fmul     st(1)
00A9:    DC 47 A8                fadd     qword ptr [edi - 0x58]
00AC:    DD 02                   fld      qword ptr [edx]
00AE:    D8 CA                   fmul     st(2)
00B0:    DC 47 A0                fadd     qword ptr [edi - 0x60]
00B3:    DD 03                   fld      qword ptr [ebx]
00B5:    D8 CB                   fmul     st(3)
00B7:    DC 47 98                fadd     qword ptr [edi - 0x68]
00BA:    D9 C0                   fld      st(0)
00BC:    D8 4E BC                fmul     dword ptr [esi - 0x44]
00BF:    D9 C2                   fld      st(2)
00C1:    D8 4E B8                fmul     dword ptr [esi - 0x48]
00C4:    DE C1                   faddp    st(1)
00C6:    D9 C3                   fld      st(3)
00C8:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
00CB:    DE C1                   faddp    st(1)
00CD:    D9 C1                   fld      st(1)
00CF:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
00D2:    D9 C3                   fld      st(3)
00D4:    D8 4E D0                fmul     dword ptr [esi - 0x30]
00D7:    DE C1                   faddp    st(1)
00D9:    D9 C4                   fld      st(4)
00DB:    D8 4E CC                fmul     dword ptr [esi - 0x34]
00DE:    DE C1                   faddp    st(1)
00E0:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00E2:    D8 4E C8                fmul     dword ptr [esi - 0x38]
00E5:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E7:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
00EA:    DE C3                   faddp    st(3)
00EC:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00EE:    D8 4E C0                fmul     dword ptr [esi - 0x40]
00F1:    DE C2                   faddp    st(2)
00F3:    DD 1B                   fstp     qword ptr [ebx]
00F5:    DD 1A                   fstp     qword ptr [edx]
00F7:    DD 19                   fstp     qword ptr [ecx]
00F9:    DD D8                   fstp     st(0)
00FB:    90                      nop
00FC:    90                      nop
00FD:    90                      nop
00FE:    90                      nop
00FF:    89 D8                   mov      eax, ebx
0101:    5B                      pop      ebx
0102:    5F                      pop      edi
0103:    5E                      pop      esi
0104:    5D                      pop      ebp
0105:    C2 08 00                ret      8
