; Disassembly: GenIntPower.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 772 bytes, decoded: 772 bytes (100.0%)
; DEoption: 0

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    66 0F 28 37             movapd   xmm6, xmmword ptr [edi]
0010:    66 0F 10 00             movupd   xmm0, xmmword ptr [eax]
0014:    F2 0F 10 29             movsd    xmm5, qword ptr [ecx]
0018:    66 0F 59 C0             mulpd    xmm0, xmm0
001C:    F2 0F 59 ED             mulsd    xmm5, xmm5
0020:    66 0F 7C C0             haddpd   xmm0, xmm0
0024:    F2 0F 58 E8             addsd    xmm5, xmm0
0028:    F2 0F 10 57 10          movsd    xmm2, qword ptr [edi + 0x10]
002D:    F2 0F 51 CD             sqrtsd   xmm1, xmm5
0031:    F2 0F 5E D1             divsd    xmm2, xmm1
0035:    8B 57 F4                mov      edx, dword ptr [edi - 0xc]
0038:    83 FA 04                cmp      edx, 4
003B:    7C 21                   jl       0x5e    ; Conditional jump
003D:    F2 0F 10 DD             movsd    xmm3, xmm5
0041:    F2 0F 59 ED             mulsd    xmm5, xmm5
0045:    83 FA 08                cmp      edx, 8
0048:    7C 0B                   jl       0x55    ; Conditional jump
004A:    F2 0F 59 ED             mulsd    xmm5, xmm5
004E:    83 FA 0A                cmp      edx, 0xa
0051:    7C 0B                   jl       0x5e    ; Conditional jump
0053:    EB 05                   jmp      0x5a
0055:    83 FA 06                cmp      edx, 6
0058:    7C 04                   jl       0x5e    ; Conditional jump
005A:    F2 0F 59 EB             mulsd    xmm5, xmm3
005E:    D1 EA                   shr      edx, 1
0060:    73 04                   jae      0x66    ; Conditional jump
0062:    F2 0F 59 E9             mulsd    xmm5, xmm1
0066:    66 0F 14 D2             unpcklpd xmm2, xmm2
006A:    66 0F 10 00             movupd   xmm0, xmmword ptr [eax]
006E:    F2 0F 10 09             movsd    xmm1, qword ptr [ecx]
0072:    66 0F 59 C2             mulpd    xmm0, xmm2
0076:    F2 0F 11 68 08          movsd    qword ptr [eax + 8], xmm5
007B:    F2 0F 59 CA             mulsd    xmm1, xmm2
007F:    8B 57 F0                mov      edx, dword ptr [edi - 0x10]
0082:    8B 5F EC                mov      ebx, dword ptr [edi - 0x14]
0085:    E8 58 00 00 00          call     0xe2
008A:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
008E:    74 0B                   je       0x9b    ; Conditional jump
0090:    8B 57 E4                mov      edx, dword ptr [edi - 0x1c]
0093:    8B 5F E0                mov      ebx, dword ptr [edi - 0x20]
0096:    E8 47 00 00 00          call     0xe2
009B:    83 7F DC 00             cmp      dword ptr [edi - 0x24], 0
009F:    74 0B                   je       0xac    ; Conditional jump
00A1:    8B 57 D8                mov      edx, dword ptr [edi - 0x28]
00A4:    8B 5F D4                mov      ebx, dword ptr [edi - 0x2c]
00A7:    E8 36 00 00 00          call     0xe2
00AC:    8B 75 08                mov      esi, dword ptr [ebp + 8]
00AF:    F2 0F 10 68 08          movsd    xmm5, qword ptr [eax + 8]
00B4:    66 0F 10 5E 18          movupd   xmm3, xmmword ptr [esi + 0x18]
00B9:    66 0F 14 ED             unpcklpd xmm5, xmm5
00BD:    F2 0F 59 CD             mulsd    xmm1, xmm5
00C1:    66 0F 59 C5             mulpd    xmm0, xmm5
00C5:    F2 0F 59 4F CC          mulsd    xmm1, qword ptr [edi - 0x34]
00CA:    66 0F 58 C3             addpd    xmm0, xmm3
00CE:    F2 0F 58 4E 28          addsd    xmm1, qword ptr [esi + 0x28]
00D3:    66 0F 11 00             movupd   xmmword ptr [eax], xmm0
00D7:    F2 0F 11 09             movsd    qword ptr [ecx], xmm1
00DB:    5B                      pop      ebx
00DC:    5F                      pop      edi
00DD:    5E                      pop      esi
00DE:    5D                      pop      ebp
00DF:    C2 08 00                ret      8
00E2:    31 F6                   xor      esi, esi
00E4:    83 FB 06                cmp      ebx, 6
00E7:    7C 06                   jl       0xef    ; Conditional jump
00E9:    83 C6 01                add      esi, 1
00EC:    83 EB 06                sub      ebx, 6
00EF:    83 FB 03                cmp      ebx, 3
00F2:    0F 8C C4 00 00 00       jl       0x1bc    ; Conditional jump
00F8:    83 EB 04                sub      ebx, 4
00FB:    79 32                   jns      0x12f    ; Conditional jump
00FD:    66 0F 28 E8             movapd   xmm5, xmm0
0101:    66 0F 59 C0             mulpd    xmm0, xmm0
0105:    66 0F 7C C0             haddpd   xmm0, xmm0
0109:    F2 0F 51 C0             sqrtsd   xmm0, xmm0
010D:    F2 0F 11 01             movsd    qword ptr [ecx], xmm0
0111:    66 0F 14 C1             unpcklpd xmm0, xmm1
0115:    E8 6B 01 00 00          call     0x285
011A:    66 0F 28 C8             movapd   xmm1, xmm0
011E:    F2 0F 5E 01             divsd    xmm0, qword ptr [ecx]
0122:    66 0F 14 C0             unpcklpd xmm0, xmm0
0126:    66 0F 15 C9             unpckhpd xmm1, xmm1
012A:    66 0F 59 C5             mulpd    xmm0, xmm5
012E:    C3                      ret
012F:    83 EB 01                sub      ebx, 1
0132:    79 42                   jns      0x176    ; Conditional jump
0134:    66 0F 17 00             movhpd   qword ptr [eax], xmm0
0138:    66 0F 14 C1             unpcklpd xmm0, xmm1
013C:    66 0F 28 E8             movapd   xmm5, xmm0
0140:    66 0F 59 C0             mulpd    xmm0, xmm0
0144:    66 0F 7C C0             haddpd   xmm0, xmm0
0148:    F2 0F 51 C0             sqrtsd   xmm0, xmm0
014C:    F2 0F 11 01             movsd    qword ptr [ecx], xmm0
0150:    66 0F 16 00             movhpd   xmm0, qword ptr [eax]
0154:    E8 2C 01 00 00          call     0x285
0159:    66 0F 28 C8             movapd   xmm1, xmm0
015D:    F2 0F 5E 01             divsd    xmm0, qword ptr [ecx]
0161:    66 0F 14 C0             unpcklpd xmm0, xmm0
0165:    66 0F 59 C5             mulpd    xmm0, xmm5
0169:    66 0F 15 C8             unpckhpd xmm1, xmm0
016D:    66 0F 14 C1             unpcklpd xmm0, xmm1
0171:    66 0F 15 C9             unpckhpd xmm1, xmm1
0175:    C3                      ret
0176:    F2 0F 11 00             movsd    qword ptr [eax], xmm0
017A:    66 0F 14 C9             unpcklpd xmm1, xmm1
017E:    66 0F 15 C1             unpckhpd xmm0, xmm1
0182:    66 0F 28 E8             movapd   xmm5, xmm0
0186:    66 0F 59 C0             mulpd    xmm0, xmm0
018A:    66 0F 7C C0             haddpd   xmm0, xmm0
018E:    F2 0F 51 C0             sqrtsd   xmm0, xmm0
0192:    F2 0F 11 01             movsd    qword ptr [ecx], xmm0
0196:    66 0F 16 00             movhpd   xmm0, qword ptr [eax]
019A:    E8 E6 00 00 00          call     0x285
019F:    66 0F 28 C8             movapd   xmm1, xmm0
01A3:    F2 0F 5E 09             divsd    xmm1, qword ptr [ecx]
01A7:    66 0F 14 C9             unpcklpd xmm1, xmm1
01AB:    66 0F 59 CD             mulpd    xmm1, xmm5
01AF:    66 0F 15 C0             unpckhpd xmm0, xmm0
01B3:    66 0F 14 C1             unpcklpd xmm0, xmm1
01B7:    66 0F 15 C9             unpckhpd xmm1, xmm1
01BB:    C3                      ret
01BC:    83 EB 01                sub      ebx, 1
01BF:    79 37                   jns      0x1f8    ; Conditional jump
01C1:    66 0F 28 D8             movapd   xmm3, xmm0
01C5:    66 0F 59 DB             mulpd    xmm3, xmm3
01C9:    66 0F 7C DB             haddpd   xmm3, xmm3
01CD:    F2 0F 51 EB             sqrtsd   xmm5, xmm3
01D1:    F2 0F 10 67 10          movsd    xmm4, qword ptr [edi + 0x10]
01D6:    F2 0F 5E E5             divsd    xmm4, xmm5
01DA:    66 0F 14 E4             unpcklpd xmm4, xmm4
01DE:    66 0F 14 ED             unpcklpd xmm5, xmm5
01E2:    F2 0F 11 09             movsd    qword ptr [ecx], xmm1
01E6:    66 0F 59 C4             mulpd    xmm0, xmm4
01EA:    E8 96 00 00 00          call     0x285
01EF:    F2 0F 10 09             movsd    xmm1, qword ptr [ecx]
01F3:    66 0F 59 C5             mulpd    xmm0, xmm5
01F7:    C3                      ret
01F8:    83 EB 01                sub      ebx, 1
01FB:    79 40                   jns      0x23d    ; Conditional jump
01FD:    66 0F 17 01             movhpd   qword ptr [ecx], xmm0
0201:    66 0F 14 C1             unpcklpd xmm0, xmm1
0205:    66 0F 28 D8             movapd   xmm3, xmm0
0209:    66 0F 59 DB             mulpd    xmm3, xmm3
020D:    66 0F 7C DB             haddpd   xmm3, xmm3
0211:    F2 0F 51 EB             sqrtsd   xmm5, xmm3
0215:    F2 0F 10 67 10          movsd    xmm4, qword ptr [edi + 0x10]
021A:    F2 0F 5E E5             divsd    xmm4, xmm5
021E:    66 0F 14 E4             unpcklpd xmm4, xmm4
0222:    66 0F 14 ED             unpcklpd xmm5, xmm5
0226:    66 0F 59 C4             mulpd    xmm0, xmm4
022A:    E8 56 00 00 00          call     0x285
022F:    66 0F 59 C5             mulpd    xmm0, xmm5
0233:    66 0F 70 C8 4E          pshufd   xmm1, xmm0, 0x4e
0238:    66 0F 16 01             movhpd   xmm0, qword ptr [ecx]
023C:    C3                      ret
023D:    F2 0F 11 01             movsd    qword ptr [ecx], xmm0
0241:    66 0F 15 C0             unpckhpd xmm0, xmm0
0245:    66 0F 14 C1             unpcklpd xmm0, xmm1
0249:    66 0F 28 D8             movapd   xmm3, xmm0
024D:    66 0F 59 DB             mulpd    xmm3, xmm3
0251:    66 0F 7C DB             haddpd   xmm3, xmm3
0255:    F2 0F 51 EB             sqrtsd   xmm5, xmm3
0259:    F2 0F 10 67 10          movsd    xmm4, qword ptr [edi + 0x10]
025E:    F2 0F 5E E5             divsd    xmm4, xmm5
0262:    66 0F 14 E4             unpcklpd xmm4, xmm4
0266:    66 0F 14 ED             unpcklpd xmm5, xmm5
026A:    66 0F 59 C4             mulpd    xmm0, xmm4
026E:    E8 12 00 00 00          call     0x285
0273:    66 0F 59 C5             mulpd    xmm0, xmm5
0277:    66 0F 70 C8 4E          pshufd   xmm1, xmm0, 0x4e
027C:    66 0F 14 C0             unpcklpd xmm0, xmm0
0280:    66 0F 12 01             movlpd   xmm0, qword ptr [ecx]
0284:    C3                      ret
0285:    83 FE 00                cmp      esi, 0
0288:    74 05                   je       0x28f    ; Conditional jump
028A:    66 0F C6 C0 01          shufpd   xmm0, xmm0, 1
028F:    66 0F 28 C8             movapd   xmm1, xmm0
0293:    E8 56 00 00 00          call     0x2ee
0298:    83 FA 02                cmp      edx, 2
029B:    7E 46                   jle      0x2e3    ; Conditional jump
029D:    66 0F 28 D9             movapd   xmm3, xmm1
02A1:    83 FA 04                cmp      edx, 4
02A4:    7C 30                   jl       0x2d6    ; Conditional jump
02A6:    66 0F 28 E0             movapd   xmm4, xmm0
02AA:    66 0F 28 C8             movapd   xmm1, xmm0
02AE:    E8 3B 00 00 00          call     0x2ee
02B3:    83 FA 08                cmp      edx, 8
02B6:    7C 10                   jl       0x2c8    ; Conditional jump
02B8:    66 0F 28 C8             movapd   xmm1, xmm0
02BC:    E8 2D 00 00 00          call     0x2ee
02C1:    83 FA 0A                cmp      edx, 0xa
02C4:    7C 10                   jl       0x2d6    ; Conditional jump
02C6:    EB 05                   jmp      0x2cd
02C8:    83 FA 06                cmp      edx, 6
02CB:    7C 09                   jl       0x2d6    ; Conditional jump
02CD:    66 0F 28 CC             movapd   xmm1, xmm4
02D1:    E8 18 00 00 00          call     0x2ee
02D6:    D1 EA                   shr      edx, 1
02D8:    73 09                   jae      0x2e3    ; Conditional jump
02DA:    66 0F 28 CB             movapd   xmm1, xmm3
02DE:    E8 0B 00 00 00          call     0x2ee
02E3:    83 FE 00                cmp      esi, 0
02E6:    74 05                   je       0x2ed    ; Conditional jump
02E8:    66 0F C6 C0 01          shufpd   xmm0, xmm0, 1
02ED:    C3                      ret
02EE:    66 0F 70 D0 4E          pshufd   xmm2, xmm0, 0x4e
02F3:    66 0F 59 C1             mulpd    xmm0, xmm1
02F7:    66 0F 59 D1             mulpd    xmm2, xmm1
02FB:    66 0F 57 C6             xorpd    xmm0, xmm6
02FF:    66 0F 7C C2             haddpd   xmm0, xmm2
0303:    C3                      ret
