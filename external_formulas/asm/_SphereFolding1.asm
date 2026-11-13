; Disassembly: _SphereFolding1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 75 bytes, decoded: 75 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 46 38                fld      qword ptr [esi + 0x38]
0011:    DC 57 F0                fcom     qword ptr [edi - 0x10]
0014:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0016:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0018:    72 05                   jb       0x1f    ; Conditional jump
001A:    DC 6F E8                fsubr    qword ptr [edi - 0x18]
001D:    EB 0C                   jmp      0x2b
001F:    DC 57 E0                fcom     qword ptr [edi - 0x20]
0022:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0024:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0026:    73 1A                   jae      0x42    ; Conditional jump
0028:    DC 6F D8                fsubr    qword ptr [edi - 0x28]
002B:    DC 76 38                fdiv     qword ptr [esi + 0x38]
002E:    DD 01                   fld      qword ptr [ecx]
0030:    DD 02                   fld      qword ptr [edx]
0032:    DD 03                   fld      qword ptr [ebx]
0034:    D8 CB                   fmul     st(3)
0036:    DD 1B                   fstp     qword ptr [ebx]
0038:    D8 CA                   fmul     st(2)
003A:    DD 1A                   fstp     qword ptr [edx]
003C:    D8 C9                   fmul     st(1)
003E:    89 D8                   mov      eax, ebx
0040:    DD 19                   fstp     qword ptr [ecx]
0042:    DD D8                   fstp     st(0)
0044:    5B                      pop      ebx
0045:    5F                      pop      edi
0046:    5E                      pop      esi
0047:    5D                      pop      ebp
0048:    C2 08 00                ret      8
