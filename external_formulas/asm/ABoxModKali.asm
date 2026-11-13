; Disassembly: ABoxModKali.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 155 bytes, decoded: 155 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 5E 30                mov      ebx, dword ptr [esi + 0x30]
000B:    66 0F 10 10             movupd   xmm2, xmmword ptr [eax]
000F:    66 0F 10 19             movupd   xmm3, xmmword ptr [ecx]
0013:    66 0F 54 13             andpd    xmm2, xmmword ptr [ebx]
0017:    66 0F 54 1B             andpd    xmm3, xmmword ptr [ebx]
001B:    66 0F 28 43 C0          movapd   xmm0, xmmword ptr [ebx - 0x40]
0020:    F2 0F 10 4B D0          movsd    xmm1, qword ptr [ebx - 0x30]
0025:    F2 0F 10 7B D8          movsd    xmm7, qword ptr [ebx - 0x28]
002A:    66 0F 5C C2             subpd    xmm0, xmm2
002E:    F2 0F 5C CB             subsd    xmm1, xmm3
0032:    66 0F 28 E0             movapd   xmm4, xmm0
0036:    66 0F 28 E9             movapd   xmm5, xmm1
003A:    66 0F 59 E4             mulpd    xmm4, xmm4
003E:    F2 0F 59 ED             mulsd    xmm5, xmm5
0042:    66 0F 70 F4 4E          pshufd   xmm6, xmm4, 0x4e
0047:    F2 0F 58 E5             addsd    xmm4, xmm5
004B:    F2 0F 58 E6             addsd    xmm4, xmm6
004F:    66 0F 2E 63 E0          ucomisd  xmm4, qword ptr [ebx - 0x20]
0054:    73 07                   jae      0x5d    ; Conditional jump
0056:    F2 0F 10 7B E8          movsd    xmm7, qword ptr [ebx - 0x18]
005B:    EB 10                   jmp      0x6d
005D:    66 0F 2E 63 10          ucomisd  xmm4, qword ptr [ebx + 0x10]
0062:    F2 0F 10 7B F0          movsd    xmm7, qword ptr [ebx - 0x10]
0067:    73 04                   jae      0x6d    ; Conditional jump
0069:    F2 0F 5E FC             divsd    xmm7, xmm4
006D:    66 0F C6 FF 00          shufpd   xmm7, xmm7, 0
0072:    66 0F 10 6E 18          movupd   xmm5, xmmword ptr [esi + 0x18]
0077:    66 0F C6 CB 02          shufpd   xmm1, xmm3, 2
007C:    66 0F 59 C7             mulpd    xmm0, xmm7
0080:    66 0F 59 CF             mulpd    xmm1, xmm7
0084:    66 0F 58 C5             addpd    xmm0, xmm5
0088:    F2 0F 58 4E 28          addsd    xmm1, qword ptr [esi + 0x28]
008D:    66 0F 11 00             movupd   xmmword ptr [eax], xmm0
0091:    66 0F 11 09             movupd   xmmword ptr [ecx], xmm1
0095:    5B                      pop      ebx
0096:    5E                      pop      esi
0097:    5D                      pop      ebp
0098:    C2 08 00                ret      8
