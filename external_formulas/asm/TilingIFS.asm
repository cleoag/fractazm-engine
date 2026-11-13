; Disassembly: TilingIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 80 bytes, decoded: 80 bytes (100.0%)
; DEoption: 21

0000:    8B 47 BC                mov      eax, dword ptr [edi - 0x44]
0003:    83 E0 01                and      eax, 1
0006:    C1 E0 03                shl      eax, 3
0009:    BA 08 00 00 00          mov      edx, 8
000E:    29 C2                   sub      edx, eax
0010:    D1 E2                   shl      edx, 1
0012:    66 0F 10 44 30 88       movupd   xmm0, xmmword ptr [eax + esi - 0x78]
0018:    F2 0F 10 5C 32 88       movsd    xmm3, qword ptr [edx + esi - 0x78]
001E:    66 0F 5C 47 E0          subpd    xmm0, xmmword ptr [edi - 0x20]
0023:    66 0F 5E 47 D0          divpd    xmm0, xmmword ptr [edi - 0x30]
0028:    F2 0F 5C 5F F0          subsd    xmm3, qword ptr [edi - 0x10]
002D:    F2 0F E6 D0             cvtpd2dq xmm2, xmm0
0031:    F2 0F 11 5C 32 88       movsd    qword ptr [edx + esi - 0x78], xmm3
0037:    F3 0F E6 D2             cvtdq2pd xmm2, xmm2
003B:    66 0F 5C C2             subpd    xmm0, xmm2
003F:    66 0F 59 47 D0          mulpd    xmm0, xmmword ptr [edi - 0x30]
0044:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
0049:    66 0F 11 44 30 88       movupd   xmmword ptr [eax + esi - 0x78], xmm0
004F:    C3                      ret
