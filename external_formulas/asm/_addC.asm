; Disassembly: _addC.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 53 bytes, decoded: 53 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 01                   fld      qword ptr [ecx]
000D:    DD 02                   fld      qword ptr [edx]
000F:    DD 00                   fld      qword ptr [eax]
0011:    DD 46 18                fld      qword ptr [esi + 0x18]
0014:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0017:    DE C1                   faddp    st(1)
0019:    DD 18                   fstp     qword ptr [eax]
001B:    DD 46 20                fld      qword ptr [esi + 0x20]
001E:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0021:    DE C1                   faddp    st(1)
0023:    DD 1A                   fstp     qword ptr [edx]
0025:    DD 46 28                fld      qword ptr [esi + 0x28]
0028:    DC 4F E0                fmul     qword ptr [edi - 0x20]
002B:    DE C1                   faddp    st(1)
002D:    DD 19                   fstp     qword ptr [ecx]
002F:    5F                      pop      edi
0030:    5E                      pop      esi
0031:    5D                      pop      ebp
0032:    C2 08 00                ret      8
