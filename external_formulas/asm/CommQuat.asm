; Disassembly: CommQuat.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 118 bytes, decoded: 118 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 41 08                fld      qword ptr [ecx + 8]
000E:    DD 01                   fld      qword ptr [ecx]
0010:    DD 02                   fld      qword ptr [edx]
0012:    DD 00                   fld      qword ptr [eax]
0014:    D9 C1                   fld      st(1)
0016:    D8 C8                   fmul     st(0)
0018:    D9 C1                   fld      st(1)
001A:    D8 C8                   fmul     st(0)
001C:    DE E1                   fsubrp   st(1)
001E:    D9 C4                   fld      st(4)
0020:    D8 CC                   fmul     st(4)
0022:    D8 C0                   fadd     st(0)
0024:    DE E9                   fsubp    st(1)
0026:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0029:    DC 46 18                fadd     qword ptr [esi + 0x18]
002C:    DD 18                   fstp     qword ptr [eax]
002E:    D9 C0                   fld      st(0)
0030:    D8 CA                   fmul     st(2)
0032:    D8 C0                   fadd     st(0)
0034:    D9 C3                   fld      st(3)
0036:    D8 C8                   fmul     st(0)
0038:    DE C1                   faddp    st(1)
003A:    D9 C4                   fld      st(4)
003C:    D8 C8                   fmul     st(0)
003E:    DE E9                   fsubp    st(1)
0040:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0043:    DC 46 20                fadd     qword ptr [esi + 0x20]
0046:    DD 1A                   fstp     qword ptr [edx]
0048:    D9 C0                   fld      st(0)
004A:    D8 CB                   fmul     st(3)
004C:    D9 C2                   fld      st(2)
004E:    D8 CD                   fmul     st(5)
0050:    DE E9                   fsubp    st(1)
0052:    D8 C0                   fadd     st(0)
0054:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0057:    DC 46 28                fadd     qword ptr [esi + 0x28]
005A:    DD 19                   fstp     qword ptr [ecx]
005C:    DE CB                   fmulp    st(3)
005E:    DE C9                   fmulp    st(1)
0060:    DE C1                   faddp    st(1)
0062:    D8 C0                   fadd     st(0)
0064:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0067:    DC 47 D0                fadd     qword ptr [edi - 0x30]
006A:    DC 46 C8                fadd     qword ptr [esi - 0x38]
006D:    DD 59 08                fstp     qword ptr [ecx + 8]
0070:    5F                      pop      edi
0071:    5E                      pop      esi
0072:    5D                      pop      ebp
0073:    C2 08 00                ret      8
