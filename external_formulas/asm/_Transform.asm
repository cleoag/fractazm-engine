; Disassembly: _Transform.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 154 bytes, decoded: 154 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 01                   fld      qword ptr [ecx]
000D:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0010:    DD 02                   fld      qword ptr [edx]
0012:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0015:    DD 00                   fld      qword ptr [eax]
0017:    DC 47 E8                fadd     qword ptr [edi - 0x18]
001A:    D9 C0                   fld      st(0)
001C:    DC 4F A0                fmul     qword ptr [edi - 0x60]
001F:    D9 C2                   fld      st(2)
0021:    DC 4F 98                fmul     qword ptr [edi - 0x68]
0024:    DE C1                   faddp    st(1)
0026:    D9 C3                   fld      st(3)
0028:    DC 4F 90                fmul     qword ptr [edi - 0x70]
002B:    DE C1                   faddp    st(1)
002D:    D9 C1                   fld      st(1)
002F:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0032:    D9 C3                   fld      st(3)
0034:    DC 4F C8                fmul     qword ptr [edi - 0x38]
0037:    DE C1                   faddp    st(1)
0039:    D9 C4                   fld      st(4)
003B:    DC 4F C0                fmul     qword ptr [edi - 0x40]
003E:    DE C1                   faddp    st(1)
0040:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0042:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0045:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0047:    DC 4F B0                fmul     qword ptr [edi - 0x50]
004A:    DE C3                   faddp    st(3)
004C:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004E:    DC 4F A8                fmul     qword ptr [edi - 0x58]
0051:    DE C2                   faddp    st(2)
0053:    DD 41 08                fld      qword ptr [ecx + 8]
0056:    DD 86 E8 00 00 00       fld      qword ptr [esi + 0xe8]
005C:    DD 47 F0                fld      qword ptr [edi - 0x10]
005F:    DC CD                   fmul     st(5), st(0)
0061:    DC CC                   fmul     st(4), st(0)
0063:    DC CB                   fmul     st(3), st(0)
0065:    DC CA                   fmul     st(2), st(0)
0067:    DE C9                   fmulp    st(1)
0069:    DD 9E E8 00 00 00       fstp     qword ptr [esi + 0xe8]
006F:    DD 59 08                fstp     qword ptr [ecx + 8]
0072:    83 BE 98 00 00 00 00    cmp      dword ptr [esi + 0x98], 0
0079:    75 13                   jne      0x8e    ; Conditional jump
007B:    83 7F 8C 00             cmp      dword ptr [edi - 0x74], 0
007F:    74 0D                   je       0x8e    ; Conditional jump
0081:    DD 56 18                fst      qword ptr [esi + 0x18]
0084:    D9 C1                   fld      st(1)
0086:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0089:    D9 C2                   fld      st(2)
008B:    DD 5E 28                fstp     qword ptr [esi + 0x28]
008E:    DD 18                   fstp     qword ptr [eax]
0090:    DD 1A                   fstp     qword ptr [edx]
0092:    DD 19                   fstp     qword ptr [ecx]
0094:    5F                      pop      edi
0095:    5E                      pop      esi
0096:    5D                      pop      ebp
0097:    C2 08 00                ret      8
