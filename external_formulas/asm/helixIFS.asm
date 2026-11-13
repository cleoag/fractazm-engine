; Disassembly: helixIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 231 bytes, decoded: 231 bytes (100.0%)
; DEoption: 20

0000:    55                      push     ebp
0001:    89 E5                   mov      ebp, esp
0003:    83 EC 10                sub      esp, 0x10
0006:    81 E4 F0 FF FF FF       and      esp, 0xfffffff0
000C:    66 0F 10 56 90          movupd   xmm2, xmmword ptr [esi - 0x70]
0011:    F2 0F 10 4E 88          movsd    xmm1, qword ptr [esi - 0x78]
0016:    66 0F 59 57 D0          mulpd    xmm2, xmmword ptr [edi - 0x30]
001B:    F2 0F 59 4F D0          mulsd    xmm1, qword ptr [edi - 0x30]
0020:    66 0F 58 57 C0          addpd    xmm2, xmmword ptr [edi - 0x40]
0025:    F2 0F 58 4F B8          addsd    xmm1, qword ptr [edi - 0x48]
002A:    83 7F AC 00             cmp      dword ptr [edi - 0x54], 0
002E:    74 19                   je       0x49    ; Conditional jump
0030:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
0035:    F2 0F 59 6F D0          mulsd    xmm5, qword ptr [edi - 0x30]
003A:    66 0F 11 56 90          movupd   xmmword ptr [esi - 0x70], xmm2
003F:    F2 0F 11 4E 88          movsd    qword ptr [esi - 0x78], xmm1
0044:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0049:    8B 47 A4                mov      eax, dword ptr [edi - 0x5c]
004C:    83 7F A8 00             cmp      dword ptr [edi - 0x58], 0
0050:    75 07                   jne      0x59    ; Conditional jump
0052:    F2 0F 11 0C 24          movsd    qword ptr [esp], xmm1
0057:    EB 1A                   jmp      0x73
0059:    83 7F A8 01             cmp      dword ptr [edi - 0x58], 1
005D:    74 0B                   je       0x6a    ; Conditional jump
005F:    66 0F 17 14 24          movhpd   qword ptr [esp], xmm2
0064:    66 0F 14 D1             unpcklpd xmm2, xmm1
0068:    EB 09                   jmp      0x73
006A:    F2 0F 11 14 24          movsd    qword ptr [esp], xmm2
006F:    F2 0F 10 D1             movsd    xmm2, xmm1
0073:    48                      dec      eax
0074:    83 E0 0F                and      eax, 0xf
0077:    D9 EB                   fldpi    ; Load pi
0079:    83 F8 01                cmp      eax, 1
007C:    7E 0E                   jle      0x8c    ; Conditional jump
007E:    40                      inc      eax
007F:    89 44 24 08             mov      dword ptr [esp + 8], eax
0083:    DB 44 24 08             fild     dword ptr [esp + 8]
0087:    DE F9                   fdivp    st(1)
0089:    48                      dec      eax
008A:    D8 C0                   fadd     st(0)
008C:    DD 04 24                fld      qword ptr [esp]
008F:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0092:    F2 0F 10 07             movsd    xmm0, qword ptr [edi]
0096:    D9 C0                   fld      st(0)
0098:    D9 FB                   fsincos    ; ST(0)=cos, push sin
009A:    DC 4F E0                fmul     qword ptr [edi - 0x20]
009D:    DD 1C 24                fstp     qword ptr [esp]
00A0:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00A3:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00A7:    66 0F 28 DA             movapd   xmm3, xmm2
00AB:    D8 C1                   fadd     st(1)
00AD:    66 0F 5C 1C 24          subpd    xmm3, xmmword ptr [esp]
00B2:    66 0F 59 DB             mulpd    xmm3, xmm3
00B6:    66 0F 70 E3 4E          pshufd   xmm4, xmm3, 0x4e
00BB:    F2 0F 58 DC             addsd    xmm3, xmm4
00BF:    F2 0F 51 DB             sqrtsd   xmm3, xmm3
00C3:    F2 0F 5D C3             minsd    xmm0, xmm3
00C7:    83 E8 01                sub      eax, 1
00CA:    79 CA                   jns      0x96    ; Conditional jump
00CC:    F2 0F 5C 47 B0          subsd    xmm0, qword ptr [edi - 0x50]
00D1:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00D3:    83 7F AC 00             cmp      dword ptr [edi - 0x54], 0
00D7:    75 05                   jne      0xde    ; Conditional jump
00D9:    F2 0F 5E 47 D0          divsd    xmm0, qword ptr [edi - 0x30]
00DE:    F2 0F 11 46 E0          movsd    qword ptr [esi - 0x20], xmm0
00E3:    89 EC                   mov      esp, ebp
00E5:    5D                      pop      ebp
00E6:    C3                      ret
