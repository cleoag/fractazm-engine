; Disassembly: cylinderIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 191 bytes, decoded: 191 bytes (100.0%)
; DEoption: 20

0000:    66 0F 10 46 90          movupd   xmm0, xmmword ptr [esi - 0x70]
0005:    F2 0F 10 4E 88          movsd    xmm1, qword ptr [esi - 0x78]
000A:    66 0F 59 47 D0          mulpd    xmm0, xmmword ptr [edi - 0x30]
000F:    F2 0F 59 4F D0          mulsd    xmm1, qword ptr [edi - 0x30]
0014:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
0019:    F2 0F 58 4F B8          addsd    xmm1, qword ptr [edi - 0x48]
001E:    83 7F A4 00             cmp      dword ptr [edi - 0x5c], 0
0022:    74 19                   je       0x3d    ; Conditional jump
0024:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
0029:    F2 0F 59 6F D0          mulsd    xmm5, qword ptr [edi - 0x30]
002E:    66 0F 11 46 90          movupd   xmmword ptr [esi - 0x70], xmm0
0033:    F2 0F 11 4E 88          movsd    qword ptr [esi - 0x78], xmm1
0038:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
003D:    66 0F 28 E1             movapd   xmm4, xmm1
0041:    66 0F 59 C0             mulpd    xmm0, xmm0
0045:    66 0F 54 0F             andpd    xmm1, xmmword ptr [edi]
0049:    66 0F 70 D0 4E          pshufd   xmm2, xmm0, 0x4e
004E:    66 0F 28 D9             movapd   xmm3, xmm1
0052:    F2 0F 5C 4F E0          subsd    xmm1, qword ptr [edi - 0x20]
0057:    F2 0F 58 C2             addsd    xmm0, xmm2
005B:    F2 0F 5D 5F E0          minsd    xmm3, qword ptr [edi - 0x20]
0060:    F2 0F 59 67 A8          mulsd    xmm4, qword ptr [edi - 0x58]
0065:    F2 0F 59 DB             mulsd    xmm3, xmm3
0069:    F2 0F 51 C0             sqrtsd   xmm0, xmm0
006D:    F2 0F 10 57 90          movsd    xmm2, qword ptr [edi - 0x70]
0072:    F2 0F 59 5F B0          mulsd    xmm3, qword ptr [edi - 0x50]
0077:    F2 0F 5C D0             subsd    xmm2, xmm0
007B:    F2 0F 5C 47 F0          subsd    xmm0, qword ptr [edi - 0x10]
0080:    F2 0F 5C D3             subsd    xmm2, xmm3
0084:    F2 0F 58 C3             addsd    xmm0, xmm3
0088:    F2 0F 5C D4             subsd    xmm2, xmm4
008C:    F2 0F 58 C4             addsd    xmm0, xmm4
0090:    F2 0F 5F C2             maxsd    xmm0, xmm2
0094:    F2 0F 5F C1             maxsd    xmm0, xmm1
0098:    83 7F A4 00             cmp      dword ptr [edi - 0x5c], 0
009C:    75 05                   jne      0xa3    ; Conditional jump
009E:    F2 0F 5E 47 D0          divsd    xmm0, qword ptr [edi - 0x30]
00A3:    8B 47 A0                mov      eax, dword ptr [edi - 0x60]
00A6:    F2 0F 11 46 E0          movsd    qword ptr [esi - 0x20], xmm0
00AB:    83 E0 03                and      eax, 3
00AE:    DD 44 C6 80             fld      qword ptr [esi + eax*8 - 0x80]
00B2:    D8 4F 98                fmul     dword ptr [edi - 0x68]
00B5:    D8 47 9C                fadd     dword ptr [edi - 0x64]
00B8:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00BE:    C3                      ret
