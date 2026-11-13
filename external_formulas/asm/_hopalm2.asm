; Disassembly: _hopalm2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 85 bytes, decoded: 85 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B D8                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 03                   fld      qword ptr [ebx]
000F:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0012:    DD 01                   fld      qword ptr [ecx]
0014:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0017:    D9 E0                   fchs    ; -ST(0)
0019:    DE C1                   faddp    st(1)
001B:    DC 46 D0                fadd     qword ptr [esi - 0x30]
001E:    D9 E8                   fld1    ; Load 1.0
0020:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0022:    DC 02                   fadd     qword ptr [edx]
0024:    DD C1                   ffree    st(1)
0026:    D9 D0                   fnop
0028:    DD 46 E8                fld      qword ptr [esi - 0x18]
002B:    DC 23                   fsub     qword ptr [ebx]
002D:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0030:    D9 D0                   fnop
0032:    DD 46 E8                fld      qword ptr [esi - 0x18]
0035:    DC 0A                   fmul     qword ptr [edx]
0037:    DD 03                   fld      qword ptr [ebx]
0039:    DC 4E E0                fmul     qword ptr [esi - 0x20]
003C:    D9 E0                   fchs    ; -ST(0)
003E:    DE C1                   faddp    st(1)
0040:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0043:    DC 01                   fadd     qword ptr [ecx]
0045:    D9 D0                   fnop
0047:    DD 19                   fstp     qword ptr [ecx]
0049:    DD 1A                   fstp     qword ptr [edx]
004B:    DD 1B                   fstp     qword ptr [ebx]
004D:    8B C3                   mov      eax, ebx
004F:    5B                      pop      ebx
0050:    5E                      pop      esi
0051:    5D                      pop      ebp
0052:    C2 08 00                ret      8
