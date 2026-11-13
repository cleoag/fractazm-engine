; Disassembly: DecoCubeIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 227 bytes, decoded: 227 bytes (100.0%)
; DEoption: 20

0000:    83 C4 E4                add      esp, -0x1c
0003:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0008:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000D:    66 0F 59 47 E0          mulpd    xmm0, xmmword ptr [edi - 0x20]
0012:    F2 0F 59 4F E0          mulsd    xmm1, qword ptr [edi - 0x20]
0017:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
001C:    F2 0F 58 4F D0          addsd    xmm1, qword ptr [edi - 0x30]
0021:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0025:    74 19                   je       0x40    ; Conditional jump
0027:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002C:    F2 0F 59 6F E0          mulsd    xmm5, qword ptr [edi - 0x20]
0031:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0036:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
003B:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0040:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0045:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
004B:    DD 04 24                fld      qword ptr [esp]
004E:    D8 C8                   fmul     st(0)
0050:    DD 1C 24                fstp     qword ptr [esp]
0053:    DD 44 24 08             fld      qword ptr [esp + 8]
0057:    D8 C8                   fmul     st(0)
0059:    DD 5C 24 08             fstp     qword ptr [esp + 8]
005D:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0061:    D8 C8                   fmul     st(0)
0063:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0067:    DD 04 24                fld      qword ptr [esp]
006A:    DC 44 24 08             fadd     qword ptr [esp + 8]
006E:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0071:    D8 C8                   fmul     st(0)
0073:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0077:    DC 67 B8                fsub     qword ptr [edi - 0x48]
007A:    D8 C8                   fmul     st(0)
007C:    DE C1                   faddp    st(1)
007E:    DD 04 24                fld      qword ptr [esp]
0081:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
0085:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0088:    D8 C8                   fmul     st(0)
008A:    DD 44 24 08             fld      qword ptr [esp + 8]
008E:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0091:    D8 C8                   fmul     st(0)
0093:    DE C1                   faddp    st(1)
0095:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0099:    DC 44 24 08             fadd     qword ptr [esp + 8]
009D:    DC 67 F0                fsub     qword ptr [edi - 0x10]
00A0:    D8 C8                   fmul     st(0)
00A2:    DD 04 24                fld      qword ptr [esp]
00A5:    DC 67 B8                fsub     qword ptr [edi - 0x48]
00A8:    D8 C8                   fmul     st(0)
00AA:    DE C1                   faddp    st(1)
00AC:    DE C9                   fmulp    st(1)
00AE:    DE C9                   fmulp    st(1)
00B0:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B2:    D9 FA                   fsqrt    ; sqrt(ST(0))
00B4:    DC 67 AC                fsub     qword ptr [edi - 0x54]
00B7:    90                      nop
00B8:    DD 07                   fld      qword ptr [edi]
00BA:    D8 D9                   fcomp    st(1)
00BC:    9B                      wait
00BD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00BF:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00C1:    73 0F                   jae      0xd2    ; Conditional jump
00C3:    DD D8                   fstp     st(0)
00C5:    DD 04 24                fld      qword ptr [esp]
00C8:    DC 44 24 08             fadd     qword ptr [esp + 8]
00CC:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
00D0:    D9 FA                   fsqrt    ; sqrt(ST(0))
00D2:    90                      nop
00D3:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
00D7:    75 03                   jne      0xdc    ; Conditional jump
00D9:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
00DC:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00DF:    83 C4 1C                add      esp, 0x1c
00E2:    C3                      ret
