; Disassembly: TgladDihed2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 528 bytes, decoded: 528 bytes (100.0%)

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
0039:    0F 83 C4 01 00 00       jae      0x203    ; Conditional jump
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
007D:    DD 1A                   fstp     qword ptr [edx]
007F:    90                      nop
0080:    D9 E8                   fld1    ; Load 1.0
0082:    DC 6D D8                fsubr    qword ptr [ebp - 0x28]
0085:    DE C9                   fmulp    st(1)
0087:    DD 1B                   fstp     qword ptr [ebx]
0089:    D9 D0                   fnop
008B:    8A 47 F0                mov      al, byte ptr [edi - 0x10]
008E:    24 07                   and      al, 7
0090:    90                      nop
0091:    DD 03                   fld      qword ptr [ebx]
0093:    D8 C8                   fmul     st(0)
0095:    DD 02                   fld      qword ptr [edx]
0097:    D8 C8                   fmul     st(0)
0099:    DE C1                   faddp    st(1)
009B:    C1 E0 10                shl      eax, 0x10
009E:    DC 57 30                fcom     qword ptr [edi + 0x30]
00A1:    9B                      wait
00A2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A4:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A6:    0F 83 59 01 00 00       jae      0x205    ; Conditional jump
00AC:    C1 E8 10                shr      eax, 0x10
00AF:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00B2:    90                      nop
00B3:    DD 02                   fld      qword ptr [edx]
00B5:    D9 E0                   fchs    ; -ST(0)
00B7:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00BA:    D9 E8                   fld1    ; Load 1.0
00BC:    DE C1                   faddp    st(1)
00BE:    DE C9                   fmulp    st(1)
00C0:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00C3:    DD 03                   fld      qword ptr [ebx]
00C5:    DC 4F 28                fmul     qword ptr [edi + 0x28]
00C8:    DE E9                   fsubp    st(1)
00CA:    D9 E8                   fld1    ; Load 1.0
00CC:    DE E9                   fsubp    st(1)
00CE:    DE C9                   fmulp    st(1)
00D0:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00D3:    DD 03                   fld      qword ptr [ebx]
00D5:    DC 4F 28                fmul     qword ptr [edi + 0x28]
00D8:    DE C1                   faddp    st(1)
00DA:    D9 E8                   fld1    ; Load 1.0
00DC:    DE E9                   fsubp    st(1)
00DE:    DE C9                   fmulp    st(1)
00E0:    DD 5D F8                fstp     qword ptr [ebp - 8]
00E3:    DD 03                   fld      qword ptr [ebx]
00E5:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00E8:    D9 E8                   fld1    ; Load 1.0
00EA:    DE E9                   fsubp    st(1)
00EC:    DE C9                   fmulp    st(1)
00EE:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00F1:    D8 C8                   fmul     st(0)
00F3:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00F6:    DC 4F 28                fmul     qword ptr [edi + 0x28]
00F9:    DE C1                   faddp    st(1)
00FB:    DD 02                   fld      qword ptr [edx]
00FD:    DC 4F 28                fmul     qword ptr [edi + 0x28]
0100:    D8 C8                   fmul     st(0)
0102:    DE C1                   faddp    st(1)
0104:    D9 E8                   fld1    ; Load 1.0
0106:    DE C1                   faddp    st(1)
0108:    DE C9                   fmulp    st(1)
010A:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
010D:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0110:    DD 02                   fld      qword ptr [edx]
0112:    DC 4F 28                fmul     qword ptr [edi + 0x28]
0115:    DE E9                   fsubp    st(1)
0117:    D9 E8                   fld1    ; Load 1.0
0119:    DE C1                   faddp    st(1)
011B:    DD 45 E8                fld      qword ptr [ebp - 0x18]
011E:    DD 02                   fld      qword ptr [edx]
0120:    DC 4F 28                fmul     qword ptr [edi + 0x28]
0123:    DE C1                   faddp    st(1)
0125:    D9 E8                   fld1    ; Load 1.0
0127:    DE C1                   faddp    st(1)
0129:    DE C9                   fmulp    st(1)
012B:    D8 C8                   fmul     st(0)
012D:    DD 47 28                fld      qword ptr [edi + 0x28]
0130:    DE F1                   fdivrp   st(1)
0132:    D9 C0                   fld      st(0)
0134:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0137:    DD 1B                   fstp     qword ptr [ebx]
0139:    DC 4D F8                fmul     qword ptr [ebp - 8]
013C:    DD 1A                   fstp     qword ptr [edx]
013E:    DD 45 B8                fld      qword ptr [ebp - 0x48]
0141:    D8 C8                   fmul     st(0)
0143:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
0146:    90                      nop
0147:    90                      nop
0148:    FE C8                   dec      al
014A:    0F 8F 41 FF FF FF       jg       0x91    ; Conditional jump
0150:    DD 03                   fld      qword ptr [ebx]
0152:    DD 02                   fld      qword ptr [edx]
0154:    DD 1B                   fstp     qword ptr [ebx]
0156:    DD 1A                   fstp     qword ptr [edx]
0158:    D9 D0                   fnop
015A:    DD 03                   fld      qword ptr [ebx]
015C:    D8 C8                   fmul     st(0)
015E:    DD 02                   fld      qword ptr [edx]
0160:    D8 C8                   fmul     st(0)
0162:    DE C1                   faddp    st(1)
0164:    DD 55 C0                fst      qword ptr [ebp - 0x40]
0167:    D9 FA                   fsqrt    ; sqrt(ST(0))
0169:    DD 55 E8                fst      qword ptr [ebp - 0x18]
016C:    DC 22                   fsub     qword ptr [edx]
016E:    DC 33                   fdiv     qword ptr [ebx]
0170:    DD 55 F0                fst      qword ptr [ebp - 0x10]
0173:    D8 C8                   fmul     st(0)
0175:    DD 55 D8                fst      qword ptr [ebp - 0x28]
0178:    D9 E8                   fld1    ; Load 1.0
017A:    DE C1                   faddp    st(1)
017C:    D9 E8                   fld1    ; Load 1.0
017E:    DE F1                   fdivrp   st(1)
0180:    DD 55 E0                fst      qword ptr [ebp - 0x20]
0183:    D9 E8                   fld1    ; Load 1.0
0185:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
0188:    DE C9                   fmulp    st(1)
018A:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
018D:    90                      nop
018E:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0191:    D8 C0                   fadd     st(0)
0193:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0196:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0199:    90                      nop
019A:    D9 E8                   fld1    ; Load 1.0
019C:    DC 45 C0                fadd     qword ptr [ebp - 0x40]
019F:    DD 47 28                fld      qword ptr [edi + 0x28]
01A2:    DE F1                   fdivrp   st(1)
01A4:    DD 55 D8                fst      qword ptr [ebp - 0x28]
01A7:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
01AA:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
01AD:    90                      nop
01AE:    90                      nop
01AF:    DD 45 B8                fld      qword ptr [ebp - 0x48]
01B2:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
01B5:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
01B8:    90                      nop
01B9:    DD 45 B8                fld      qword ptr [ebp - 0x48]
01BC:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
01BF:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
01C2:    90                      nop
01C3:    DD 45 D8                fld      qword ptr [ebp - 0x28]
01C6:    D9 E8                   fld1    ; Load 1.0
01C8:    DE E1                   fsubrp   st(1)
01CA:    DC 4D B8                fmul     qword ptr [ebp - 0x48]
01CD:    90                      nop
01CE:    90                      nop
01CF:    DD 19                   fstp     qword ptr [ecx]
01D1:    DD 1A                   fstp     qword ptr [edx]
01D3:    DD 1B                   fstp     qword ptr [ebx]
01D5:    D9 D0                   fnop
01D7:    FE CC                   dec      ah
01D9:    80 FC 00                cmp      ah, 0
01DC:    0F 8F 3A FE FF FF       jg       0x1c    ; Conditional jump
01E2:    DD 03                   fld      qword ptr [ebx]
01E4:    DC 46 18                fadd     qword ptr [esi + 0x18]
01E7:    DD 02                   fld      qword ptr [edx]
01E9:    DC 46 20                fadd     qword ptr [esi + 0x20]
01EC:    DD 01                   fld      qword ptr [ecx]
01EE:    DC 46 28                fadd     qword ptr [esi + 0x28]
01F1:    DD 19                   fstp     qword ptr [ecx]
01F3:    DD 1A                   fstp     qword ptr [edx]
01F5:    DD 1B                   fstp     qword ptr [ebx]
01F7:    90                      nop
01F8:    8B C3                   mov      eax, ebx
01FA:    5F                      pop      edi
01FB:    5E                      pop      esi
01FC:    5B                      pop      ebx
01FD:    89 EC                   mov      esp, ebp
01FF:    5D                      pop      ebp
0200:    C2 08 00                ret      8
0203:    DD D8                   fstp     st(0)
0205:    DD D8                   fstp     st(0)
0207:    DD 47 30                fld      qword ptr [edi + 0x30]
020A:    D9 C0                   fld      st(0)
020C:    D9 C0                   fld      st(0)
020E:    EB E1                   jmp      0x1f1
