; Disassembly: Aex-Octo.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 72 bytes, decoded: 72 bytes (100.0%)

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
0010:    D9 D0                   fnop
0012:    DD 03                   fld      qword ptr [ebx]
0014:    DC 09                   fmul     qword ptr [ecx]
0016:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0019:    DC 46 18                fadd     qword ptr [esi + 0x18]
001C:    90                      nop
001D:    DD 03                   fld      qword ptr [ebx]
001F:    D8 C8                   fmul     st(0)
0021:    DD 01                   fld      qword ptr [ecx]
0023:    D8 C8                   fmul     st(0)
0025:    DE E1                   fsubrp   st(1)
0027:    DC 4F E8                fmul     qword ptr [edi - 0x18]
002A:    DC 46 20                fadd     qword ptr [esi + 0x20]
002D:    90                      nop
002E:    DD 02                   fld      qword ptr [edx]
0030:    DC 46 28                fadd     qword ptr [esi + 0x28]
0033:    90                      nop
0034:    DD 19                   fstp     qword ptr [ecx]
0036:    DD 1A                   fstp     qword ptr [edx]
0038:    DD 1B                   fstp     qword ptr [ebx]
003A:    DD D8                   fstp     st(0)
003C:    D9 D0                   fnop
003E:    8B C3                   mov      eax, ebx
0040:    5F                      pop      edi
0041:    5E                      pop      esi
0042:    58                      pop      eax
0043:    5B                      pop      ebx
0044:    5D                      pop      ebp
0045:    C2 08 00                ret      8
