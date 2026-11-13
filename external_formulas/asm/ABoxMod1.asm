; Disassembly: ABoxMod1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 231 bytes, decoded: 231 bytes (100.0%)
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
006D:    DC 46 B8                fadd     qword ptr [esi - 0x48]
0070:    D9 E1                   fabs    ; |ST(0)|
0072:    D8 E3                   fsub     st(3)
0074:    90                      nop
0075:    D9 E1                   fabs    ; |ST(0)|
0077:    D9 E0                   fchs    ; -ST(0)
0079:    D8 C3                   fadd     st(3)
007B:    DD 46 B8                fld      qword ptr [esi - 0x48]
007E:    D9 E1                   fabs    ; |ST(0)|
0080:    D9 E0                   fchs    ; -ST(0)
0082:    DE C1                   faddp    st(1)
0084:    90                      nop
0085:    DD DB                   fstp     st(3)
0087:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0089:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
008B:    D9 C0                   fld      st(0)
008D:    D8 C9                   fmul     st(1)
008F:    D9 C2                   fld      st(2)
0091:    D8 CB                   fmul     st(3)
0093:    DE C1                   faddp    st(1)
0095:    D9 C3                   fld      st(3)
0097:    D8 CC                   fmul     st(4)
0099:    DE C1                   faddp    st(1)
009B:    DC 56 E0                fcom     qword ptr [esi - 0x20]
009E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A0:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A2:    73 07                   jae      0xab    ; Conditional jump
00A4:    DD D8                   fstp     st(0)
00A6:    DD 46 E8                fld      qword ptr [esi - 0x18]
00A9:    EB 16                   jmp      0xc1
00AB:    D9 E8                   fld1    ; Load 1.0
00AD:    D8 D1                   fcom     st(1)
00AF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B1:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00B3:    72 07                   jb       0xbc    ; Conditional jump
00B5:    DD D8                   fstp     st(0)
00B7:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
00BA:    EB 05                   jmp      0xc1
00BC:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00BE:    DD 47 48                fld      qword ptr [edi + 0x48]
00C1:    DD 41 08                fld      qword ptr [ecx + 8]
00C4:    D8 C9                   fmul     st(1)
00C6:    DD 59 08                fstp     qword ptr [ecx + 8]
00C9:    DC CB                   fmul     st(3), st(0)
00CB:    DC CA                   fmul     st(2), st(0)
00CD:    DE C9                   fmulp    st(1)
00CF:    DC 47 A8                fadd     qword ptr [edi - 0x58]
00D2:    DD 19                   fstp     qword ptr [ecx]
00D4:    DC 47 98                fadd     qword ptr [edi - 0x68]
00D7:    DD 1A                   fstp     qword ptr [edx]
00D9:    DC 47 A0                fadd     qword ptr [edi - 0x60]
00DC:    DD 1B                   fstp     qword ptr [ebx]
00DE:    89 D8                   mov      eax, ebx
00E0:    5B                      pop      ebx
00E1:    5F                      pop      edi
00E2:    5E                      pop      esi
00E3:    5D                      pop      ebp
00E4:    C2 08 00                ret      8
