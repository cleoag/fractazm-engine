; Disassembly: _updateC.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 70 bytes, decoded: 70 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    D9 D0                   fnop
000D:    DD 00                   fld      qword ptr [eax]
000F:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0012:    DD 46 18                fld      qword ptr [esi + 0x18]
0015:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0018:    DE C1                   faddp    st(1)
001A:    DD 5E 18                fstp     qword ptr [esi + 0x18]
001D:    90                      nop
001E:    DD 02                   fld      qword ptr [edx]
0020:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0023:    DD 46 20                fld      qword ptr [esi + 0x20]
0026:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0029:    DE C1                   faddp    st(1)
002B:    DD 5E 20                fstp     qword ptr [esi + 0x20]
002E:    90                      nop
002F:    DD 01                   fld      qword ptr [ecx]
0031:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0034:    DD 46 28                fld      qword ptr [esi + 0x28]
0037:    DC 4F E8                fmul     qword ptr [edi - 0x18]
003A:    DE C1                   faddp    st(1)
003C:    DD 5E 28                fstp     qword ptr [esi + 0x28]
003F:    90                      nop
0040:    5F                      pop      edi
0041:    5E                      pop      esi
0042:    5D                      pop      ebp
0043:    C2 08 00                ret      8
