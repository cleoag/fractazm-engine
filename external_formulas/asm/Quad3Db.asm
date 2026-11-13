; Disassembly: Quad3Db.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 376 bytes, decoded: 376 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    50                      push     eax
0005:    56                      push     esi
0006:    57                      push     edi
0007:    90                      nop
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B D8                   mov      ebx, eax
000D:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0010:    D9 D0                   fnop
0012:    D9 D0                   fnop
0014:    DD 03                   fld      qword ptr [ebx]
0016:    D8 C8                   fmul     st(0)
0018:    DD 02                   fld      qword ptr [edx]
001A:    D8 C8                   fmul     st(0)
001C:    DD 47 D8                fld      qword ptr [edi - 0x28]
001F:    DE C1                   faddp    st(1)
0021:    DE C1                   faddp    st(1)
0023:    D9 EE                   fldz    ; Load 0.0
0025:    D8 D9                   fcomp    st(1)
0027:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0029:    9E                      sahf
002A:    0F 87 35 01 00 00       ja       0x165    ; Conditional jump
0030:    0F 84 37 01 00 00       je       0x16d    ; Conditional jump
0036:    90                      nop
0037:    90                      nop
0038:    DD 03                   fld      qword ptr [ebx]
003A:    D8 C8                   fmul     st(0)
003C:    DD 02                   fld      qword ptr [edx]
003E:    D8 C8                   fmul     st(0)
0040:    D9 E0                   fchs    ; -ST(0)
0042:    DE C1                   faddp    st(1)
0044:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0047:    DD 47 D0                fld      qword ptr [edi - 0x30]
004A:    DD 01                   fld      qword ptr [ecx]
004C:    D8 C8                   fmul     st(0)
004E:    D8 F3                   fdiv     st(3)
0050:    D9 E0                   fchs    ; -ST(0)
0052:    DE C1                   faddp    st(1)
0054:    DE C9                   fmulp    st(1)
0056:    8B 47 CC                mov      eax, dword ptr [edi - 0x34]
0059:    66 83 E0 03             and      ax, 3
005D:    66 83 F8 00             cmp      ax, 0
0061:    75 07                   jne      0x6a    ; Conditional jump
0063:    D9 E1                   fabs    ; |ST(0)|
0065:    DC 46 18                fadd     qword ptr [esi + 0x18]
0068:    EB 32                   jmp      0x9c
006A:    66 83 F8 01             cmp      ax, 1
006E:    75 07                   jne      0x77    ; Conditional jump
0070:    DC 46 18                fadd     qword ptr [esi + 0x18]
0073:    D9 E1                   fabs    ; |ST(0)|
0075:    EB 25                   jmp      0x9c
0077:    66 83 F8 02             cmp      ax, 2
007B:    75 1A                   jne      0x97    ; Conditional jump
007D:    D9 C0                   fld      st(0)
007F:    DC 46 18                fadd     qword ptr [esi + 0x18]
0082:    D9 E1                   fabs    ; |ST(0)|
0084:    D9 C1                   fld      st(1)
0086:    DC 66 18                fsub     qword ptr [esi + 0x18]
0089:    D9 E1                   fabs    ; |ST(0)|
008B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
008D:    DE E1                   fsubrp   st(1)
008F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0091:    D9 E0                   fchs    ; -ST(0)
0093:    DE C1                   faddp    st(1)
0095:    EB 05                   jmp      0x9c
0097:    DC 46 18                fadd     qword ptr [esi + 0x18]
009A:    D9 D0                   fnop
009C:    DD 03                   fld      qword ptr [ebx]
009E:    DC 0A                   fmul     qword ptr [edx]
00A0:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00A3:    DD 47 D0                fld      qword ptr [edi - 0x30]
00A6:    DD 01                   fld      qword ptr [ecx]
00A8:    D8 C8                   fmul     st(0)
00AA:    D8 F4                   fdiv     st(4)
00AC:    D9 E0                   fchs    ; -ST(0)
00AE:    DE C1                   faddp    st(1)
00B0:    DE C9                   fmulp    st(1)
00B2:    8B 47 CC                mov      eax, dword ptr [edi - 0x34]
00B5:    66 C1 E8 02             shr      ax, 2
00B9:    66 83 E0 03             and      ax, 3
00BD:    66 83 F8 00             cmp      ax, 0
00C1:    75 07                   jne      0xca    ; Conditional jump
00C3:    D9 E1                   fabs    ; |ST(0)|
00C5:    DC 46 20                fadd     qword ptr [esi + 0x20]
00C8:    EB 32                   jmp      0xfc
00CA:    66 83 F8 01             cmp      ax, 1
00CE:    75 07                   jne      0xd7    ; Conditional jump
00D0:    DC 46 20                fadd     qword ptr [esi + 0x20]
00D3:    D9 E1                   fabs    ; |ST(0)|
00D5:    EB 25                   jmp      0xfc
00D7:    66 83 F8 02             cmp      ax, 2
00DB:    75 1A                   jne      0xf7    ; Conditional jump
00DD:    D9 C0                   fld      st(0)
00DF:    DC 46 20                fadd     qword ptr [esi + 0x20]
00E2:    D9 E1                   fabs    ; |ST(0)|
00E4:    D9 C1                   fld      st(1)
00E6:    DC 66 20                fsub     qword ptr [esi + 0x20]
00E9:    D9 E1                   fabs    ; |ST(0)|
00EB:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00ED:    DE E1                   fsubrp   st(1)
00EF:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00F1:    D9 E0                   fchs    ; -ST(0)
00F3:    DE C1                   faddp    st(1)
00F5:    EB 05                   jmp      0xfc
00F7:    DC 46 20                fadd     qword ptr [esi + 0x20]
00FA:    D9 D0                   fnop
00FC:    DD 01                   fld      qword ptr [ecx]
00FE:    D9 C3                   fld      st(3)
0100:    D9 FA                   fsqrt    ; sqrt(ST(0))
0102:    DE C9                   fmulp    st(1)
0104:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0107:    8B 47 CC                mov      eax, dword ptr [edi - 0x34]
010A:    66 C1 E8 04             shr      ax, 4
010E:    66 83 E0 03             and      ax, 3
0112:    66 83 F8 00             cmp      ax, 0
0116:    75 07                   jne      0x11f    ; Conditional jump
0118:    D9 E1                   fabs    ; |ST(0)|
011A:    DC 46 28                fadd     qword ptr [esi + 0x28]
011D:    EB 32                   jmp      0x151
011F:    66 83 F8 01             cmp      ax, 1
0123:    75 07                   jne      0x12c    ; Conditional jump
0125:    DC 46 28                fadd     qword ptr [esi + 0x28]
0128:    D9 E1                   fabs    ; |ST(0)|
012A:    EB 25                   jmp      0x151
012C:    66 83 F8 02             cmp      ax, 2
0130:    75 1A                   jne      0x14c    ; Conditional jump
0132:    D9 C0                   fld      st(0)
0134:    DC 46 28                fadd     qword ptr [esi + 0x28]
0137:    D9 E1                   fabs    ; |ST(0)|
0139:    D9 C1                   fld      st(1)
013B:    DC 66 28                fsub     qword ptr [esi + 0x28]
013E:    D9 E1                   fabs    ; |ST(0)|
0140:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0142:    DE E1                   fsubrp   st(1)
0144:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0146:    D9 E0                   fchs    ; -ST(0)
0148:    DE C1                   faddp    st(1)
014A:    EB 05                   jmp      0x151
014C:    DC 46 28                fadd     qword ptr [esi + 0x28]
014F:    D9 D0                   fnop
0151:    DD 19                   fstp     qword ptr [ecx]
0153:    DD 1A                   fstp     qword ptr [edx]
0155:    DD 1B                   fstp     qword ptr [ebx]
0157:    DD D8                   fstp     st(0)
0159:    D9 D0                   fnop
015B:    8B C3                   mov      eax, ebx
015D:    5F                      pop      edi
015E:    5E                      pop      esi
015F:    58                      pop      eax
0160:    5B                      pop      ebx
0161:    5D                      pop      ebp
0162:    C2 08 00                ret      8
0165:    D9 EE                   fldz    ; Load 0.0
0167:    D9 EE                   fldz    ; Load 0.0
0169:    D9 EE                   fldz    ; Load 0.0
016B:    EB E2                   jmp      0x14f
016D:    DD 46 18                fld      qword ptr [esi + 0x18]
0170:    DD 46 20                fld      qword ptr [esi + 0x20]
0173:    DD 46 28                fld      qword ptr [esi + 0x28]
0176:    EB D7                   jmp      0x14f
