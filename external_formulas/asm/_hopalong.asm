; Disassembly: _hopalong.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 100 bytes, decoded: 100 bytes (100.0%)
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
0026:    D9 EE                   fldz    ; Load 0.0
0028:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
002A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
002C:    80 E4 41                and      ah, 0x41
002F:    7F 02                   jg       0x33    ; Conditional jump
0031:    D9 E0                   fchs    ; -ST(0)
0033:    DC 02                   fadd     qword ptr [edx]
0035:    D9 D0                   fnop
0037:    DD 46 E8                fld      qword ptr [esi - 0x18]
003A:    DC 23                   fsub     qword ptr [ebx]
003C:    DC 4E F0                fmul     qword ptr [esi - 0x10]
003F:    D9 D0                   fnop
0041:    DD 46 E8                fld      qword ptr [esi - 0x18]
0044:    DC 0A                   fmul     qword ptr [edx]
0046:    DD 03                   fld      qword ptr [ebx]
0048:    DC 4E E0                fmul     qword ptr [esi - 0x20]
004B:    D9 E0                   fchs    ; -ST(0)
004D:    DE C1                   faddp    st(1)
004F:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0052:    DC 01                   fadd     qword ptr [ecx]
0054:    D9 D0                   fnop
0056:    DD 19                   fstp     qword ptr [ecx]
0058:    DD 1A                   fstp     qword ptr [edx]
005A:    DD 1B                   fstp     qword ptr [ebx]
005C:    8B C3                   mov      eax, ebx
005E:    5B                      pop      ebx
005F:    5E                      pop      esi
0060:    5D                      pop      ebp
0061:    C2 08 00                ret      8
