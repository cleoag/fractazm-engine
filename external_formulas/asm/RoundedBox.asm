; Disassembly: RoundedBox.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 170 bytes, decoded: 170 bytes (100.0%)
; DEoption: 20

0000:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0005:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000A:    66 0F 59 47 D0          mulpd    xmm0, xmmword ptr [edi - 0x30]
000F:    F2 0F 59 4F D0          mulsd    xmm1, qword ptr [edi - 0x30]
0014:    66 0F 58 47 B0          addpd    xmm0, xmmword ptr [edi - 0x50]
0019:    F2 0F 58 4F C0          addsd    xmm1, qword ptr [edi - 0x40]
001E:    66 0F 57 E4             xorpd    xmm4, xmm4
0022:    83 7F 9C 00             cmp      dword ptr [edi - 0x64], 0
0026:    74 19                   je       0x41    ; Conditional jump
0028:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002D:    F2 0F 59 6F D0          mulsd    xmm5, qword ptr [edi - 0x30]
0032:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0037:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
003C:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0041:    66 0F 54 07             andpd    xmm0, xmmword ptr [edi]
0045:    66 0F 54 0F             andpd    xmm1, xmmword ptr [edi]
0049:    66 0F 5C 47 E0          subpd    xmm0, xmmword ptr [edi - 0x20]
004E:    F2 0F 5C 4F F0          subsd    xmm1, qword ptr [edi - 0x10]
0053:    66 0F 58 47 A0          addpd    xmm0, xmmword ptr [edi - 0x60]
0058:    F2 0F 58 4F A0          addsd    xmm1, qword ptr [edi - 0x60]
005D:    66 0F 5F C4             maxpd    xmm0, xmm4
0061:    F2 0F 5F CC             maxsd    xmm1, xmm4
0065:    66 0F 59 C0             mulpd    xmm0, xmm0
0069:    F2 0F 59 C9             mulsd    xmm1, xmm1
006D:    F2 0F 58 C8             addsd    xmm1, xmm0
0071:    66 0F C6 C0 01          shufpd   xmm0, xmm0, 1
0076:    F2 0F 58 C8             addsd    xmm1, xmm0
007A:    F2 0F 51 C9             sqrtsd   xmm1, xmm1
007E:    F2 0F 5C 4F A0          subsd    xmm1, qword ptr [edi - 0x60]
0083:    83 7F 9C 00             cmp      dword ptr [edi - 0x64], 0
0087:    75 05                   jne      0x8e    ; Conditional jump
0089:    F2 0F 5E 4F D0          divsd    xmm1, qword ptr [edi - 0x30]
008E:    8B 47 98                mov      eax, dword ptr [edi - 0x68]
0091:    F2 0F 11 4E E0          movsd    qword ptr [esi - 0x20], xmm1
0096:    83 E0 03                and      eax, 3
0099:    DD 44 C6 80             fld      qword ptr [esi + eax*8 - 0x80]
009D:    D8 4F 90                fmul     dword ptr [edi - 0x70]
00A0:    D8 47 94                fadd     dword ptr [edi - 0x6c]
00A3:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
00A9:    C3                      ret
