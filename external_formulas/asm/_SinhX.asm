; Disassembly: _SinhX.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 67 bytes, decoded: 67 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    50                      push     eax
0005:    56                      push     esi
0006:    57                      push     edi
0007:    90                      nop
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B D8                   mov      ebx, eax
000D:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0010:    DD 03                   fld      qword ptr [ebx]
0012:    DD 47 E0                fld      qword ptr [edi - 0x20]
0015:    DE C9                   fmulp    st(1)
0017:    DD 47 F0                fld      qword ptr [edi - 0x10]
001A:    DE C1                   faddp    st(1)
001C:    DD 13                   fst      qword ptr [ebx]
001E:    8A 43 07                mov      al, byte ptr [ebx + 7]
0021:    24 80                   and      al, 0x80
0023:    90                      nop
0024:    D9 E1                   fabs    ; |ST(0)|
0026:    DD 47 E8                fld      qword ptr [edi - 0x18]
0029:    D9 E1                   fabs    ; |ST(0)|
002B:    DE C1                   faddp    st(1)
002D:    DD 47 D8                fld      qword ptr [edi - 0x28]
0030:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0032:    D9 F1                   fyl2x
0034:    DD 1B                   fstp     qword ptr [ebx]
0036:    30 43 07                xor      byte ptr [ebx + 7], al
0039:    8B C3                   mov      eax, ebx
003B:    5F                      pop      edi
003C:    5E                      pop      esi
003D:    58                      pop      eax
003E:    5B                      pop      ebx
003F:    5D                      pop      ebp
0040:    C2 08 00                ret      8
