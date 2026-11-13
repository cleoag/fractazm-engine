; Disassembly: KaliLinCombSSE2sm.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 323 bytes, decoded: 323 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    57                      push     edi
0004:    56                      push     esi
0005:    53                      push     ebx
0006:    52                      push     edx
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 56 30                mov      edx, dword ptr [esi + 0x30]
000D:    81 C6 80 00 00 00       add      esi, 0x80
0013:    8B 7A F4                mov      edi, dword ptr [edx - 0xc]
0016:    83 7E 50 00             cmp      dword ptr [esi + 0x50], 0
001A:    75 64                   jne      0x80    ; Conditional jump
001C:    FF 46 50                inc      dword ptr [esi + 0x50]
001F:    66 0F 10 30             movupd   xmm6, xmmword ptr [eax]
0023:    66 0F 12 29             movlpd   xmm5, qword ptr [ecx]
0027:    66 0F 57 E4             xorpd    xmm4, xmm4
002B:    66 0F 14 ED             unpcklpd xmm5, xmm5
002F:    66 0F 11 76 58          movupd   xmmword ptr [esi + 0x58], xmm6
0034:    66 0F 11 6E 70          movupd   xmmword ptr [esi + 0x70], xmm5
0039:    66 0F 13 A6 70 FF FF FF movlpd   qword ptr [esi - 0x90], xmm4
0041:    EB 47                   jmp      0x8a
0043:    90                      nop
0044:    66 0F 28 D1             movapd   xmm2, xmm1
0048:    66 0F 59 C8             mulpd    xmm1, xmm0
004C:    66 0F 59 C0             mulpd    xmm0, xmm0
0050:    66 0F 59 D2             mulpd    xmm2, xmm2
0054:    66 0F 58 C9             addpd    xmm1, xmm1
0058:    66 0F 5C C2             subpd    xmm0, xmm2
005C:    C3                      ret
005D:    90                      nop
005E:    66 0F 28 D0             movapd   xmm2, xmm0
0062:    66 0F 28 D9             movapd   xmm3, xmm1
0066:    66 0F 59 D6             mulpd    xmm2, xmm6
006A:    66 0F 59 CD             mulpd    xmm1, xmm5
006E:    66 0F 59 C5             mulpd    xmm0, xmm5
0072:    66 0F 59 DE             mulpd    xmm3, xmm6
0076:    66 0F 58 CA             addpd    xmm1, xmm2
007A:    66 0F 5C C3             subpd    xmm0, xmm3
007E:    C3                      ret
007F:    90                      nop
0080:    66 0F 10 76 58          movupd   xmm6, xmmword ptr [esi + 0x58]
0085:    66 0F 10 6E 70          movupd   xmm5, xmmword ptr [esi + 0x70]
008A:    66 0F 28 CE             movapd   xmm1, xmm6
008E:    66 0F 28 C5             movapd   xmm0, xmm5
0092:    E8 AD FF FF FF          call     0x44
0097:    89 FB                   mov      ebx, edi
0099:    D1 EF                   shr      edi, 1
009B:    83 FF 03                cmp      edi, 3
009E:    75 05                   jne      0xa5    ; Conditional jump
00A0:    E8 B9 FF FF FF          call     0x5e
00A5:    83 EF 02                sub      edi, 2
00A8:    78 05                   js       0xaf    ; Conditional jump
00AA:    E8 95 FF FF FF          call     0x44
00AF:    83 EF 02                sub      edi, 2
00B2:    78 05                   js       0xb9    ; Conditional jump
00B4:    E8 8B FF FF FF          call     0x44
00B9:    D1 EB                   shr      ebx, 1
00BB:    73 05                   jae      0xc2    ; Conditional jump
00BD:    E8 9C FF FF FF          call     0x5e
00C2:    66 0F 12 56 A8          movlpd   xmm2, qword ptr [esi - 0x58]
00C7:    66 0F 10 5E 98          movupd   xmm3, xmmword ptr [esi - 0x68]
00CC:    66 0F 14 D2             unpcklpd xmm2, xmm2
00D0:    66 0F 58 CB             addpd    xmm1, xmm3
00D4:    66 0F 58 C2             addpd    xmm0, xmm2
00D8:    66 0F 11 4E 58          movupd   xmmword ptr [esi + 0x58], xmm1
00DD:    66 0F 11 46 70          movupd   xmmword ptr [esi + 0x70], xmm0
00E2:    66 0F 70 D0 4E          pshufd   xmm2, xmm0, 0x4e
00E7:    66 0F 70 D9 4E          pshufd   xmm3, xmm1, 0x4e
00EC:    F2 0F 58 D0             addsd    xmm2, xmm0
00F0:    F2 0F 58 D9             addsd    xmm3, xmm1
00F4:    66 0F 59 C0             mulpd    xmm0, xmm0
00F8:    66 0F 59 C9             mulpd    xmm1, xmm1
00FC:    66 0F 14 D3             unpcklpd xmm2, xmm3
0100:    66 0F 58 C1             addpd    xmm0, xmm1
0104:    66 0F 28 EA             movapd   xmm5, xmm2
0108:    66 0F 70 E0 4E          pshufd   xmm4, xmm0, 0x4e
010D:    66 0F 59 ED             mulpd    xmm5, xmm5
0111:    F2 0F 58 C4             addsd    xmm0, xmm4
0115:    66 0F 70 DD 4E          pshufd   xmm3, xmm5, 0x4e
011A:    F2 0F 59 42 F8          mulsd    xmm0, qword ptr [edx - 8]
011F:    F2 0F 58 DD             addsd    xmm3, xmm5
0123:    F2 0F 58 1A             addsd    xmm3, qword ptr [edx]
0127:    F2 0F 5E C3             divsd    xmm0, xmm3
012B:    F2 0F 51 C0             sqrtsd   xmm0, xmm0
012F:    66 0F 14 C0             unpcklpd xmm0, xmm0
0133:    66 0F 59 D0             mulpd    xmm2, xmm0
0137:    66 0F 11 10             movupd   xmmword ptr [eax], xmm2
013B:    5A                      pop      edx
013C:    5B                      pop      ebx
013D:    5E                      pop      esi
013E:    5F                      pop      edi
013F:    5D                      pop      ebp
0140:    C2 08 00                ret      8
