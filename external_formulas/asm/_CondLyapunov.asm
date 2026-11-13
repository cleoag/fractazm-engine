; Disassembly: _CondLyapunov.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 245 bytes, decoded: 245 bytes (100.0%)
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
0030:    75 60                   jne      0x92    ; Conditional jump
0032:    8B 7F F0                mov      edi, dword ptr [edi - 0x10]
0035:    83 FF 01                cmp      edi, 1
0038:    7D 07                   jge      0x41    ; Conditional jump
003A:    BF 01 00 00 00          mov      edi, 1
003F:    EB 0A                   jmp      0x4b
0041:    83 FF 06                cmp      edi, 6
0044:    7E 05                   jle      0x4b    ; Conditional jump
0046:    BF 06 00 00 00          mov      edi, 6
004B:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
004F:    83 3F 00                cmp      dword ptr [edi], 0
0052:    7D 05                   jge      0x59    ; Conditional jump
0054:    0D 00 00 00 80          or       eax, 0x80000000
0059:    89 07                   mov      dword ptr [edi], eax
005B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
005E:    33 C0                   xor      eax, eax
0060:    40                      inc      eax
0061:    8B 7F EC                mov      edi, dword ptr [edi - 0x14]
0064:    83 FF 01                cmp      edi, 1
0067:    7D 07                   jge      0x70    ; Conditional jump
0069:    BF 01 00 00 00          mov      edi, 1
006E:    EB 0A                   jmp      0x7a
0070:    83 FF 06                cmp      edi, 6
0073:    7E 05                   jle      0x7a    ; Conditional jump
0075:    BF 06 00 00 00          mov      edi, 6
007A:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
007E:    83 3F 00                cmp      dword ptr [edi], 0
0081:    7D 05                   jge      0x88    ; Conditional jump
0083:    0D 00 00 00 80          or       eax, 0x80000000
0088:    EB 60                   jmp      0xea
008A:    90                      nop
008B:    90                      nop
008C:    90                      nop
008D:    90                      nop
008E:    90                      nop
008F:    90                      nop
0090:    90                      nop
0091:    90                      nop
0092:    33 C0                   xor      eax, eax
0094:    8B 7F EC                mov      edi, dword ptr [edi - 0x14]
0097:    83 FF 01                cmp      edi, 1
009A:    7D 07                   jge      0xa3    ; Conditional jump
009C:    BF 01 00 00 00          mov      edi, 1
00A1:    EB 0A                   jmp      0xad
00A3:    83 FF 06                cmp      edi, 6
00A6:    7E 05                   jle      0xad    ; Conditional jump
00A8:    BF 06 00 00 00          mov      edi, 6
00AD:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
00B1:    83 3F 00                cmp      dword ptr [edi], 0
00B4:    7D 05                   jge      0xbb    ; Conditional jump
00B6:    0D 00 00 00 80          or       eax, 0x80000000
00BB:    89 07                   mov      dword ptr [edi], eax
00BD:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
00C0:    33 C0                   xor      eax, eax
00C2:    40                      inc      eax
00C3:    8B 7F F0                mov      edi, dword ptr [edi - 0x10]
00C6:    83 FF 01                cmp      edi, 1
00C9:    7D 07                   jge      0xd2    ; Conditional jump
00CB:    BF 01 00 00 00          mov      edi, 1
00D0:    EB 0A                   jmp      0xdc
00D2:    83 FF 06                cmp      edi, 6
00D5:    7E 05                   jle      0xdc    ; Conditional jump
00D7:    BF 06 00 00 00          mov      edi, 6
00DC:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
00E0:    83 3F 00                cmp      dword ptr [edi], 0
00E3:    7D 05                   jge      0xea    ; Conditional jump
00E5:    0D 00 00 00 80          or       eax, 0x80000000
00EA:    89 07                   mov      dword ptr [edi], eax
00EC:    5F                      pop      edi
00ED:    5E                      pop      esi
00EE:    5B                      pop      ebx
00EF:    5A                      pop      edx
00F0:    58                      pop      eax
00F1:    5D                      pop      ebp
00F2:    C2 08 00                ret      8
