; Disassembly: _invtorical2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 82 bytes, decoded: 82 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 50 00 00 00       sub      esp, 0x50
0009:    56                      push     esi
000A:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000D:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
0010:    DD 02                   fld      qword ptr [edx]
0012:    DC 46 E8                fadd     qword ptr [esi - 0x18]
0015:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0017:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
001A:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
001D:    DD 01                   fld      qword ptr [ecx]
001F:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0021:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0024:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0027:    90                      nop
0028:    DD 00                   fld      qword ptr [eax]
002A:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
002D:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0030:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0033:    90                      nop
0034:    DD 00                   fld      qword ptr [eax]
0036:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0039:    DD 19                   fstp     qword ptr [ecx]
003B:    DD 45 D0                fld      qword ptr [ebp - 0x30]
003E:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0041:    DD 1A                   fstp     qword ptr [edx]
0043:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0046:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0049:    DD 18                   fstp     qword ptr [eax]
004B:    5E                      pop      esi
004C:    89 EC                   mov      esp, ebp
004E:    5D                      pop      ebp
004F:    C2 08 00                ret      8
