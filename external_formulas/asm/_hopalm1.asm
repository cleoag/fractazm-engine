; Disassembly: _hopalm1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 136 bytes, decoded: 136 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B D8                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 03                   fld      qword ptr [ebx]
000F:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0012:    DD 01                   fld      qword ptr [ecx]
0014:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0017:    D9 E0                   fchs    ; -ST(0)
0019:    DE C1                   faddp    st(1)
001B:    DC 46 D0                fadd     qword ptr [esi - 0x30]
001E:    D9 C0                   fld      st(0)
0020:    D9 E1                   fabs    ; |ST(0)|
0022:    D9 FA                   fsqrt    ; sqrt(ST(0))
0024:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0026:    D9 C0                   fld      st(0)
0028:    D9 E1                   fabs    ; |ST(0)|
002A:    D9 E8                   fld1    ; Load 1.0
002C:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
002E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0030:    80 E4 41                and      ah, 0x41
0033:    7F 0F                   jg       0x44    ; Conditional jump
0035:    D9 EE                   fldz    ; Load 0.0
0037:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0039:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
003B:    80 E4 41                and      ah, 0x41
003E:    7F 02                   jg       0x42    ; Conditional jump
0040:    D9 E0                   fchs    ; -ST(0)
0042:    EB 11                   jmp      0x55
0044:    D9 EE                   fldz    ; Load 0.0
0046:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0048:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004A:    80 E4 41                and      ah, 0x41
004D:    7F 02                   jg       0x51    ; Conditional jump
004F:    D9 E0                   fchs    ; -ST(0)
0051:    D9 E8                   fld1    ; Load 1.0
0053:    D8 F1                   fdiv     st(1)
0055:    DC 02                   fadd     qword ptr [edx]
0057:    DD C1                   ffree    st(1)
0059:    D9 D0                   fnop
005B:    DD 46 E8                fld      qword ptr [esi - 0x18]
005E:    DC 23                   fsub     qword ptr [ebx]
0060:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0063:    D9 D0                   fnop
0065:    DD 46 E8                fld      qword ptr [esi - 0x18]
0068:    DC 0A                   fmul     qword ptr [edx]
006A:    DD 03                   fld      qword ptr [ebx]
006C:    DC 4E E0                fmul     qword ptr [esi - 0x20]
006F:    D9 E0                   fchs    ; -ST(0)
0071:    DE C1                   faddp    st(1)
0073:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0076:    DC 01                   fadd     qword ptr [ecx]
0078:    D9 D0                   fnop
007A:    DD 19                   fstp     qword ptr [ecx]
007C:    DD 1A                   fstp     qword ptr [edx]
007E:    DD 1B                   fstp     qword ptr [ebx]
0080:    8B C3                   mov      eax, ebx
0082:    5B                      pop      ebx
0083:    5E                      pop      esi
0084:    5D                      pop      ebp
0085:    C2 08 00                ret      8
