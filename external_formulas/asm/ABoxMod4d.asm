; Disassembly: ABoxMod4d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 143 bytes, decoded: 143 bytes (100.0%)
; DEoption: 5

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 47 E8                fld      qword ptr [edi - 0x18]
000E:    DD 41 08                fld      qword ptr [ecx + 8]
0011:    D8 C1                   fadd     st(1)
0013:    D9 E1                   fabs    ; |ST(0)|
0015:    D8 E1                   fsub     st(1)
0017:    DD 01                   fld      qword ptr [ecx]
0019:    D8 C2                   fadd     st(2)
001B:    D9 E1                   fabs    ; |ST(0)|
001D:    D8 E2                   fsub     st(2)
001F:    DD 02                   fld      qword ptr [edx]
0021:    D8 C3                   fadd     st(3)
0023:    D9 E1                   fabs    ; |ST(0)|
0025:    D8 E3                   fsub     st(3)
0027:    DD 00                   fld      qword ptr [eax]
0029:    D8 C4                   fadd     st(4)
002B:    D9 E1                   fabs    ; |ST(0)|
002D:    D8 E4                   fsub     st(4)
002F:    DD 47 F0                fld      qword ptr [edi - 0x10]
0032:    D9 C4                   fld      st(4)
0034:    D8 C8                   fmul     st(0)
0036:    D9 C4                   fld      st(4)
0038:    D8 C8                   fmul     st(0)
003A:    DE C1                   faddp    st(1)
003C:    D9 C3                   fld      st(3)
003E:    D8 C8                   fmul     st(0)
0040:    DE C1                   faddp    st(1)
0042:    D9 C2                   fld      st(2)
0044:    D8 C8                   fmul     st(0)
0046:    DE C1                   faddp    st(1)
0048:    DC 07                   fadd     qword ptr [edi]
004A:    D9 E8                   fld1    ; Load 1.0
004C:    D8 D9                   fcomp    st(1)
004E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0050:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0052:    72 02                   jb       0x56    ; Conditional jump
0054:    DC F9                   fdiv     st(1), st(0)
0056:    8D 42 F8                lea      eax, [edx - 8]
0059:    DD D8                   fstp     st(0)
005B:    DC CC                   fmul     st(4), st(0)
005D:    DC CB                   fmul     st(3), st(0)
005F:    DC CA                   fmul     st(2), st(0)
0061:    DC C9                   fmul     st(1), st(0)
0063:    DC 8E E8 00 00 00       fmul     qword ptr [esi + 0xe8]
0069:    DD 9E E8 00 00 00       fstp     qword ptr [esi + 0xe8]
006F:    DC 46 18                fadd     qword ptr [esi + 0x18]
0072:    DD 18                   fstp     qword ptr [eax]
0074:    DC 46 20                fadd     qword ptr [esi + 0x20]
0077:    DD 1A                   fstp     qword ptr [edx]
0079:    DC 46 28                fadd     qword ptr [esi + 0x28]
007C:    DD 19                   fstp     qword ptr [ecx]
007E:    DC 46 C8                fadd     qword ptr [esi - 0x38]
0081:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0084:    DD 59 08                fstp     qword ptr [ecx + 8]
0087:    DD D8                   fstp     st(0)
0089:    5F                      pop      edi
008A:    5E                      pop      esi
008B:    5D                      pop      ebp
008C:    C2 08 00                ret      8
