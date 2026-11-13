; Disassembly: MsltoeSym4.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 168 bytes, decoded: 168 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
000E:    DD 00                   fld      qword ptr [eax]
0010:    DD 01                   fld      qword ptr [ecx]
0012:    D9 C0                   fld      st(0)
0014:    D9 E1                   fabs    ; |ST(0)|
0016:    D8 4E F4                fmul     dword ptr [esi - 0xc]
0019:    D9 C2                   fld      st(2)
001B:    D9 E1                   fabs    ; |ST(0)|
001D:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
001F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0021:    80 E4 41                and      ah, 0x41
0024:    74 02                   je       0x28    ; Conditional jump
0026:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0028:    DD 02                   fld      qword ptr [edx]
002A:    D9 C0                   fld      st(0)
002C:    D9 E1                   fabs    ; |ST(0)|
002E:    D8 4E F0                fmul     dword ptr [esi - 0x10]
0031:    D9 C3                   fld      st(3)
0033:    D9 E1                   fabs    ; |ST(0)|
0035:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0037:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0039:    80 E4 41                and      ah, 0x41
003C:    74 02                   je       0x40    ; Conditional jump
003E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0040:    D9 C1                   fld      st(1)
0042:    D9 E1                   fabs    ; |ST(0)|
0044:    D8 4E EC                fmul     dword ptr [esi - 0x14]
0047:    D9 C2                   fld      st(2)
0049:    D9 E1                   fabs    ; |ST(0)|
004B:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
004D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004F:    80 E4 41                and      ah, 0x41
0052:    74 02                   je       0x56    ; Conditional jump
0054:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0056:    D9 C1                   fld      st(1)
0058:    D8 CB                   fmul     st(3)
005A:    D9 E4                   ftst
005C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005E:    DD D8                   fstp     st(0)
0060:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0062:    72 06                   jb       0x6a    ; Conditional jump
0064:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0066:    D9 E0                   fchs    ; -ST(0)
0068:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006A:    D9 C0                   fld      st(0)
006C:    D8 CB                   fmul     st(3)
006E:    D9 E4                   ftst
0070:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0072:    DD D8                   fstp     st(0)
0074:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0076:    72 02                   jb       0x7a    ; Conditional jump
0078:    D9 E0                   fchs    ; -ST(0)
007A:    D9 C2                   fld      st(2)
007C:    D8 C8                   fmul     st(0)
007E:    D9 C1                   fld      st(1)
0080:    D8 C8                   fmul     st(0)
0082:    DE E9                   fsubp    st(1)
0084:    D9 C2                   fld      st(2)
0086:    D8 C8                   fmul     st(0)
0088:    DE E9                   fsubp    st(1)
008A:    DC 47 18                fadd     qword ptr [edi + 0x18]
008D:    DD 1B                   fstp     qword ptr [ebx]
008F:    D8 CA                   fmul     st(2)
0091:    D8 C0                   fadd     st(0)
0093:    DC 47 20                fadd     qword ptr [edi + 0x20]
0096:    DD 1A                   fstp     qword ptr [edx]
0098:    DE C9                   fmulp    st(1)
009A:    D8 C0                   fadd     st(0)
009C:    DC 47 28                fadd     qword ptr [edi + 0x28]
009F:    DD 19                   fstp     qword ptr [ecx]
00A1:    5B                      pop      ebx
00A2:    5F                      pop      edi
00A3:    5E                      pop      esi
00A4:    5D                      pop      ebp
00A5:    C2 08 00                ret      8
