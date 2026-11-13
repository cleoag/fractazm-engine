; Disassembly: ABoxSphereOffset4d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 228 bytes, decoded: 228 bytes (100.0%)
; DEoption: 5

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
0008:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
000B:    81 C7 80 00 00 00       add      edi, 0x80
0011:    DD 46 D8                fld      qword ptr [esi - 0x28]
0014:    DD 00                   fld      qword ptr [eax]
0016:    D9 C0                   fld      st(0)
0018:    D8 E2                   fsub     st(2)
001A:    D9 E1                   fabs    ; |ST(0)|
001C:    D8 C1                   fadd     st(1)
001E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0020:    D8 C2                   fadd     st(2)
0022:    D9 E1                   fabs    ; |ST(0)|
0024:    DE E1                   fsubrp   st(1)
0026:    DC 66 D0                fsub     qword ptr [esi - 0x30]
0029:    DD 02                   fld      qword ptr [edx]
002B:    D9 C0                   fld      st(0)
002D:    D8 E3                   fsub     st(3)
002F:    D9 E1                   fabs    ; |ST(0)|
0031:    D8 C1                   fadd     st(1)
0033:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0035:    D8 C3                   fadd     st(3)
0037:    D9 E1                   fabs    ; |ST(0)|
0039:    DE E1                   fsubrp   st(1)
003B:    DC 66 C8                fsub     qword ptr [esi - 0x38]
003E:    DD 01                   fld      qword ptr [ecx]
0040:    D9 C0                   fld      st(0)
0042:    D8 E4                   fsub     st(4)
0044:    D9 E1                   fabs    ; |ST(0)|
0046:    D8 C1                   fadd     st(1)
0048:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
004A:    D8 C4                   fadd     st(4)
004C:    D9 E1                   fabs    ; |ST(0)|
004E:    DE E1                   fsubrp   st(1)
0050:    DC 66 C0                fsub     qword ptr [esi - 0x40]
0053:    DD 41 08                fld      qword ptr [ecx + 8]
0056:    D9 C0                   fld      st(0)
0058:    D8 E5                   fsub     st(5)
005A:    D9 E1                   fabs    ; |ST(0)|
005C:    D8 C1                   fadd     st(1)
005E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0060:    D8 C5                   fadd     st(5)
0062:    D9 E1                   fabs    ; |ST(0)|
0064:    DE E1                   fsubrp   st(1)
0066:    DC 66 B8                fsub     qword ptr [esi - 0x48]
0069:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
006B:    DD D8                   fstp     st(0)
006D:    D9 C0                   fld      st(0)
006F:    D8 C9                   fmul     st(1)
0071:    D9 C2                   fld      st(2)
0073:    D8 CB                   fmul     st(3)
0075:    DE C1                   faddp    st(1)
0077:    D9 C3                   fld      st(3)
0079:    D8 CC                   fmul     st(4)
007B:    DE C1                   faddp    st(1)
007D:    D9 C4                   fld      st(4)
007F:    D8 CD                   fmul     st(5)
0081:    DE C1                   faddp    st(1)
0083:    DC 56 E0                fcom     qword ptr [esi - 0x20]
0086:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0088:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
008A:    73 07                   jae      0x93    ; Conditional jump
008C:    DD D8                   fstp     st(0)
008E:    DD 46 E8                fld      qword ptr [esi - 0x18]
0091:    EB 13                   jmp      0xa6
0093:    D9 E8                   fld1    ; Load 1.0
0095:    D8 D9                   fcomp    st(1)
0097:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0099:    DD 46 F0                fld      qword ptr [esi - 0x10]
009C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
009E:    72 02                   jb       0xa2    ; Conditional jump
00A0:    D8 F1                   fdiv     st(1)
00A2:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00A4:    DD D8                   fstp     st(0)
00A6:    DD 47 68                fld      qword ptr [edi + 0x68]
00A9:    D8 C9                   fmul     st(1)
00AB:    DD 5F 68                fstp     qword ptr [edi + 0x68]
00AE:    DC CC                   fmul     st(4), st(0)
00B0:    DC CB                   fmul     st(3), st(0)
00B2:    DC CA                   fmul     st(2), st(0)
00B4:    DE C9                   fmulp    st(1)
00B6:    DC 47 A8                fadd     qword ptr [edi - 0x58]
00B9:    DC 46 C0                fadd     qword ptr [esi - 0x40]
00BC:    DD 19                   fstp     qword ptr [ecx]
00BE:    DC 47 A0                fadd     qword ptr [edi - 0x60]
00C1:    DC 46 C8                fadd     qword ptr [esi - 0x38]
00C4:    DD 1A                   fstp     qword ptr [edx]
00C6:    DC 47 98                fadd     qword ptr [edi - 0x68]
00C9:    DC 46 D0                fadd     qword ptr [esi - 0x30]
00CC:    DD 5A F8                fstp     qword ptr [edx - 8]
00CF:    DC 46 B0                fadd     qword ptr [esi - 0x50]
00D2:    DC 46 B8                fadd     qword ptr [esi - 0x48]
00D5:    DC 87 48 FF FF FF       fadd     qword ptr [edi - 0xb8]
00DB:    DD 59 08                fstp     qword ptr [ecx + 8]
00DE:    5F                      pop      edi
00DF:    5E                      pop      esi
00E0:    5D                      pop      ebp
00E1:    C2 08 00                ret      8
