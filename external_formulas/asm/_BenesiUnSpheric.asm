; Disassembly: _BenesiUnSpheric.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 160 bytes, decoded: 160 bytes (100.0%)
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
003B:    D9 E8                   fld1    ; Load 1.0
003D:    DE F1                   fdivrp   st(1)
003F:    D9 C0                   fld      st(0)
0041:    DC 0A                   fmul     qword ptr [edx]
0043:    DD 55 F8                fst      qword ptr [ebp - 8]
0046:    D8 C8                   fmul     st(0)
0048:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
004B:    90                      nop
004C:    DC 09                   fmul     qword ptr [ecx]
004E:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0051:    D8 C8                   fmul     st(0)
0053:    DC 45 E0                fadd     qword ptr [ebp - 0x20]
0056:    DD 03                   fld      qword ptr [ebx]
0058:    D8 C8                   fmul     st(0)
005A:    DE F9                   fdivp    st(1)
005C:    D9 E8                   fld1    ; Load 1.0
005E:    D8 D9                   fcomp    st(1)
0060:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0062:    80 E4 41                and      ah, 0x41
0065:    7E 04                   jle      0x6b    ; Conditional jump
0067:    D9 E8                   fld1    ; Load 1.0
0069:    DE F1                   fdivrp   st(1)
006B:    D9 E8                   fld1    ; Load 1.0
006D:    DE C1                   faddp    st(1)
006F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0071:    D9 E8                   fld1    ; Load 1.0
0073:    DE F1                   fdivrp   st(1)
0075:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0078:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
007B:    90                      nop
007C:    DD 03                   fld      qword ptr [ebx]
007E:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0081:    DD 45 F8                fld      qword ptr [ebp - 8]
0084:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0087:    DD 45 F0                fld      qword ptr [ebp - 0x10]
008A:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
008D:    DD 19                   fstp     qword ptr [ecx]
008F:    DD 1A                   fstp     qword ptr [edx]
0091:    DD 1B                   fstp     qword ptr [ebx]
0093:    D9 D0                   fnop
0095:    8B C3                   mov      eax, ebx
0097:    5F                      pop      edi
0098:    5E                      pop      esi
0099:    5B                      pop      ebx
009A:    89 EC                   mov      esp, ebp
009C:    5D                      pop      ebp
009D:    C2 08 00                ret      8
