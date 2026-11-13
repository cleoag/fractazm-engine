; Disassembly: _BRotateFromMag.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 107 bytes, decoded: 107 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 30 00 00 00       sub      esp, 0x30
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 03                   fld      qword ptr [ebx]
0018:    DC 4F 10                fmul     qword ptr [edi + 0x10]
001B:    DD 5D F8                fstp     qword ptr [ebp - 8]
001E:    90                      nop
001F:    DD 02                   fld      qword ptr [edx]
0021:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0024:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0027:    90                      nop
0028:    DD 45 F8                fld      qword ptr [ebp - 8]
002B:    DD 45 F0                fld      qword ptr [ebp - 0x10]
002E:    DE C1                   faddp    st(1)
0030:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0033:    90                      nop
0034:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0037:    DC 65 F8                fsub     qword ptr [ebp - 8]
003A:    DD 1A                   fstp     qword ptr [edx]
003C:    90                      nop
003D:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0040:    DC 0F                   fmul     qword ptr [edi]
0042:    DD 01                   fld      qword ptr [ecx]
0044:    DC 4F 08                fmul     qword ptr [edi + 8]
0047:    DE C1                   faddp    st(1)
0049:    DD 1B                   fstp     qword ptr [ebx]
004B:    90                      nop
004C:    DD 01                   fld      qword ptr [ecx]
004E:    DC 0F                   fmul     qword ptr [edi]
0050:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0053:    DC 4F 08                fmul     qword ptr [edi + 8]
0056:    D9 E0                   fchs    ; -ST(0)
0058:    90                      nop
0059:    DE C1                   faddp    st(1)
005B:    DD 19                   fstp     qword ptr [ecx]
005D:    90                      nop
005E:    D9 D0                   fnop
0060:    8B C3                   mov      eax, ebx
0062:    5F                      pop      edi
0063:    5E                      pop      esi
0064:    5B                      pop      ebx
0065:    89 EC                   mov      esp, ebp
0067:    5D                      pop      ebp
0068:    C2 08 00                ret      8
