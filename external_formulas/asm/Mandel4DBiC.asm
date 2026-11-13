; Disassembly: Mandel4DBiC.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 93 bytes, decoded: 93 bytes (100.0%)
; DEoption: 4

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
0012:    D8 C8                   fmul     st(0)
0014:    DD 02                   fld      qword ptr [edx]
0016:    D8 C8                   fmul     st(0)
0018:    DE E9                   fsubp    st(1)
001A:    DC 46 18                fadd     qword ptr [esi + 0x18]
001D:    D9 D0                   fnop
001F:    DD 03                   fld      qword ptr [ebx]
0021:    DC 0A                   fmul     qword ptr [edx]
0023:    D8 C0                   fadd     st(0)
0025:    DC 46 20                fadd     qword ptr [esi + 0x20]
0028:    D9 D0                   fnop
002A:    DD 01                   fld      qword ptr [ecx]
002C:    D8 C8                   fmul     st(0)
002E:    DD 41 08                fld      qword ptr [ecx + 8]
0031:    D8 C8                   fmul     st(0)
0033:    DE E9                   fsubp    st(1)
0035:    DC 46 28                fadd     qword ptr [esi + 0x28]
0038:    D9 D0                   fnop
003A:    DD 01                   fld      qword ptr [ecx]
003C:    DC 49 08                fmul     qword ptr [ecx + 8]
003F:    D8 C0                   fadd     st(0)
0041:    DC 46 C8                fadd     qword ptr [esi - 0x38]
0044:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0047:    D9 D0                   fnop
0049:    DD 59 08                fstp     qword ptr [ecx + 8]
004C:    DD 19                   fstp     qword ptr [ecx]
004E:    DD 1A                   fstp     qword ptr [edx]
0050:    DD 1B                   fstp     qword ptr [ebx]
0052:    90                      nop
0053:    8B C3                   mov      eax, ebx
0055:    5F                      pop      edi
0056:    5E                      pop      esi
0057:    58                      pop      eax
0058:    5B                      pop      ebx
0059:    5D                      pop      ebp
005A:    C2 08 00                ret      8
