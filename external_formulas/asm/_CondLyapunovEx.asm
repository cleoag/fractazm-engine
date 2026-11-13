; Disassembly: _CondLyapunovEx.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 341 bytes, decoded: 341 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    50                      push     eax
0004:    52                      push     edx
0005:    53                      push     ebx
0006:    56                      push     esi
0007:    57                      push     edi
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    FF 86 D0 00 00 00       inc      dword ptr [esi + 0xd0]
0014:    8B 86 D0 00 00 00       mov      eax, dword ptr [esi + 0xd0]
001A:    8B D0                   mov      edx, eax
001C:    83 E2 0F                and      edx, 0xf
001F:    90                      nop
0020:    8B 47 F4                mov      eax, dword ptr [edi - 0xc]
0023:    4A                      dec      edx
0024:    83 FA 00                cmp      edx, 0
0027:    7E 04                   jle      0x2d    ; Conditional jump
0029:    D1 F8                   sar      eax, 1
002B:    EB F6                   jmp      0x23
002D:    83 E0 01                and      eax, 1
0030:    0F 85 8F 00 00 00       jne      0xc5    ; Conditional jump
0036:    90                      nop
0037:    8B 47 E8                mov      eax, dword ptr [edi - 0x18]
003A:    83 F8 00                cmp      eax, 0
003D:    7D 07                   jge      0x46    ; Conditional jump
003F:    B8 00 00 00 00          mov      eax, 0
0044:    EB 0A                   jmp      0x50
0046:    83 F8 64                cmp      eax, 0x64
0049:    7E 05                   jle      0x50    ; Conditional jump
004B:    B8 64 00 00 00          mov      eax, 0x64
0050:    8B 7F F0                mov      edi, dword ptr [edi - 0x10]
0053:    83 FF 01                cmp      edi, 1
0056:    7D 07                   jge      0x5f    ; Conditional jump
0058:    BF 01 00 00 00          mov      edi, 1
005D:    EB 0A                   jmp      0x69
005F:    83 FF 06                cmp      edi, 6
0062:    7E 05                   jle      0x69    ; Conditional jump
0064:    BF 06 00 00 00          mov      edi, 6
0069:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
006D:    83 3F 00                cmp      dword ptr [edi], 0
0070:    7D 05                   jge      0x77    ; Conditional jump
0072:    0D 00 00 00 80          or       eax, 0x80000000
0077:    89 07                   mov      dword ptr [edi], eax
0079:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
007C:    8B 47 E4                mov      eax, dword ptr [edi - 0x1c]
007F:    83 F8 01                cmp      eax, 1
0082:    7D 07                   jge      0x8b    ; Conditional jump
0084:    B8 01 00 00 00          mov      eax, 1
0089:    EB 0A                   jmp      0x95
008B:    83 F8 64                cmp      eax, 0x64
008E:    7E 05                   jle      0x95    ; Conditional jump
0090:    B8 64 00 00 00          mov      eax, 0x64
0095:    8B 7F EC                mov      edi, dword ptr [edi - 0x14]
0098:    83 FF 01                cmp      edi, 1
009B:    7D 07                   jge      0xa4    ; Conditional jump
009D:    BF 01 00 00 00          mov      edi, 1
00A2:    EB 0A                   jmp      0xae
00A4:    83 FF 06                cmp      edi, 6
00A7:    7E 05                   jle      0xae    ; Conditional jump
00A9:    BF 06 00 00 00          mov      edi, 6
00AE:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
00B2:    83 3F 00                cmp      dword ptr [edi], 0
00B5:    7D 05                   jge      0xbc    ; Conditional jump
00B7:    0D 00 00 00 80          or       eax, 0x80000000
00BC:    89 07                   mov      dword ptr [edi], eax
00BE:    E9 89 00 00 00          jmp      0x14c
00C3:    90                      nop
00C4:    90                      nop
00C5:    8B 47 E8                mov      eax, dword ptr [edi - 0x18]
00C8:    83 F8 00                cmp      eax, 0
00CB:    7D 07                   jge      0xd4    ; Conditional jump
00CD:    B8 00 00 00 00          mov      eax, 0
00D2:    EB 0A                   jmp      0xde
00D4:    83 F8 64                cmp      eax, 0x64
00D7:    7E 05                   jle      0xde    ; Conditional jump
00D9:    B8 64 00 00 00          mov      eax, 0x64
00DE:    8B 7F EC                mov      edi, dword ptr [edi - 0x14]
00E1:    83 FF 01                cmp      edi, 1
00E4:    7D 07                   jge      0xed    ; Conditional jump
00E6:    BF 01 00 00 00          mov      edi, 1
00EB:    EB 0A                   jmp      0xf7
00ED:    83 FF 06                cmp      edi, 6
00F0:    7E 05                   jle      0xf7    ; Conditional jump
00F2:    BF 06 00 00 00          mov      edi, 6
00F7:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
00FB:    83 3F 00                cmp      dword ptr [edi], 0
00FE:    7D 05                   jge      0x105    ; Conditional jump
0100:    0D 00 00 00 80          or       eax, 0x80000000
0105:    89 07                   mov      dword ptr [edi], eax
0107:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
010A:    8B 47 E4                mov      eax, dword ptr [edi - 0x1c]
010D:    83 F8 01                cmp      eax, 1
0110:    7D 07                   jge      0x119    ; Conditional jump
0112:    B8 01 00 00 00          mov      eax, 1
0117:    EB 0A                   jmp      0x123
0119:    83 F8 64                cmp      eax, 0x64
011C:    7E 05                   jle      0x123    ; Conditional jump
011E:    B8 64 00 00 00          mov      eax, 0x64
0123:    8B 7F F0                mov      edi, dword ptr [edi - 0x10]
0126:    83 FF 01                cmp      edi, 1
0129:    7D 07                   jge      0x132    ; Conditional jump
012B:    BF 01 00 00 00          mov      edi, 1
0130:    EB 0A                   jmp      0x13c
0132:    83 FF 06                cmp      edi, 6
0135:    7E 05                   jle      0x13c    ; Conditional jump
0137:    BF 06 00 00 00          mov      edi, 6
013C:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
0140:    83 3F 00                cmp      dword ptr [edi], 0
0143:    7D 05                   jge      0x14a    ; Conditional jump
0145:    0D 00 00 00 80          or       eax, 0x80000000
014A:    89 07                   mov      dword ptr [edi], eax
014C:    5F                      pop      edi
014D:    5E                      pop      esi
014E:    5B                      pop      ebx
014F:    5A                      pop      edx
0150:    58                      pop      eax
0151:    5D                      pop      ebp
0152:    C2 08 00                ret      8
