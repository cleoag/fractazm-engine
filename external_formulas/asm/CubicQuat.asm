; Disassembly: CubicQuat.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 132 bytes, decoded: 132 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 02                   fld      qword ptr [edx]
000D:    D8 C8                   fmul     st(0)
000F:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0012:    DD 01                   fld      qword ptr [ecx]
0014:    D8 C8                   fmul     st(0)
0016:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0019:    DD 41 08                fld      qword ptr [ecx + 8]
001C:    D8 C8                   fmul     st(0)
001E:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0021:    DE C1                   faddp    st(1)
0023:    DE C1                   faddp    st(1)
0025:    D9 E0                   fchs    ; -ST(0)
0027:    DD 00                   fld      qword ptr [eax]
0029:    D8 C8                   fmul     st(0)
002B:    DC 4F F0                fmul     qword ptr [edi - 0x10]
002E:    DE C1                   faddp    st(1)
0030:    DC 46 18                fadd     qword ptr [esi + 0x18]
0033:    DD 46 20                fld      qword ptr [esi + 0x20]
0036:    DC 4F C8                fmul     qword ptr [edi - 0x38]
0039:    DE C1                   faddp    st(1)
003B:    D9 D0                   fnop
003D:    DD 00                   fld      qword ptr [eax]
003F:    DC 0A                   fmul     qword ptr [edx]
0041:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0044:    D8 C0                   fadd     st(0)
0046:    DC 46 20                fadd     qword ptr [esi + 0x20]
0049:    DD 46 18                fld      qword ptr [esi + 0x18]
004C:    DC 4F C0                fmul     qword ptr [edi - 0x40]
004F:    DE C1                   faddp    st(1)
0051:    D9 D0                   fnop
0053:    DD 00                   fld      qword ptr [eax]
0055:    DC 09                   fmul     qword ptr [ecx]
0057:    DC 4F E0                fmul     qword ptr [edi - 0x20]
005A:    D8 C0                   fadd     st(0)
005C:    DC 46 28                fadd     qword ptr [esi + 0x28]
005F:    D9 D0                   fnop
0061:    DD 00                   fld      qword ptr [eax]
0063:    DC 49 08                fmul     qword ptr [ecx + 8]
0066:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0069:    D8 C0                   fadd     st(0)
006B:    DC 47 D0                fadd     qword ptr [edi - 0x30]
006E:    DC 46 C8                fadd     qword ptr [esi - 0x38]
0071:    D9 D0                   fnop
0073:    DD 59 08                fstp     qword ptr [ecx + 8]
0076:    DD 19                   fstp     qword ptr [ecx]
0078:    DD 1A                   fstp     qword ptr [edx]
007A:    DD 18                   fstp     qword ptr [eax]
007C:    D9 D0                   fnop
007E:    5F                      pop      edi
007F:    5E                      pop      esi
0080:    5D                      pop      ebp
0081:    C2 08 00                ret      8
