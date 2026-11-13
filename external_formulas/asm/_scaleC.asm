; Disassembly: _scaleC.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 44 bytes, decoded: 44 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 46 18                fld      qword ptr [esi + 0x18]
000E:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0011:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0014:    DD 46 20                fld      qword ptr [esi + 0x20]
0017:    DC 4F E8                fmul     qword ptr [edi - 0x18]
001A:    DD 5E 20                fstp     qword ptr [esi + 0x20]
001D:    DD 46 28                fld      qword ptr [esi + 0x28]
0020:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0023:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0026:    5F                      pop      edi
0027:    5E                      pop      esi
0028:    5D                      pop      ebp
0029:    C2 08 00                ret      8
