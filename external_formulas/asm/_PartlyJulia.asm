; Disassembly: _PartlyJulia.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 109 bytes, decoded: 109 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    50                      push     eax
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    DD 06                   fld      qword ptr [esi]
000E:    DC 57 F0                fcom     qword ptr [edi - 0x10]
0011:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0013:    80 E4 41                and      ah, 0x41
0016:    74 0A                   je       0x22    ; Conditional jump
0018:    DC 57 E8                fcom     qword ptr [edi - 0x18]
001B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
001D:    80 E4 41                and      ah, 0x41
0020:    74 06                   je       0x28    ; Conditional jump
0022:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0025:    DD 56 18                fst      qword ptr [esi + 0x18]
0028:    DD 46 08                fld      qword ptr [esi + 8]
002B:    DC 57 D8                fcom     qword ptr [edi - 0x28]
002E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0030:    80 E4 41                and      ah, 0x41
0033:    74 0A                   je       0x3f    ; Conditional jump
0035:    DC 57 D0                fcom     qword ptr [edi - 0x30]
0038:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
003A:    80 E4 41                and      ah, 0x41
003D:    74 06                   je       0x45    ; Conditional jump
003F:    DC 47 C8                fadd     qword ptr [edi - 0x38]
0042:    DD 56 20                fst      qword ptr [esi + 0x20]
0045:    DD 46 10                fld      qword ptr [esi + 0x10]
0048:    DC 57 C0                fcom     qword ptr [edi - 0x40]
004B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004D:    80 E4 41                and      ah, 0x41
0050:    74 0A                   je       0x5c    ; Conditional jump
0052:    DC 57 B8                fcom     qword ptr [edi - 0x48]
0055:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0057:    80 E4 41                and      ah, 0x41
005A:    74 06                   je       0x62    ; Conditional jump
005C:    DC 47 B0                fadd     qword ptr [edi - 0x50]
005F:    DD 56 28                fst      qword ptr [esi + 0x28]
0062:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0064:    DD D8                   fstp     st(0)
0066:    58                      pop      eax
0067:    5F                      pop      edi
0068:    5E                      pop      esi
0069:    5D                      pop      ebp
006A:    C2 08 00                ret      8
