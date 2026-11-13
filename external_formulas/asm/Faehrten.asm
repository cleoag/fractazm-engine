; Disassembly: Faehrten.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 106 bytes, decoded: 106 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 00                   fld      qword ptr [eax]
000D:    DD 02                   fld      qword ptr [edx]
000F:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0012:    DD 01                   fld      qword ptr [ecx]
0014:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0017:    DE C1                   faddp    st(1)
0019:    DE C9                   fmulp    st(1)
001B:    DC 46 18                fadd     qword ptr [esi + 0x18]
001E:    90                      nop
001F:    DD 02                   fld      qword ptr [edx]
0021:    D8 C8                   fmul     st(0)
0023:    DD 00                   fld      qword ptr [eax]
0025:    D8 C8                   fmul     st(0)
0027:    DD 01                   fld      qword ptr [ecx]
0029:    D8 C8                   fmul     st(0)
002B:    DE C1                   faddp    st(1)
002D:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0030:    DE C1                   faddp    st(1)
0032:    DD 00                   fld      qword ptr [eax]
0034:    DC 09                   fmul     qword ptr [ecx]
0036:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0039:    DE C1                   faddp    st(1)
003B:    DC 46 20                fadd     qword ptr [esi + 0x20]
003E:    90                      nop
003F:    DD 01                   fld      qword ptr [ecx]
0041:    D8 C8                   fmul     st(0)
0043:    DD 00                   fld      qword ptr [eax]
0045:    D8 C8                   fmul     st(0)
0047:    DD 02                   fld      qword ptr [edx]
0049:    D8 C8                   fmul     st(0)
004B:    DE C1                   faddp    st(1)
004D:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0050:    DE C1                   faddp    st(1)
0052:    DD 00                   fld      qword ptr [eax]
0054:    DC 0A                   fmul     qword ptr [edx]
0056:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0059:    DE C1                   faddp    st(1)
005B:    DC 46 28                fadd     qword ptr [esi + 0x28]
005E:    DD 19                   fstp     qword ptr [ecx]
0060:    DD 1A                   fstp     qword ptr [edx]
0062:    DD 18                   fstp     qword ptr [eax]
0064:    5F                      pop      edi
0065:    5E                      pop      esi
0066:    5D                      pop      ebp
0067:    C2 08 00                ret      8
