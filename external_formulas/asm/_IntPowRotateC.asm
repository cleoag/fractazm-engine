; Disassembly: _IntPowRotateC.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 724 bytes, decoded: 724 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    51                      push     ecx
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000D:    83 C6 18                add      esi, 0x18
0010:    66 0F 28 37             movapd   xmm6, xmmword ptr [edi]
0014:    66 0F 10 06             movupd   xmm0, xmmword ptr [esi]
0018:    F2 0F 10 4E 10          movsd    xmm1, qword ptr [esi + 0x10]
001D:    66 0F 28 D8             movapd   xmm3, xmm0
0021:    F2 0F 10 E9             movsd    xmm5, xmm1
0025:    66 0F 59 DB             mulpd    xmm3, xmm3
0029:    F2 0F 59 ED             mulsd    xmm5, xmm5
002D:    66 0F 7C DB             haddpd   xmm3, xmm3
0031:    F2 0F 58 EB             addsd    xmm5, xmm3
0035:    F2 0F 10 57 10          movsd    xmm2, qword ptr [edi + 0x10]
003A:    F2 0F 51 ED             sqrtsd   xmm5, xmm5
003E:    F2 0F 5E D5             divsd    xmm2, xmm5
0042:    66 0F 14 D2             unpcklpd xmm2, xmm2
0046:    66 0F 59 C2             mulpd    xmm0, xmm2
004A:    F2 0F 59 CA             mulsd    xmm1, xmm2
004E:    83 7F F4 00             cmp      dword ptr [edi - 0xc], 0
0052:    74 05                   je       0x59    ; Conditional jump
0054:    66 0F C6 C0 01          shufpd   xmm0, xmm0, 1
0059:    F2 0F 11 6E 08          movsd    qword ptr [esi + 8], xmm5
005E:    8B 57 F0                mov      edx, dword ptr [edi - 0x10]
0061:    8B 5F EC                mov      ebx, dword ptr [edi - 0x14]
0064:    E8 49 00 00 00          call     0xb2
0069:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
006D:    74 0B                   je       0x7a    ; Conditional jump
006F:    8B 57 E4                mov      edx, dword ptr [edi - 0x1c]
0072:    8B 5F E0                mov      ebx, dword ptr [edi - 0x20]
0075:    E8 38 00 00 00          call     0xb2
007A:    83 7F DC 00             cmp      dword ptr [edi - 0x24], 0
007E:    74 0B                   je       0x8b    ; Conditional jump
0080:    8B 57 D8                mov      edx, dword ptr [edi - 0x28]
0083:    8B 5F D4                mov      ebx, dword ptr [edi - 0x2c]
0086:    E8 27 00 00 00          call     0xb2
008B:    F2 0F 10 6E 08          movsd    xmm5, qword ptr [esi + 8]
0090:    66 0F 14 ED             unpcklpd xmm5, xmm5
0094:    F2 0F 59 CD             mulsd    xmm1, xmm5
0098:    66 0F 59 C5             mulpd    xmm0, xmm5
009C:    F2 0F 59 4F CC          mulsd    xmm1, qword ptr [edi - 0x34]
00A1:    66 0F 11 06             movupd   xmmword ptr [esi], xmm0
00A5:    F2 0F 11 4E 10          movsd    qword ptr [esi + 0x10], xmm1
00AA:    59                      pop      ecx
00AB:    5B                      pop      ebx
00AC:    5F                      pop      edi
00AD:    5E                      pop      esi
00AE:    5D                      pop      ebp
00AF:    C2 08 00                ret      8
00B2:    31 C9                   xor      ecx, ecx
00B4:    83 FB 06                cmp      ebx, 6
00B7:    7C 06                   jl       0xbf    ; Conditional jump
00B9:    83 C1 01                add      ecx, 1
00BC:    83 EB 06                sub      ebx, 6
00BF:    83 EB 01                sub      ebx, 1
00C2:    79 37                   jns      0xfb    ; Conditional jump
00C4:    66 0F 28 D8             movapd   xmm3, xmm0
00C8:    66 0F 59 DB             mulpd    xmm3, xmm3
00CC:    66 0F 7C DB             haddpd   xmm3, xmm3
00D0:    F2 0F 51 EB             sqrtsd   xmm5, xmm3
00D4:    F2 0F 10 67 10          movsd    xmm4, qword ptr [edi + 0x10]
00D9:    F2 0F 5E E5             divsd    xmm4, xmm5
00DD:    66 0F 14 E4             unpcklpd xmm4, xmm4
00E1:    66 0F 14 ED             unpcklpd xmm5, xmm5
00E5:    F2 0F 11 0E             movsd    qword ptr [esi], xmm1
00E9:    66 0F 59 C4             mulpd    xmm0, xmm4
00ED:    E8 63 01 00 00          call     0x255
00F2:    F2 0F 10 0E             movsd    xmm1, qword ptr [esi]
00F6:    66 0F 59 C5             mulpd    xmm0, xmm5
00FA:    C3                      ret
00FB:    83 EB 01                sub      ebx, 1
00FE:    79 40                   jns      0x140    ; Conditional jump
0100:    66 0F 17 06             movhpd   qword ptr [esi], xmm0
0104:    66 0F 14 C1             unpcklpd xmm0, xmm1
0108:    66 0F 28 D8             movapd   xmm3, xmm0
010C:    66 0F 59 DB             mulpd    xmm3, xmm3
0110:    66 0F 7C DB             haddpd   xmm3, xmm3
0114:    F2 0F 51 EB             sqrtsd   xmm5, xmm3
0118:    F2 0F 10 67 10          movsd    xmm4, qword ptr [edi + 0x10]
011D:    F2 0F 5E E5             divsd    xmm4, xmm5
0121:    66 0F 14 E4             unpcklpd xmm4, xmm4
0125:    66 0F 14 ED             unpcklpd xmm5, xmm5
0129:    66 0F 59 C4             mulpd    xmm0, xmm4
012D:    E8 23 01 00 00          call     0x255
0132:    66 0F 59 C5             mulpd    xmm0, xmm5
0136:    66 0F 70 C8 4E          pshufd   xmm1, xmm0, 0x4e
013B:    66 0F 16 06             movhpd   xmm0, qword ptr [esi]
013F:    C3                      ret
0140:    83 EB 01                sub      ebx, 1
0143:    79 48                   jns      0x18d    ; Conditional jump
0145:    F2 0F 11 06             movsd    qword ptr [esi], xmm0
0149:    66 0F 15 C0             unpckhpd xmm0, xmm0
014D:    66 0F 14 C1             unpcklpd xmm0, xmm1
0151:    66 0F 28 D8             movapd   xmm3, xmm0
0155:    66 0F 59 DB             mulpd    xmm3, xmm3
0159:    66 0F 7C DB             haddpd   xmm3, xmm3
015D:    F2 0F 51 EB             sqrtsd   xmm5, xmm3
0161:    F2 0F 10 67 10          movsd    xmm4, qword ptr [edi + 0x10]
0166:    F2 0F 5E E5             divsd    xmm4, xmm5
016A:    66 0F 14 E4             unpcklpd xmm4, xmm4
016E:    66 0F 14 ED             unpcklpd xmm5, xmm5
0172:    66 0F 59 C4             mulpd    xmm0, xmm4
0176:    E8 DA 00 00 00          call     0x255
017B:    66 0F 59 C5             mulpd    xmm0, xmm5
017F:    66 0F 70 C8 4E          pshufd   xmm1, xmm0, 0x4e
0184:    66 0F 14 C0             unpcklpd xmm0, xmm0
0188:    66 0F 12 06             movlpd   xmm0, qword ptr [esi]
018C:    C3                      ret
018D:    83 EB 01                sub      ebx, 1
0190:    79 32                   jns      0x1c4    ; Conditional jump
0192:    66 0F 28 E8             movapd   xmm5, xmm0
0196:    66 0F 59 C0             mulpd    xmm0, xmm0
019A:    66 0F 7C C0             haddpd   xmm0, xmm0
019E:    F2 0F 51 C0             sqrtsd   xmm0, xmm0
01A2:    F2 0F 11 06             movsd    qword ptr [esi], xmm0
01A6:    66 0F 14 C1             unpcklpd xmm0, xmm1
01AA:    E8 A6 00 00 00          call     0x255
01AF:    66 0F 28 C8             movapd   xmm1, xmm0
01B3:    F2 0F 5E 06             divsd    xmm0, qword ptr [esi]
01B7:    66 0F 14 C0             unpcklpd xmm0, xmm0
01BB:    66 0F 15 C9             unpckhpd xmm1, xmm1
01BF:    66 0F 59 C5             mulpd    xmm0, xmm5
01C3:    C3                      ret
01C4:    83 EB 01                sub      ebx, 1
01C7:    79 44                   jns      0x20d    ; Conditional jump
01C9:    66 0F 17 46 10          movhpd   qword ptr [esi + 0x10], xmm0
01CE:    66 0F 14 C1             unpcklpd xmm0, xmm1
01D2:    66 0F 28 E8             movapd   xmm5, xmm0
01D6:    66 0F 59 C0             mulpd    xmm0, xmm0
01DA:    66 0F 7C C0             haddpd   xmm0, xmm0
01DE:    F2 0F 51 C0             sqrtsd   xmm0, xmm0
01E2:    F2 0F 11 06             movsd    qword ptr [esi], xmm0
01E6:    66 0F 16 46 10          movhpd   xmm0, qword ptr [esi + 0x10]
01EB:    E8 65 00 00 00          call     0x255
01F0:    66 0F 28 C8             movapd   xmm1, xmm0
01F4:    F2 0F 5E 06             divsd    xmm0, qword ptr [esi]
01F8:    66 0F 14 C0             unpcklpd xmm0, xmm0
01FC:    66 0F 59 C5             mulpd    xmm0, xmm5
0200:    66 0F 15 C8             unpckhpd xmm1, xmm0
0204:    66 0F 14 C1             unpcklpd xmm0, xmm1
0208:    66 0F 15 C9             unpckhpd xmm1, xmm1
020C:    C3                      ret
020D:    F2 0F 11 46 10          movsd    qword ptr [esi + 0x10], xmm0
0212:    66 0F 14 C9             unpcklpd xmm1, xmm1
0216:    66 0F 15 C1             unpckhpd xmm0, xmm1
021A:    66 0F 28 E8             movapd   xmm5, xmm0
021E:    66 0F 59 C0             mulpd    xmm0, xmm0
0222:    66 0F 7C C0             haddpd   xmm0, xmm0
0226:    F2 0F 51 C0             sqrtsd   xmm0, xmm0
022A:    F2 0F 11 06             movsd    qword ptr [esi], xmm0
022E:    66 0F 16 46 10          movhpd   xmm0, qword ptr [esi + 0x10]
0233:    E8 1D 00 00 00          call     0x255
0238:    66 0F 28 C8             movapd   xmm1, xmm0
023C:    F2 0F 5E 0E             divsd    xmm1, qword ptr [esi]
0240:    66 0F 14 C9             unpcklpd xmm1, xmm1
0244:    66 0F 59 CD             mulpd    xmm1, xmm5
0248:    66 0F 15 C0             unpckhpd xmm0, xmm0
024C:    66 0F 14 C1             unpcklpd xmm0, xmm1
0250:    66 0F 15 C9             unpckhpd xmm1, xmm1
0254:    C3                      ret
0255:    83 F9 00                cmp      ecx, 0
0258:    74 05                   je       0x25f    ; Conditional jump
025A:    66 0F C6 C0 01          shufpd   xmm0, xmm0, 1
025F:    66 0F 28 C8             movapd   xmm1, xmm0
0263:    E8 56 00 00 00          call     0x2be
0268:    83 FA 02                cmp      edx, 2
026B:    7E 46                   jle      0x2b3    ; Conditional jump
026D:    66 0F 28 D9             movapd   xmm3, xmm1
0271:    83 FA 04                cmp      edx, 4
0274:    7C 30                   jl       0x2a6    ; Conditional jump
0276:    66 0F 28 E0             movapd   xmm4, xmm0
027A:    66 0F 28 C8             movapd   xmm1, xmm0
027E:    E8 3B 00 00 00          call     0x2be
0283:    83 FA 08                cmp      edx, 8
0286:    7C 10                   jl       0x298    ; Conditional jump
0288:    66 0F 28 C8             movapd   xmm1, xmm0
028C:    E8 2D 00 00 00          call     0x2be
0291:    83 FA 0A                cmp      edx, 0xa
0294:    7C 10                   jl       0x2a6    ; Conditional jump
0296:    EB 05                   jmp      0x29d
0298:    83 FA 06                cmp      edx, 6
029B:    7C 09                   jl       0x2a6    ; Conditional jump
029D:    66 0F 28 CC             movapd   xmm1, xmm4
02A1:    E8 18 00 00 00          call     0x2be
02A6:    D1 EA                   shr      edx, 1
02A8:    73 09                   jae      0x2b3    ; Conditional jump
02AA:    66 0F 28 CB             movapd   xmm1, xmm3
02AE:    E8 0B 00 00 00          call     0x2be
02B3:    83 F9 00                cmp      ecx, 0
02B6:    74 05                   je       0x2bd    ; Conditional jump
02B8:    66 0F C6 C0 01          shufpd   xmm0, xmm0, 1
02BD:    C3                      ret
02BE:    66 0F 70 D0 4E          pshufd   xmm2, xmm0, 0x4e
02C3:    66 0F 59 C1             mulpd    xmm0, xmm1
02C7:    66 0F 59 D1             mulpd    xmm2, xmm1
02CB:    66 0F 57 C6             xorpd    xmm0, xmm6
02CF:    66 0F 7C C2             haddpd   xmm0, xmm2
02D3:    C3                      ret
