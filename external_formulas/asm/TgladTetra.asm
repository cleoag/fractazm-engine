; Disassembly: TgladTetra.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 503 bytes, decoded: 503 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 60 00 00 00       sub      esp, 0x60
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    8A 67 F4                mov      ah, byte ptr [edi - 0xc]
0019:    80 E4 07                and      ah, 7
001C:    DD 03                   fld      qword ptr [ebx]
001E:    D8 C8                   fmul     st(0)
0020:    DD 02                   fld      qword ptr [edx]
0022:    D8 C8                   fmul     st(0)
0024:    DE C1                   faddp    st(1)
0026:    D9 C0                   fld      st(0)
0028:    DD 01                   fld      qword ptr [ecx]
002A:    D8 C8                   fmul     st(0)
002C:    DE C1                   faddp    st(1)
002E:    C1 E0 10                shl      eax, 0x10
0031:    DC 57 30                fcom     qword ptr [edi + 0x30]
0034:    9B                      wait
0035:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0037:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0039:    0F 83 AB 01 00 00       jae      0x1ea    ; Conditional jump
003F:    C1 E8 10                shr      eax, 0x10
0042:    D9 FA                   fsqrt    ; sqrt(ST(0))
0044:    DD 55 B8                fst      qword ptr [ebp - 0x48]
0047:    DD 01                   fld      qword ptr [ecx]
0049:    DE F1                   fdivrp   st(1)
004B:    DD 5D F8                fstp     qword ptr [ebp - 8]
004E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0050:    DC 22                   fsub     qword ptr [edx]
0052:    DC 33                   fdiv     qword ptr [ebx]
0054:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0057:    D8 C8                   fmul     st(0)
0059:    DD 55 D8                fst      qword ptr [ebp - 0x28]
005C:    D9 E8                   fld1    ; Load 1.0
005E:    DE C1                   faddp    st(1)
0060:    D9 E8                   fld1    ; Load 1.0
0062:    DE F1                   fdivrp   st(1)
0064:    90                      nop
0065:    D9 E8                   fld1    ; Load 1.0
0067:    DC 45 F8                fadd     qword ptr [ebp - 8]
006A:    D9 E8                   fld1    ; Load 1.0
006C:    DC 65 F8                fsub     qword ptr [ebp - 8]
006F:    DE F1                   fdivrp   st(1)
0071:    D9 FA                   fsqrt    ; sqrt(ST(0))
0073:    90                      nop
0074:    DE C9                   fmulp    st(1)
0076:    D9 C0                   fld      st(0)
0078:    D8 C0                   fadd     st(0)
007A:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
007D:    DD 1B                   fstp     qword ptr [ebx]
007F:    90                      nop
0080:    D9 E8                   fld1    ; Load 1.0
0082:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
0085:    DE C9                   fmulp    st(1)
0087:    DD 1A                   fstp     qword ptr [edx]
0089:    D9 D0                   fnop
008B:    8A 47 F0                mov      al, byte ptr [edi - 0x10]
008E:    24 07                   and      al, 7
0090:    90                      nop
0091:    DD 03                   fld      qword ptr [ebx]
0093:    D9 C0                   fld      st(0)
0095:    D8 C8                   fmul     st(0)
0097:    DE C9                   fmulp    st(1)
0099:    DD 02                   fld      qword ptr [edx]
009B:    D8 C8                   fmul     st(0)
009D:    DC 4F 08                fmul     qword ptr [edi + 8]
00A0:    DC 0B                   fmul     qword ptr [ebx]
00A2:    DE C1                   faddp    st(1)
00A4:    C1 E0 10                shl      eax, 0x10
00A7:    DC 57 30                fcom     qword ptr [edi + 0x30]
00AA:    9B                      wait
00AB:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AD:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00AF:    0F 83 37 01 00 00       jae      0x1ec    ; Conditional jump
00B5:    C1 E8 10                shr      eax, 0x10
00B8:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00BB:    DD 02                   fld      qword ptr [edx]
00BD:    D9 C0                   fld      st(0)
00BF:    D8 C8                   fmul     st(0)
00C1:    DE C9                   fmulp    st(1)
00C3:    DD 03                   fld      qword ptr [ebx]
00C5:    D8 C8                   fmul     st(0)
00C7:    DC 4F 08                fmul     qword ptr [edi + 8]
00CA:    DC 0A                   fmul     qword ptr [edx]
00CC:    DE C1                   faddp    st(1)
00CE:    DD 55 D8                fst      qword ptr [ebp - 0x28]
00D1:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00D4:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00D7:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00DA:    D8 C8                   fmul     st(0)
00DC:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
00DF:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00E2:    DC 47 18                fadd     qword ptr [edi + 0x18]
00E5:    DD 47 10                fld      qword ptr [edi + 0x10]
00E8:    DC 65 E0                fsub     qword ptr [ebp - 0x20]
00EB:    DE C9                   fmulp    st(1)
00ED:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
00F0:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
00F3:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00F6:    DC 47 18                fadd     qword ptr [edi + 0x18]
00F9:    D8 C8                   fmul     st(0)
00FB:    DC 45 D8                fadd     qword ptr [ebp - 0x28]
00FE:    DD 47 18                fld      qword ptr [edi + 0x18]
0101:    D9 E0                   fchs    ; -ST(0)
0103:    DE F1                   fdivrp   st(1)
0105:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0108:    90                      nop
0109:    DD 45 C8                fld      qword ptr [ebp - 0x38]
010C:    DC 0B                   fmul     qword ptr [ebx]
010E:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0111:    DC 0A                   fmul     qword ptr [edx]
0113:    D9 E0                   fchs    ; -ST(0)
0115:    DE C1                   faddp    st(1)
0117:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
011A:    90                      nop
011B:    DD 45 C8                fld      qword ptr [ebp - 0x38]
011E:    DC 0A                   fmul     qword ptr [edx]
0120:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0123:    DC 0B                   fmul     qword ptr [ebx]
0125:    DE C1                   faddp    st(1)
0127:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
012A:    90                      nop
012B:    DD 1A                   fstp     qword ptr [edx]
012D:    DD 1B                   fstp     qword ptr [ebx]
012F:    DD 45 B8                fld      qword ptr [ebp - 0x48]
0132:    D8 C8                   fmul     st(0)
0134:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
0137:    FE C8                   dec      al
0139:    0F 8F 52 FF FF FF       jg       0x91    ; Conditional jump
013F:    D9 D0                   fnop
0141:    DD 03                   fld      qword ptr [ebx]
0143:    D8 C8                   fmul     st(0)
0145:    DD 02                   fld      qword ptr [edx]
0147:    D8 C8                   fmul     st(0)
0149:    DE C1                   faddp    st(1)
014B:    DD 55 C0                fst      qword ptr [ebp - 0x40]
014E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0150:    DD 55 E8                fst      qword ptr [ebp - 0x18]
0153:    DC 22                   fsub     qword ptr [edx]
0155:    DC 33                   fdiv     qword ptr [ebx]
0157:    DD 55 F0                fst      qword ptr [ebp - 0x10]
015A:    D8 C8                   fmul     st(0)
015C:    DD 55 D8                fst      qword ptr [ebp - 0x28]
015F:    D9 E8                   fld1    ; Load 1.0
0161:    DE C1                   faddp    st(1)
0163:    D9 E8                   fld1    ; Load 1.0
0165:    DE F1                   fdivrp   st(1)
0167:    DD 55 E0                fst      qword ptr [ebp - 0x20]
016A:    D9 E8                   fld1    ; Load 1.0
016C:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
016F:    DE C9                   fmulp    st(1)
0171:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0174:    90                      nop
0175:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0178:    D8 C0                   fadd     st(0)
017A:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
017D:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0180:    90                      nop
0181:    D9 E8                   fld1    ; Load 1.0
0183:    DC 45 C0                fadd     qword ptr [ebp - 0x40]
0186:    DD 47 28                fld      qword ptr [edi + 0x28]
0189:    DE F1                   fdivrp   st(1)
018B:    DD 55 D8                fst      qword ptr [ebp - 0x28]
018E:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0191:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0194:    90                      nop
0195:    90                      nop
0196:    DD 45 B8                fld      qword ptr [ebp - 0x48]
0199:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
019C:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
019F:    90                      nop
01A0:    DD 45 B8                fld      qword ptr [ebp - 0x48]
01A3:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
01A6:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
01A9:    90                      nop
01AA:    DD 45 D8                fld      qword ptr [ebp - 0x28]
01AD:    D9 E8                   fld1    ; Load 1.0
01AF:    DE E9                   fsubp    st(1)
01B1:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
01B4:    90                      nop
01B5:    90                      nop
01B6:    DD 19                   fstp     qword ptr [ecx]
01B8:    DD 1A                   fstp     qword ptr [edx]
01BA:    DD 1B                   fstp     qword ptr [ebx]
01BC:    D9 D0                   fnop
01BE:    FE CC                   dec      ah
01C0:    80 FC 00                cmp      ah, 0
01C3:    0F 8F 53 FE FF FF       jg       0x1c    ; Conditional jump
01C9:    DD 03                   fld      qword ptr [ebx]
01CB:    DC 46 18                fadd     qword ptr [esi + 0x18]
01CE:    DD 02                   fld      qword ptr [edx]
01D0:    DC 46 20                fadd     qword ptr [esi + 0x20]
01D3:    DD 01                   fld      qword ptr [ecx]
01D5:    DC 46 28                fadd     qword ptr [esi + 0x28]
01D8:    DD 19                   fstp     qword ptr [ecx]
01DA:    DD 1A                   fstp     qword ptr [edx]
01DC:    DD 1B                   fstp     qword ptr [ebx]
01DE:    90                      nop
01DF:    8B C3                   mov      eax, ebx
01E1:    5F                      pop      edi
01E2:    5E                      pop      esi
01E3:    5B                      pop      ebx
01E4:    89 EC                   mov      esp, ebp
01E6:    5D                      pop      ebp
01E7:    C2 08 00                ret      8
01EA:    DD D8                   fstp     st(0)
01EC:    DD D8                   fstp     st(0)
01EE:    DD 47 30                fld      qword ptr [edi + 0x30]
01F1:    D9 C0                   fld      st(0)
01F3:    D9 C0                   fld      st(0)
01F5:    EB E1                   jmp      0x1d8
