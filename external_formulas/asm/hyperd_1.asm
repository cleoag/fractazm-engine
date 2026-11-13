; Disassembly: hyperd_1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 257 bytes, decoded: 257 bytes (100.0%)
; DEoption: 4

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
0016:    8B 47 EC                mov      eax, dword ptr [edi - 0x14]
0019:    83 F8 11                cmp      eax, 0x11
001C:    7C 02                   jl       0x20    ; Conditional jump
001E:    EB 05                   jmp      0x25
0020:    83 F8 01                cmp      eax, 1
0023:    7D 05                   jge      0x2a    ; Conditional jump
0025:    B8 02 00 00 00          mov      eax, 2
002A:    90                      nop
002B:    DD 03                   fld      qword ptr [ebx]
002D:    DD 5D F8                fstp     qword ptr [ebp - 8]
0030:    90                      nop
0031:    DD 02                   fld      qword ptr [edx]
0033:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0036:    90                      nop
0037:    DD 01                   fld      qword ptr [ecx]
0039:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
003C:    90                      nop
003D:    DD 41 08                fld      qword ptr [ecx + 8]
0040:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0043:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0046:    90                      nop
0047:    48                      dec      eax
0048:    90                      nop
0049:    48                      dec      eax
004A:    DD 03                   fld      qword ptr [ebx]
004C:    DC 4D F8                fmul     qword ptr [ebp - 8]
004F:    DD 02                   fld      qword ptr [edx]
0051:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0054:    DE E9                   fsubp    st(1)
0056:    DD 01                   fld      qword ptr [ecx]
0058:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
005B:    DD 41 08                fld      qword ptr [ecx + 8]
005E:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0061:    DE C1                   faddp    st(1)
0063:    DE E9                   fsubp    st(1)
0065:    DD 01                   fld      qword ptr [ecx]
0067:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
006A:    DD 41 08                fld      qword ptr [ecx + 8]
006D:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0070:    DE C1                   faddp    st(1)
0072:    DC 4F E4                fmul     qword ptr [edi - 0x1c]
0075:    DE C1                   faddp    st(1)
0077:    90                      nop
0078:    DD 03                   fld      qword ptr [ebx]
007A:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
007D:    DD 02                   fld      qword ptr [edx]
007F:    DC 4D F8                fmul     qword ptr [ebp - 8]
0082:    DE C1                   faddp    st(1)
0084:    90                      nop
0085:    DD 03                   fld      qword ptr [ebx]
0087:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
008A:    DD 01                   fld      qword ptr [ecx]
008C:    DC 4D F8                fmul     qword ptr [ebp - 8]
008F:    DE C1                   faddp    st(1)
0091:    DD 02                   fld      qword ptr [edx]
0093:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0096:    DD 41 08                fld      qword ptr [ecx + 8]
0099:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
009C:    DE C1                   faddp    st(1)
009E:    DC 4F DC                fmul     qword ptr [edi - 0x24]
00A1:    DE C1                   faddp    st(1)
00A3:    90                      nop
00A4:    DD 03                   fld      qword ptr [ebx]
00A6:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
00A9:    DD 41 08                fld      qword ptr [ecx + 8]
00AC:    DC 4D F8                fmul     qword ptr [ebp - 8]
00AF:    DE C1                   faddp    st(1)
00B1:    DD 02                   fld      qword ptr [edx]
00B3:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
00B6:    DD 01                   fld      qword ptr [ecx]
00B8:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00BB:    DE C1                   faddp    st(1)
00BD:    DC 4F D4                fmul     qword ptr [edi - 0x2c]
00C0:    DE C1                   faddp    st(1)
00C2:    83 F8 00                cmp      eax, 0
00C5:    0F 8E 13 00 00 00       jle      0xde    ; Conditional jump
00CB:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00CE:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00D1:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
00D4:    DD 5D F8                fstp     qword ptr [ebp - 8]
00D7:    E9 6D FF FF FF          jmp      0x49
00DC:    D9 D0                   fnop
00DE:    DC 46 C8                fadd     qword ptr [esi - 0x38]
00E1:    DD 59 08                fstp     qword ptr [ecx + 8]
00E4:    DC 46 28                fadd     qword ptr [esi + 0x28]
00E7:    DD 19                   fstp     qword ptr [ecx]
00E9:    DC 46 20                fadd     qword ptr [esi + 0x20]
00EC:    DD 1A                   fstp     qword ptr [edx]
00EE:    DC 46 18                fadd     qword ptr [esi + 0x18]
00F1:    DD 1B                   fstp     qword ptr [ebx]
00F3:    90                      nop
00F4:    D9 D0                   fnop
00F6:    8B C3                   mov      eax, ebx
00F8:    5F                      pop      edi
00F9:    5E                      pop      esi
00FA:    5B                      pop      ebx
00FB:    89 EC                   mov      esp, ebp
00FD:    5D                      pop      ebp
00FE:    C2 08 00                ret      8
