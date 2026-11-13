; Disassembly: _CondItcountSwX2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 277 bytes, decoded: 277 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    50                      push     eax
0004:    52                      push     edx
0005:    56                      push     esi
0006:    57                      push     edi
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000D:    FF 86 D0 00 00 00       inc      dword ptr [esi + 0xd0]
0013:    8B 86 D0 00 00 00       mov      eax, dword ptr [esi + 0xd0]
0019:    8B D0                   mov      edx, eax
001B:    8B 47 F4                mov      eax, dword ptr [edi - 0xc]
001E:    83 F8 01                cmp      eax, 1
0021:    7D 07                   jge      0x2a    ; Conditional jump
0023:    B8 01 00 00 00          mov      eax, 1
0028:    EB 0A                   jmp      0x34
002A:    83 F8 64                cmp      eax, 0x64
002D:    7E 05                   jle      0x34    ; Conditional jump
002F:    B8 64 00 00 00          mov      eax, 0x64
0034:    3B D0                   cmp      edx, eax
0036:    7C 07                   jl       0x3f    ; Conditional jump
0038:    74 4A                   je       0x84    ; Conditional jump
003A:    E9 4B 00 00 00          jmp      0x8a
003F:    8B 47 EC                mov      eax, dword ptr [edi - 0x14]
0042:    83 F8 00                cmp      eax, 0
0045:    7D 07                   jge      0x4e    ; Conditional jump
0047:    B8 00 00 00 00          mov      eax, 0
004C:    EB 0A                   jmp      0x58
004E:    83 F8 64                cmp      eax, 0x64
0051:    7E 05                   jle      0x58    ; Conditional jump
0053:    B8 64 00 00 00          mov      eax, 0x64
0058:    8B 7F F0                mov      edi, dword ptr [edi - 0x10]
005B:    83 FF 01                cmp      edi, 1
005E:    7D 07                   jge      0x67    ; Conditional jump
0060:    BF 01 00 00 00          mov      edi, 1
0065:    EB 0A                   jmp      0x71
0067:    83 FF 06                cmp      edi, 6
006A:    7E 05                   jle      0x71    ; Conditional jump
006C:    BF 06 00 00 00          mov      edi, 6
0071:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
0075:    83 3F 00                cmp      dword ptr [edi], 0
0078:    7D 05                   jge      0x7f    ; Conditional jump
007A:    0D 00 00 00 80          or       eax, 0x80000000
007F:    89 07                   mov      dword ptr [edi], eax
0081:    EB 0D                   jmp      0x90
0083:    90                      nop
0084:    8B 47 E8                mov      eax, dword ptr [edi - 0x18]
0087:    EB B9                   jmp      0x42
0089:    90                      nop
008A:    8B 47 E4                mov      eax, dword ptr [edi - 0x1c]
008D:    EB B3                   jmp      0x42
008F:    90                      nop
0090:    90                      nop
0091:    90                      nop
0092:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0095:    83 C7 EC                add      edi, -0x14
0098:    8B 47 F4                mov      eax, dword ptr [edi - 0xc]
009B:    83 F8 01                cmp      eax, 1
009E:    7D 07                   jge      0xa7    ; Conditional jump
00A0:    B8 01 00 00 00          mov      eax, 1
00A5:    EB 0A                   jmp      0xb1
00A7:    83 F8 64                cmp      eax, 0x64
00AA:    7E 05                   jle      0xb1    ; Conditional jump
00AC:    B8 64 00 00 00          mov      eax, 0x64
00B1:    3B D0                   cmp      edx, eax
00B3:    7C 07                   jl       0xbc    ; Conditional jump
00B5:    74 4A                   je       0x101    ; Conditional jump
00B7:    E9 4B 00 00 00          jmp      0x107
00BC:    8B 47 EC                mov      eax, dword ptr [edi - 0x14]
00BF:    83 F8 00                cmp      eax, 0
00C2:    7D 07                   jge      0xcb    ; Conditional jump
00C4:    B8 00 00 00 00          mov      eax, 0
00C9:    EB 0A                   jmp      0xd5
00CB:    83 F8 64                cmp      eax, 0x64
00CE:    7E 05                   jle      0xd5    ; Conditional jump
00D0:    B8 64 00 00 00          mov      eax, 0x64
00D5:    8B 7F F0                mov      edi, dword ptr [edi - 0x10]
00D8:    83 FF 01                cmp      edi, 1
00DB:    7D 07                   jge      0xe4    ; Conditional jump
00DD:    BF 01 00 00 00          mov      edi, 1
00E2:    EB 0A                   jmp      0xee
00E4:    83 FF 06                cmp      edi, 6
00E7:    7E 05                   jle      0xee    ; Conditional jump
00E9:    BF 06 00 00 00          mov      edi, 6
00EE:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
00F2:    83 3F 00                cmp      dword ptr [edi], 0
00F5:    7D 05                   jge      0xfc    ; Conditional jump
00F7:    0D 00 00 00 80          or       eax, 0x80000000
00FC:    89 07                   mov      dword ptr [edi], eax
00FE:    EB 0D                   jmp      0x10d
0100:    90                      nop
0101:    8B 47 E8                mov      eax, dword ptr [edi - 0x18]
0104:    EB B9                   jmp      0xbf
0106:    90                      nop
0107:    8B 47 E4                mov      eax, dword ptr [edi - 0x1c]
010A:    EB B3                   jmp      0xbf
010C:    90                      nop
010D:    5F                      pop      edi
010E:    5E                      pop      esi
010F:    5A                      pop      edx
0110:    58                      pop      eax
0111:    5D                      pop      ebp
0112:    C2 08 00                ret      8
