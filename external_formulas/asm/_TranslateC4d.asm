; Disassembly: _TranslateC4d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 83 bytes, decoded: 83 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 41 08                fld      qword ptr [ecx + 8]
000E:    DD 01                   fld      qword ptr [ecx]
0010:    DD 02                   fld      qword ptr [edx]
0012:    DD 00                   fld      qword ptr [eax]
0014:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0017:    DD 18                   fstp     qword ptr [eax]
0019:    DC 47 E8                fadd     qword ptr [edi - 0x18]
001C:    DD 1A                   fstp     qword ptr [edx]
001E:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0021:    DD 19                   fstp     qword ptr [ecx]
0023:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0026:    DD 59 08                fstp     qword ptr [ecx + 8]
0029:    DD 46 C8                fld      qword ptr [esi - 0x38]
002C:    DD 46 28                fld      qword ptr [esi + 0x28]
002F:    DD 46 20                fld      qword ptr [esi + 0x20]
0032:    DD 46 18                fld      qword ptr [esi + 0x18]
0035:    DC 47 D0                fadd     qword ptr [edi - 0x30]
0038:    DD 5E 18                fstp     qword ptr [esi + 0x18]
003B:    DC 47 C8                fadd     qword ptr [edi - 0x38]
003E:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0041:    DC 47 C0                fadd     qword ptr [edi - 0x40]
0044:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0047:    DC 47 B8                fadd     qword ptr [edi - 0x48]
004A:    DD 5E C8                fstp     qword ptr [esi - 0x38]
004D:    5F                      pop      edi
004E:    5E                      pop      esi
004F:    5D                      pop      ebp
0050:    C2 08 00                ret      8
