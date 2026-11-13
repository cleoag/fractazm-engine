; Disassembly: _CondItcountR.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 118 bytes, decoded: 118 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    50                      push     eax
0004:    56                      push     esi
0005:    57                      push     edi
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    DD 46 38                fld      qword ptr [esi + 0x38]
000F:    DC 57 F0                fcom     qword ptr [edi - 0x10]
0012:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0014:    80 E4 41                and      ah, 0x41
0017:    74 05                   je       0x1e    ; Conditional jump
0019:    8B 47 E0                mov      eax, dword ptr [edi - 0x20]
001C:    EB 12                   jmp      0x30
001E:    DC 57 E8                fcom     qword ptr [edi - 0x18]
0021:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0023:    80 E4 41                and      ah, 0x41
0026:    74 05                   je       0x2d    ; Conditional jump
0028:    8B 47 DC                mov      eax, dword ptr [edi - 0x24]
002B:    EB 03                   jmp      0x30
002D:    8B 47 D8                mov      eax, dword ptr [edi - 0x28]
0030:    DD D8                   fstp     st(0)
0032:    85 C0                   test     eax, eax
0034:    7D 04                   jge      0x3a    ; Conditional jump
0036:    31 C0                   xor      eax, eax
0038:    EB 0C                   jmp      0x46
003A:    3D 9F 86 01 00          cmp      eax, 0x1869f
003F:    7E 05                   jle      0x46    ; Conditional jump
0041:    B8 9F 86 01 00          mov      eax, 0x1869f
0046:    8B 7F E4                mov      edi, dword ptr [edi - 0x1c]
0049:    83 FF 01                cmp      edi, 1
004C:    7D 07                   jge      0x55    ; Conditional jump
004E:    BF 01 00 00 00          mov      edi, 1
0053:    EB 0A                   jmp      0x5f
0055:    83 FF 06                cmp      edi, 6
0058:    7E 05                   jle      0x5f    ; Conditional jump
005A:    BF 06 00 00 00          mov      edi, 6
005F:    8D 7C BE 48             lea      edi, [esi + edi*4 + 0x48]
0063:    83 3F 00                cmp      dword ptr [edi], 0
0066:    7D 05                   jge      0x6d    ; Conditional jump
0068:    0D 00 00 00 80          or       eax, 0x80000000
006D:    89 07                   mov      dword ptr [edi], eax
006F:    5F                      pop      edi
0070:    5E                      pop      esi
0071:    58                      pop      eax
0072:    5D                      pop      ebp
0073:    C2 08 00                ret      8
