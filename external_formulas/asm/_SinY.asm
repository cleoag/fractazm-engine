; Disassembly: _SinY.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 43 bytes, decoded: 43 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    8B 77 F4                mov      esi, dword ptr [edi - 0xc]
000E:    83 E6 03                and      esi, 3
0011:    DD 04 F0                fld      qword ptr [eax + esi*8]
0014:    D8 67 F0                fsub     dword ptr [edi - 0x10]
0017:    D8 4F EC                fmul     dword ptr [edi - 0x14]
001A:    D9 FE                   fsin    ; sin(ST(0))
001C:    D8 4F E8                fmul     dword ptr [edi - 0x18]
001F:    D8 47 E4                fadd     dword ptr [edi - 0x1c]
0022:    DD 1C F0                fstp     qword ptr [eax + esi*8]
0025:    5F                      pop      edi
0026:    5E                      pop      esi
0027:    5D                      pop      ebp
0028:    C2 08 00                ret      8
