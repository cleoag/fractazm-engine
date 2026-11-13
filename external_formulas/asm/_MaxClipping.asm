; Disassembly: _MaxClipping.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 115 bytes, decoded: 115 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    51                      push     ecx
0004:    53                      push     ebx
0005:    56                      push     esi
0006:    89 4D FC                mov      dword ptr [ebp - 4], ecx
0009:    8B C8                   mov      ecx, eax
000B:    8B 45 08                mov      eax, dword ptr [ebp + 8]
000E:    8B 58 30                mov      ebx, dword ptr [eax + 0x30]
0011:    DD 01                   fld      qword ptr [ecx]
0013:    8B C3                   mov      eax, ebx
0015:    83 E8 10                sub      eax, 0x10
0018:    DC 18                   fcomp    qword ptr [eax]
001A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
001C:    9E                      sahf
001D:    76 0F                   jbe      0x2e    ; Conditional jump
001F:    8B C3                   mov      eax, ebx
0021:    83 E8 10                sub      eax, 0x10
0024:    8B 30                   mov      esi, dword ptr [eax]
0026:    89 31                   mov      dword ptr [ecx], esi
0028:    8B 70 04                mov      esi, dword ptr [eax + 4]
002B:    89 71 04                mov      dword ptr [ecx + 4], esi
002E:    DD 02                   fld      qword ptr [edx]
0030:    8B C3                   mov      eax, ebx
0032:    83 E8 18                sub      eax, 0x18
0035:    DC 18                   fcomp    qword ptr [eax]
0037:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0039:    9E                      sahf
003A:    76 0F                   jbe      0x4b    ; Conditional jump
003C:    8B C3                   mov      eax, ebx
003E:    83 E8 18                sub      eax, 0x18
0041:    8B 08                   mov      ecx, dword ptr [eax]
0043:    89 0A                   mov      dword ptr [edx], ecx
0045:    8B 48 04                mov      ecx, dword ptr [eax + 4]
0048:    89 4A 04                mov      dword ptr [edx + 4], ecx
004B:    8B 45 FC                mov      eax, dword ptr [ebp - 4]
004E:    DD 00                   fld      qword ptr [eax]
0050:    8B C3                   mov      eax, ebx
0052:    83 E8 20                sub      eax, 0x20
0055:    DC 18                   fcomp    qword ptr [eax]
0057:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0059:    9E                      sahf
005A:    76 10                   jbe      0x6c    ; Conditional jump
005C:    8B 45 FC                mov      eax, dword ptr [ebp - 4]
005F:    83 EB 20                sub      ebx, 0x20
0062:    8B 13                   mov      edx, dword ptr [ebx]
0064:    89 10                   mov      dword ptr [eax], edx
0066:    8B 53 04                mov      edx, dword ptr [ebx + 4]
0069:    89 50 04                mov      dword ptr [eax + 4], edx
006C:    5E                      pop      esi
006D:    5B                      pop      ebx
006E:    59                      pop      ecx
006F:    5D                      pop      ebp
0070:    C2 08 00                ret      8
