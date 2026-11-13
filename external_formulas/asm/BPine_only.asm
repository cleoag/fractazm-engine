; Disassembly: BPine_only.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 138 bytes, decoded: 138 bytes (100.0%)

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
0018:    D8 C8                   fmul     st(0)
001A:    DD 5D F8                fstp     qword ptr [ebp - 8]
001D:    DD 02                   fld      qword ptr [edx]
001F:    D8 C8                   fmul     st(0)
0021:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0024:    DD 01                   fld      qword ptr [ecx]
0026:    D8 C8                   fmul     st(0)
0028:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
002B:    DD 03                   fld      qword ptr [ebx]
002D:    D8 C0                   fadd     st(0)
002F:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0032:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0035:    D9 FA                   fsqrt    ; sqrt(ST(0))
0037:    DE F9                   fdivp    st(1)
0039:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
003C:    90                      nop
003D:    DD 45 F8                fld      qword ptr [ebp - 8]
0040:    DC 65 F0                fsub     qword ptr [ebp - 0x10]
0043:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
0046:    90                      nop
0047:    DD 45 E0                fld      qword ptr [ebp - 0x20]
004A:    DD 45 F0                fld      qword ptr [ebp - 0x10]
004D:    DC 65 E8                fsub     qword ptr [ebp - 0x18]
0050:    DE C9                   fmulp    st(1)
0052:    90                      nop
0053:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0056:    D8 C0                   fadd     st(0)
0058:    DC 09                   fmul     qword ptr [ecx]
005A:    DC 0A                   fmul     qword ptr [edx]
005C:    90                      nop
005D:    DD 46 20                fld      qword ptr [esi + 0x20]
0060:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0063:    DE C1                   faddp    st(1)
0065:    DD 1A                   fstp     qword ptr [edx]
0067:    90                      nop
0068:    DD 46 28                fld      qword ptr [esi + 0x28]
006B:    DC 4F E8                fmul     qword ptr [edi - 0x18]
006E:    DE C1                   faddp    st(1)
0070:    DD 19                   fstp     qword ptr [ecx]
0072:    90                      nop
0073:    DD 46 18                fld      qword ptr [esi + 0x18]
0076:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0079:    DE C1                   faddp    st(1)
007B:    DD 1B                   fstp     qword ptr [ebx]
007D:    D9 D0                   fnop
007F:    8B C3                   mov      eax, ebx
0081:    5F                      pop      edi
0082:    5E                      pop      esi
0083:    5B                      pop      ebx
0084:    89 EC                   mov      esp, ebp
0086:    5D                      pop      ebp
0087:    C2 08 00                ret      8
