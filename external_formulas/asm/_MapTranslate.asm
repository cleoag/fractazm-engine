; Disassembly: _MapTranslate.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 80 bytes, decoded: 80 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
000E:    DD 00                   fld      qword ptr [eax]
0010:    DD 02                   fld      qword ptr [edx]
0012:    DD 01                   fld      qword ptr [ecx]
0014:    89 C1                   mov      ecx, eax
0016:    8B 56 F4                mov      edx, dword ptr [esi - 0xc]
0019:    FF 97 60 01 00 00       call     dword ptr [edi + 0x160]
001F:    DD 03                   fld      qword ptr [ebx]
0021:    D8 46 F0                fadd     dword ptr [esi - 0x10]
0024:    D8 4E E4                fmul     dword ptr [esi - 0x1c]
0027:    DE C3                   faddp    st(3)
0029:    DD 43 08                fld      qword ptr [ebx + 8]
002C:    D8 46 EC                fadd     dword ptr [esi - 0x14]
002F:    D8 4E E0                fmul     dword ptr [esi - 0x20]
0032:    DE C2                   faddp    st(2)
0034:    DD 43 10                fld      qword ptr [ebx + 0x10]
0037:    D8 46 E8                fadd     dword ptr [esi - 0x18]
003A:    D8 4E DC                fmul     dword ptr [esi - 0x24]
003D:    DE C1                   faddp    st(1)
003F:    DD 5B 10                fstp     qword ptr [ebx + 0x10]
0042:    DD 5B 08                fstp     qword ptr [ebx + 8]
0045:    DD 1B                   fstp     qword ptr [ebx]
0047:    89 D8                   mov      eax, ebx
0049:    5B                      pop      ebx
004A:    5F                      pop      edi
004B:    5E                      pop      esi
004C:    5D                      pop      ebp
004D:    C2 08 00                ret      8
