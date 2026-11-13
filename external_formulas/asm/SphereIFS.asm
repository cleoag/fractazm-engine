; Disassembly: SphereIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 184 bytes, decoded: 184 bytes (100.0%)
; DEoption: 20

0000:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0005:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000A:    66 0F 59 47 E0          mulpd    xmm0, xmmword ptr [edi - 0x20]
000F:    F2 0F 59 4F E0          mulsd    xmm1, qword ptr [edi - 0x20]
0014:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
0019:    F2 0F 58 4F D0          addsd    xmm1, qword ptr [edi - 0x30]
001E:    83 7F 9C 00             cmp      dword ptr [edi - 0x64], 0
0022:    74 19                   je       0x3d    ; Conditional jump
0024:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
0029:    F2 0F 59 6F E0          mulsd    xmm5, qword ptr [edi - 0x20]
002E:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0033:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
0038:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
003D:    66 0F 28 5F A0          movapd   xmm3, xmmword ptr [edi - 0x60]
0042:    F2 0F 10 67 B0          movsd    xmm4, qword ptr [edi - 0x50]
0047:    66 0F 59 C3             mulpd    xmm0, xmm3
004B:    F2 0F 59 CC             mulsd    xmm1, xmm4
004F:    66 0F 59 C0             mulpd    xmm0, xmm0
0053:    F2 0F 59 C9             mulsd    xmm1, xmm1
0057:    F2 0F 58 C8             addsd    xmm1, xmm0
005B:    66 0F 15 C0             unpckhpd xmm0, xmm0
005F:    F2 0F 58 C8             addsd    xmm1, xmm0
0063:    F2 0F 51 C9             sqrtsd   xmm1, xmm1
0067:    F2 0F 10 57 88          movsd    xmm2, qword ptr [edi - 0x78]
006C:    F2 0F 5C D1             subsd    xmm2, xmm1
0070:    F2 0F 5C 4F F0          subsd    xmm1, qword ptr [edi - 0x10]
0075:    F2 0F 5F CA             maxsd    xmm1, xmm2
0079:    66 0F 54 1F             andpd    xmm3, xmmword ptr [edi]
007D:    66 0F 54 27             andpd    xmm4, xmmword ptr [edi]
0081:    F2 0F 5F E3             maxsd    xmm4, xmm3
0085:    66 0F 15 DB             unpckhpd xmm3, xmm3
0089:    F2 0F 5F E3             maxsd    xmm4, xmm3
008D:    F2 0F 5E CC             divsd    xmm1, xmm4
0091:    83 7F 9C 00             cmp      dword ptr [edi - 0x64], 0
0095:    75 05                   jne      0x9c    ; Conditional jump
0097:    F2 0F 5E 4F E0          divsd    xmm1, qword ptr [edi - 0x20]
009C:    8B 47 98                mov      eax, dword ptr [edi - 0x68]
009F:    F2 0F 11 4E E0          movsd    qword ptr [esi - 0x20], xmm1
00A4:    83 E0 03                and      eax, 3
00A7:    DD 44 C6 80             fld      qword ptr [esi + eax*8 - 0x80]
00AB:    D8 4F 90                fmul     dword ptr [edi - 0x70]
00AE:    D8 47 94                fadd     dword ptr [edi - 0x6c]
00B1:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00B7:    C3                      ret
