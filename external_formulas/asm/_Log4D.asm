; Disassembly: _Log4D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 171 bytes, decoded: 171 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B D8                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 41 08                fld      qword ptr [ecx + 8]
0010:    D8 C8                   fmul     st(0)
0012:    DD 02                   fld      qword ptr [edx]
0014:    D8 C8                   fmul     st(0)
0016:    DE C1                   faddp    st(1)
0018:    DD 01                   fld      qword ptr [ecx]
001A:    D8 C8                   fmul     st(0)
001C:    DE C1                   faddp    st(1)
001E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0020:    D9 C0                   fld      st(0)
0022:    D9 E4                   ftst
0024:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0026:    D9 D0                   fnop
0028:    9E                      sahf
0029:    DD C0                   ffree    st(0)
002B:    D9 F7                   fincstp
002D:    74 51                   je       0x80    ; Conditional jump
002F:    D9 C0                   fld      st(0)
0031:    DD 03                   fld      qword ptr [ebx]
0033:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0035:    D9 C1                   fld      st(1)
0037:    DE F9                   fdivp    st(1)
0039:    DD 03                   fld      qword ptr [ebx]
003B:    D8 C8                   fmul     st(0)
003D:    D9 C2                   fld      st(2)
003F:    D8 C8                   fmul     st(0)
0041:    DE C1                   faddp    st(1)
0043:    D9 C0                   fld      st(0)
0045:    DD 06                   fld      qword ptr [esi]
0047:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0049:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004B:    80 E4 41                and      ah, 0x41
004E:    74 0D                   je       0x5d    ; Conditional jump
0050:    D9 C0                   fld      st(0)
0052:    D9 ED                   fldln2    ; Load ln(2)
0054:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0056:    D9 F1                   fyl2x
0058:    DC 4E 08                fmul     qword ptr [esi + 8]
005B:    EB 03                   jmp      0x60
005D:    DD 46 10                fld      qword ptr [esi + 0x10]
0060:    DD 1B                   fstp     qword ptr [ebx]
0062:    DD 02                   fld      qword ptr [edx]
0064:    D8 CA                   fmul     st(2)
0066:    DD 1A                   fstp     qword ptr [edx]
0068:    DD 01                   fld      qword ptr [ecx]
006A:    D8 CA                   fmul     st(2)
006C:    DD 19                   fstp     qword ptr [ecx]
006E:    DD 41 08                fld      qword ptr [ecx + 8]
0071:    D8 CA                   fmul     st(2)
0073:    DD 59 08                fstp     qword ptr [ecx + 8]
0076:    DD D8                   fstp     st(0)
0078:    DD D8                   fstp     st(0)
007A:    DD D8                   fstp     st(0)
007C:    EB 25                   jmp      0xa3
007E:    90                      nop
007F:    90                      nop
0080:    90                      nop
0081:    DD D8                   fstp     st(0)
0083:    DD 03                   fld      qword ptr [ebx]
0085:    DD 06                   fld      qword ptr [esi]
0087:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0089:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
008B:    80 E4 41                and      ah, 0x41
008E:    74 0E                   je       0x9e    ; Conditional jump
0090:    DD 03                   fld      qword ptr [ebx]
0092:    D9 ED                   fldln2    ; Load ln(2)
0094:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0096:    D9 F1                   fyl2x
0098:    DD 1B                   fstp     qword ptr [ebx]
009A:    EB 07                   jmp      0xa3
009C:    90                      nop
009D:    90                      nop
009E:    DD 46 10                fld      qword ptr [esi + 0x10]
00A1:    DD 1B                   fstp     qword ptr [ebx]
00A3:    8B C3                   mov      eax, ebx
00A5:    5B                      pop      ebx
00A6:    5E                      pop      esi
00A7:    5D                      pop      ebp
00A8:    C2 08 00                ret      8
