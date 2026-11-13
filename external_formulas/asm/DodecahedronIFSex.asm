; Disassembly: DodecahedronIFSex.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 530 bytes, decoded: 530 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 40 00 00 00       sub      esp, 0x40
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    E9 8C 00 00 00          jmp      0xa7
001B:    90                      nop
001C:    DD 03                   fld      qword ptr [ebx]
001E:    DC 4D F8                fmul     qword ptr [ebp - 8]
0021:    DD 02                   fld      qword ptr [edx]
0023:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0026:    DD 01                   fld      qword ptr [ecx]
0028:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
002B:    DE C1                   faddp    st(1)
002D:    DE C1                   faddp    st(1)
002F:    D9 E4                   ftst
0031:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0033:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0035:    76 04                   jbe      0x3b    ; Conditional jump
0037:    DD D8                   fstp     st(0)
0039:    C3                      ret
003A:    90                      nop
003B:    D8 C0                   fadd     st(0)
003D:    D9 E0                   fchs    ; -ST(0)
003F:    D9 C0                   fld      st(0)
0041:    DC 4D F8                fmul     qword ptr [ebp - 8]
0044:    DC 03                   fadd     qword ptr [ebx]
0046:    DD 1B                   fstp     qword ptr [ebx]
0048:    D9 C0                   fld      st(0)
004A:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
004D:    DC 02                   fadd     qword ptr [edx]
004F:    DD 1A                   fstp     qword ptr [edx]
0051:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0054:    DC 01                   fadd     qword ptr [ecx]
0056:    DD 19                   fstp     qword ptr [ecx]
0058:    C3                      ret
0059:    90                      nop
005A:    D9 E8                   fld1    ; Load 1.0
005C:    DC 67 88                fsub     qword ptr [edi - 0x78]
005F:    D9 E0                   fchs    ; -ST(0)
0061:    DD 55 F8                fst      qword ptr [ebp - 8]
0064:    D9 E8                   fld1    ; Load 1.0
0066:    DE F1                   fdivrp   st(1)
0068:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
006B:    D9 E8                   fld1    ; Load 1.0
006D:    D9 E0                   fchs    ; -ST(0)
006F:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0072:    DD 45 F8                fld      qword ptr [ebp - 8]
0075:    D8 C8                   fmul     st(0)
0077:    DD 45 F0                fld      qword ptr [ebp - 0x10]
007A:    D8 C8                   fmul     st(0)
007C:    DD 45 E8                fld      qword ptr [ebp - 0x18]
007F:    D8 C8                   fmul     st(0)
0081:    DE C1                   faddp    st(1)
0083:    DE C1                   faddp    st(1)
0085:    D9 FA                   fsqrt    ; sqrt(ST(0))
0087:    D9 E8                   fld1    ; Load 1.0
0089:    DE F1                   fdivrp   st(1)
008B:    DD 45 F8                fld      qword ptr [ebp - 8]
008E:    D8 C9                   fmul     st(1)
0090:    DD 5D F8                fstp     qword ptr [ebp - 8]
0093:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0096:    D8 C9                   fmul     st(1)
0098:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
009B:    DD 45 E8                fld      qword ptr [ebp - 0x18]
009E:    D8 C9                   fmul     st(1)
00A0:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00A3:    DD D8                   fstp     st(0)
00A5:    C3                      ret
00A6:    90                      nop
00A7:    DD 01                   fld      qword ptr [ecx]
00A9:    DD 02                   fld      qword ptr [edx]
00AB:    DD 03                   fld      qword ptr [ebx]
00AD:    D9 C0                   fld      st(0)
00AF:    D8 4F BC                fmul     dword ptr [edi - 0x44]
00B2:    D9 C2                   fld      st(2)
00B4:    D8 4F B8                fmul     dword ptr [edi - 0x48]
00B7:    DE C1                   faddp    st(1)
00B9:    D9 C3                   fld      st(3)
00BB:    D8 4F B4                fmul     dword ptr [edi - 0x4c]
00BE:    DE C1                   faddp    st(1)
00C0:    90                      nop
00C1:    90                      nop
00C2:    D9 C1                   fld      st(1)
00C4:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00C7:    D9 C3                   fld      st(3)
00C9:    D8 4F D0                fmul     dword ptr [edi - 0x30]
00CC:    DE C1                   faddp    st(1)
00CE:    D9 C4                   fld      st(4)
00D0:    D8 4F CC                fmul     dword ptr [edi - 0x34]
00D3:    DE C1                   faddp    st(1)
00D5:    90                      nop
00D6:    90                      nop
00D7:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00D9:    D8 4F C8                fmul     dword ptr [edi - 0x38]
00DC:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00DE:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
00E1:    DE C3                   faddp    st(3)
00E3:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E5:    D8 4F C0                fmul     dword ptr [edi - 0x40]
00E8:    DE C2                   faddp    st(2)
00EA:    DD 1B                   fstp     qword ptr [ebx]
00EC:    DD 1A                   fstp     qword ptr [edx]
00EE:    DD 19                   fstp     qword ptr [ecx]
00F0:    E8 65 FF FF FF          call     0x5a
00F5:    E8 22 FF FF FF          call     0x1c
00FA:    83 7F 84 00             cmp      dword ptr [edi - 0x7c], 0
00FE:    75 05                   jne      0x105    ; Conditional jump
0100:    E8 1A 00 00 00          call     0x11f
0105:    EB 5F                   jmp      0x166
0107:    DD 45 F8                fld      qword ptr [ebp - 8]
010A:    DD 45 F0                fld      qword ptr [ebp - 0x10]
010D:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0110:    DD 5D F8                fstp     qword ptr [ebp - 8]
0113:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0116:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0119:    E8 FE FE FF FF          call     0x1c
011E:    C3                      ret
011F:    DD 01                   fld      qword ptr [ecx]
0121:    DD 02                   fld      qword ptr [edx]
0123:    DD 03                   fld      qword ptr [ebx]
0125:    D9 C1                   fld      st(1)
0127:    D8 4F 98                fmul     dword ptr [edi - 0x68]
012A:    D9 C1                   fld      st(1)
012C:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
012F:    DE C1                   faddp    st(1)
0131:    D9 C3                   fld      st(3)
0133:    D8 4F 90                fmul     dword ptr [edi - 0x70]
0136:    DE C1                   faddp    st(1)
0138:    D9 C2                   fld      st(2)
013A:    D8 4F B0                fmul     dword ptr [edi - 0x50]
013D:    D9 C2                   fld      st(2)
013F:    D8 4F AC                fmul     dword ptr [edi - 0x54]
0142:    DE C1                   faddp    st(1)
0144:    D9 C4                   fld      st(4)
0146:    D8 4F A8                fmul     dword ptr [edi - 0x58]
0149:    DE C1                   faddp    st(1)
014B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
014D:    D8 4F A4                fmul     dword ptr [edi - 0x5c]
0150:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0152:    D8 4F A0                fmul     dword ptr [edi - 0x60]
0155:    DE C2                   faddp    st(2)
0157:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0159:    D8 4F 9C                fmul     dword ptr [edi - 0x64]
015C:    DE C1                   faddp    st(1)
015E:    DD 1B                   fstp     qword ptr [ebx]
0160:    DD 1A                   fstp     qword ptr [edx]
0162:    DD 19                   fstp     qword ptr [ecx]
0164:    C3                      ret
0165:    90                      nop
0166:    E8 9C FF FF FF          call     0x107
016B:    83 7F 84 01             cmp      dword ptr [edi - 0x7c], 1
016F:    75 05                   jne      0x176    ; Conditional jump
0171:    E8 A9 FF FF FF          call     0x11f
0176:    E8 8C FF FF FF          call     0x107
017B:    83 7F 84 02             cmp      dword ptr [edi - 0x7c], 2
017F:    75 05                   jne      0x186    ; Conditional jump
0181:    E8 99 FF FF FF          call     0x11f
0186:    E8 7C FF FF FF          call     0x107
018B:    83 7F 84 03             cmp      dword ptr [edi - 0x7c], 3
018F:    75 05                   jne      0x196    ; Conditional jump
0191:    E8 89 FF FF FF          call     0x11f
0196:    E8 6C FF FF FF          call     0x107
019B:    83 7F 84 04             cmp      dword ptr [edi - 0x7c], 4
019F:    75 05                   jne      0x1a6    ; Conditional jump
01A1:    E8 79 FF FF FF          call     0x11f
01A6:    E8 5C FF FF FF          call     0x107
01AB:    83 7F 84 05             cmp      dword ptr [edi - 0x7c], 5
01AF:    75 05                   jne      0x1b6    ; Conditional jump
01B1:    E8 69 FF FF FF          call     0x11f
01B6:    E8 4C FF FF FF          call     0x107
01BB:    83 7F 84 06             cmp      dword ptr [edi - 0x7c], 6
01BF:    75 05                   jne      0x1c6    ; Conditional jump
01C1:    E8 59 FF FF FF          call     0x11f
01C6:    DD 01                   fld      qword ptr [ecx]
01C8:    DD 02                   fld      qword ptr [edx]
01CA:    DD 03                   fld      qword ptr [ebx]
01CC:    DD 47 F0                fld      qword ptr [edi - 0x10]
01CF:    DD 41 08                fld      qword ptr [ecx + 8]
01D2:    D8 C9                   fmul     st(1)
01D4:    DD 59 08                fstp     qword ptr [ecx + 8]
01D7:    DC CB                   fmul     st(3), st(0)
01D9:    DC CA                   fmul     st(2), st(0)
01DB:    DC C9                   fmul     st(1), st(0)
01DD:    D9 E8                   fld1    ; Load 1.0
01DF:    DE E9                   fsubp    st(1)
01E1:    D9 C0                   fld      st(0)
01E3:    D9 C0                   fld      st(0)
01E5:    DC 4F E8                fmul     qword ptr [edi - 0x18]
01E8:    DE EC                   fsubp    st(4)
01EA:    DC 4F E0                fmul     qword ptr [edi - 0x20]
01ED:    DE EA                   fsubp    st(2)
01EF:    DC 4F D8                fmul     qword ptr [edi - 0x28]
01F2:    DE EB                   fsubp    st(3)
01F4:    DD 1B                   fstp     qword ptr [ebx]
01F6:    DD 1A                   fstp     qword ptr [edx]
01F8:    DD 19                   fstp     qword ptr [ecx]
01FA:    83 7F 84 07             cmp      dword ptr [edi - 0x7c], 7
01FE:    75 05                   jne      0x205    ; Conditional jump
0200:    E8 1A FF FF FF          call     0x11f
0205:    D9 D0                   fnop
0207:    8B C3                   mov      eax, ebx
0209:    5F                      pop      edi
020A:    5E                      pop      esi
020B:    5B                      pop      ebx
020C:    8B E5                   mov      esp, ebp
020E:    5D                      pop      ebp
020F:    C2 08 00                ret      8
