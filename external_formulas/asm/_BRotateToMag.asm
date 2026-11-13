; Disassembly: _BRotateToMag.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 92 bytes, decoded: 92 bytes (100.0%)
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
0018:    DC 0F                   fmul     qword ptr [edi]
001A:    DD 01                   fld      qword ptr [ecx]
001C:    DC 4F 08                fmul     qword ptr [edi + 8]
001F:    D9 E0                   fchs    ; -ST(0)
0021:    DE C1                   faddp    st(1)
0023:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0026:    DD 5D F8                fstp     qword ptr [ebp - 8]
0029:    DD 02                   fld      qword ptr [edx]
002B:    DC 4F 10                fmul     qword ptr [edi + 0x10]
002E:    DD 1A                   fstp     qword ptr [edx]
0030:    90                      nop
0031:    DD 03                   fld      qword ptr [ebx]
0033:    DC 4F 08                fmul     qword ptr [edi + 8]
0036:    DD 01                   fld      qword ptr [ecx]
0038:    DC 0F                   fmul     qword ptr [edi]
003A:    DE C1                   faddp    st(1)
003C:    DD 19                   fstp     qword ptr [ecx]
003E:    90                      nop
003F:    DD 45 F8                fld      qword ptr [ebp - 8]
0042:    DC 22                   fsub     qword ptr [edx]
0044:    DD 1B                   fstp     qword ptr [ebx]
0046:    90                      nop
0047:    DD 45 F8                fld      qword ptr [ebp - 8]
004A:    DC 02                   fadd     qword ptr [edx]
004C:    DD 1A                   fstp     qword ptr [edx]
004E:    90                      nop
004F:    D9 D0                   fnop
0051:    8B C3                   mov      eax, ebx
0053:    5F                      pop      edi
0054:    5E                      pop      esi
0055:    5B                      pop      ebx
0056:    89 EC                   mov      esp, ebp
0058:    5D                      pop      ebp
0059:    C2 08 00                ret      8
