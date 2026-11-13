; Disassembly: GeneralQuat.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 188 bytes, decoded: 188 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    83 EC 20                sub      esp, 0x20
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 41 08                fld      qword ptr [ecx + 8]
0011:    DD 01                   fld      qword ptr [ecx]
0013:    DD 00                   fld      qword ptr [eax]
0015:    DD 02                   fld      qword ptr [edx]
0017:    D9 C1                   fld      st(1)
0019:    D8 C8                   fmul     st(0)
001B:    D9 C1                   fld      st(1)
001D:    D8 C8                   fmul     st(0)
001F:    DE E9                   fsubp    st(1)
0021:    DD 1C 24                fstp     qword ptr [esp]
0024:    D9 C1                   fld      st(1)
0026:    D8 C9                   fmul     st(1)
0028:    D8 C0                   fadd     st(0)
002A:    DD 5C 24 08             fstp     qword ptr [esp + 8]
002E:    D9 C1                   fld      st(1)
0030:    D8 CB                   fmul     st(3)
0032:    D8 C0                   fadd     st(0)
0034:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0038:    D9 C1                   fld      st(1)
003A:    D8 CC                   fmul     st(4)
003C:    D8 C0                   fadd     st(0)
003E:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0042:    83 EF 0C                sub      edi, 0xc
0045:    D9 C3                   fld      st(3)
0047:    DC CC                   fmul     st(4), st(0)
0049:    DC CB                   fmul     st(3), st(0)
004B:    D9 C3                   fld      st(3)
004D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0051:    D8 CA                   fmul     st(2)
0053:    D9 C0                   fld      st(0)
0055:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0057:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0059:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
005B:    DD D8                   fstp     st(0)
005D:    DD 01                   fld      qword ptr [ecx]
005F:    D9 C0                   fld      st(0)
0061:    D8 C8                   fmul     st(0)
0063:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0065:    DE C9                   fmulp    st(1)
0067:    D9 C0                   fld      st(0)
0069:    B9 07 00 00 00          mov      ecx, 7
006E:    8B 07                   mov      eax, dword ptr [edi]
0070:    48                      dec      eax
0071:    79 07                   jns      0x7a    ; Conditional jump
0073:    F7 D8                   neg      eax
0075:    83 E8 02                sub      eax, 2
0078:    D9 E0                   fchs    ; -ST(0)
007A:    83 E0 03                and      eax, 3
007D:    DC 04 C4                fadd     qword ptr [esp + eax*8]
0080:    DD 1C C4                fstp     qword ptr [esp + eax*8]
0083:    83 EF 04                sub      edi, 4
0086:    49                      dec      ecx
0087:    79 E5                   jns      0x6e    ; Conditional jump
0089:    8D 42 F8                lea      eax, [edx - 8]
008C:    8D 4A 08                lea      ecx, [edx + 8]
008F:    DD 44 24 18             fld      qword ptr [esp + 0x18]
0093:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0097:    DD 44 24 08             fld      qword ptr [esp + 8]
009B:    DD 04 24                fld      qword ptr [esp]
009E:    DC 46 18                fadd     qword ptr [esi + 0x18]
00A1:    DD 18                   fstp     qword ptr [eax]
00A3:    DC 46 20                fadd     qword ptr [esi + 0x20]
00A6:    DD 1A                   fstp     qword ptr [edx]
00A8:    DC 46 28                fadd     qword ptr [esi + 0x28]
00AB:    DD 19                   fstp     qword ptr [ecx]
00AD:    DC 46 C8                fadd     qword ptr [esi - 0x38]
00B0:    DD 59 08                fstp     qword ptr [ecx + 8]
00B3:    83 C4 20                add      esp, 0x20
00B6:    5F                      pop      edi
00B7:    5E                      pop      esi
00B8:    5D                      pop      ebp
00B9:    C2 08 00                ret      8
