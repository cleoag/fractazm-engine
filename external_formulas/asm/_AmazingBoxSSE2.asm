; Disassembly: _AmazingBoxSSE2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 147 bytes, decoded: 147 bytes (100.0%)
; DEoption: 11

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    66 0F 10 10             movupd   xmm2, xmmword ptr [eax]
000E:    F2 0F 10 21             movsd    xmm4, qword ptr [ecx]
0012:    66 0F 28 C2             movapd   xmm0, xmm2
0016:    66 0F 5F 46 C0          maxpd    xmm0, xmmword ptr [esi - 0x40]
001B:    F2 0F 5F 66 C0          maxsd    xmm4, qword ptr [esi - 0x40]
0020:    66 0F 5D 46 D0          minpd    xmm0, xmmword ptr [esi - 0x30]
0025:    F2 0F 5D 66 D0          minsd    xmm4, qword ptr [esi - 0x30]
002A:    66 0F 58 C0             addpd    xmm0, xmm0
002E:    F2 0F 58 E4             addsd    xmm4, xmm4
0032:    66 0F 5C C2             subpd    xmm0, xmm2
0036:    F2 0F 5C 21             subsd    xmm4, qword ptr [ecx]
003A:    66 0F 28 C8             movapd   xmm1, xmm0
003E:    F2 0F 10 EC             movsd    xmm5, xmm4
0042:    66 0F 59 C9             mulpd    xmm1, xmm1
0046:    F2 0F 59 ED             mulsd    xmm5, xmm5
004A:    66 0F 70 D1 4E          pshufd   xmm2, xmm1, 0x4e
004F:    F2 0F 58 CD             addsd    xmm1, xmm5
0053:    F2 0F 58 CA             addsd    xmm1, xmm2
0057:    66 0F 2E 4E E0          ucomisd  xmm1, qword ptr [esi - 0x20]
005C:    73 07                   jae      0x65    ; Conditional jump
005E:    F2 0F 10 5E E8          movsd    xmm3, qword ptr [esi - 0x18]
0063:    EB 0F                   jmp      0x74
0065:    66 0F 2E 0E             ucomisd  xmm1, qword ptr [esi]
0069:    F2 0F 10 5E F0          movsd    xmm3, qword ptr [esi - 0x10]
006E:    73 04                   jae      0x74    ; Conditional jump
0070:    F2 0F 5E D9             divsd    xmm3, xmm1
0074:    66 0F 16 61 08          movhpd   xmm4, qword ptr [ecx + 8]
0079:    66 0F C6 DB 00          shufpd   xmm3, xmm3, 0
007E:    66 0F 59 C3             mulpd    xmm0, xmm3
0082:    66 0F 59 E3             mulpd    xmm4, xmm3
0086:    66 0F 11 00             movupd   xmmword ptr [eax], xmm0
008A:    66 0F 11 21             movupd   xmmword ptr [ecx], xmm4
008E:    5E                      pop      esi
008F:    5D                      pop      ebp
0090:    C2 08 00                ret      8
