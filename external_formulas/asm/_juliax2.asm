; Disassembly: _juliax2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 81 bytes, decoded: 81 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 03                   fld      qword ptr [ebx]
0010:    D9 E4                   ftst
0012:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0014:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0016:    73 20                   jae      0x38    ; Conditional jump
0018:    DC 47 F0                fadd     qword ptr [edi - 0x10]
001B:    DD 1B                   fstp     qword ptr [ebx]
001D:    DD 47 E8                fld      qword ptr [edi - 0x18]
0020:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0023:    DD 47 E0                fld      qword ptr [edi - 0x20]
0026:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0029:    DD 47 D8                fld      qword ptr [edi - 0x28]
002C:    DD 5E 28                fstp     qword ptr [esi + 0x28]
002F:    89 D8                   mov      eax, ebx
0031:    5B                      pop      ebx
0032:    5F                      pop      edi
0033:    5E                      pop      esi
0034:    5D                      pop      ebp
0035:    C2 08 00                ret      8
0038:    DC 67 F0                fsub     qword ptr [edi - 0x10]
003B:    DD 1B                   fstp     qword ptr [ebx]
003D:    DD 47 D0                fld      qword ptr [edi - 0x30]
0040:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0043:    DD 47 C8                fld      qword ptr [edi - 0x38]
0046:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0049:    DD 47 C0                fld      qword ptr [edi - 0x40]
004C:    DD 5E 28                fstp     qword ptr [esi + 0x28]
004F:    EB DE                   jmp      0x2f
