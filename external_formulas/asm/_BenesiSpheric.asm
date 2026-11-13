; Disassembly: _BenesiSpheric.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 145 bytes, decoded: 145 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 30 00 00 00       sub      esp, 0x30
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 02                   fld      qword ptr [edx]
0018:    D8 C8                   fmul     st(0)
001A:    DD 55 F8                fst      qword ptr [ebp - 8]
001D:    DD 01                   fld      qword ptr [ecx]
001F:    D8 C8                   fmul     st(0)
0021:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0024:    DE F9                   fdivp    st(1)
0026:    D9 E8                   fld1    ; Load 1.0
0028:    D8 D9                   fcomp    st(1)
002A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
002C:    80 E4 41                and      ah, 0x41
002F:    7E 04                   jle      0x35    ; Conditional jump
0031:    D9 E8                   fld1    ; Load 1.0
0033:    DE F1                   fdivrp   st(1)
0035:    D9 E8                   fld1    ; Load 1.0
0037:    DE C1                   faddp    st(1)
0039:    D9 FA                   fsqrt    ; sqrt(ST(0))
003B:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
003E:    90                      nop
003F:    DD 45 F8                fld      qword ptr [ebp - 8]
0042:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0045:    DD 03                   fld      qword ptr [ebx]
0047:    D8 C8                   fmul     st(0)
0049:    DE F9                   fdivp    st(1)
004B:    D9 E8                   fld1    ; Load 1.0
004D:    D8 D9                   fcomp    st(1)
004F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0051:    80 E4 41                and      ah, 0x41
0054:    7E 04                   jle      0x5a    ; Conditional jump
0056:    D9 E8                   fld1    ; Load 1.0
0058:    DE F1                   fdivrp   st(1)
005A:    D9 E8                   fld1    ; Load 1.0
005C:    DE C1                   faddp    st(1)
005E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0060:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0063:    D9 C0                   fld      st(0)
0065:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0068:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
006B:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
006E:    90                      nop
006F:    DD 03                   fld      qword ptr [ebx]
0071:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0074:    DD 02                   fld      qword ptr [edx]
0076:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0079:    DD 01                   fld      qword ptr [ecx]
007B:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
007E:    DD 19                   fstp     qword ptr [ecx]
0080:    DD 1A                   fstp     qword ptr [edx]
0082:    DD 1B                   fstp     qword ptr [ebx]
0084:    D9 D0                   fnop
0086:    8B C3                   mov      eax, ebx
0088:    5F                      pop      edi
0089:    5E                      pop      esi
008A:    5B                      pop      ebx
008B:    89 EC                   mov      esp, ebp
008D:    5D                      pop      ebp
008E:    C2 08 00                ret      8
