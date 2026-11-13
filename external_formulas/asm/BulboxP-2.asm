; Disassembly: BulboxP-2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 553 bytes, decoded: 553 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 80 00 00 00       sub      esp, 0x80
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 03                   fld      qword ptr [ebx]
0018:    DC 47 E8                fadd     qword ptr [edi - 0x18]
001B:    D9 E1                   fabs    ; |ST(0)|
001D:    DD 03                   fld      qword ptr [ebx]
001F:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0022:    D9 E1                   fabs    ; |ST(0)|
0024:    D9 E0                   fchs    ; -ST(0)
0026:    DC 23                   fsub     qword ptr [ebx]
0028:    DE C1                   faddp    st(1)
002A:    DD 1B                   fstp     qword ptr [ebx]
002C:    DD 02                   fld      qword ptr [edx]
002E:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0031:    D9 E1                   fabs    ; |ST(0)|
0033:    DD 02                   fld      qword ptr [edx]
0035:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0038:    D9 E1                   fabs    ; |ST(0)|
003A:    D9 E0                   fchs    ; -ST(0)
003C:    DC 22                   fsub     qword ptr [edx]
003E:    DE C1                   faddp    st(1)
0040:    DD 1A                   fstp     qword ptr [edx]
0042:    DD 01                   fld      qword ptr [ecx]
0044:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0047:    D9 E1                   fabs    ; |ST(0)|
0049:    DD 01                   fld      qword ptr [ecx]
004B:    DC 67 E8                fsub     qword ptr [edi - 0x18]
004E:    D9 E1                   fabs    ; |ST(0)|
0050:    D9 E0                   fchs    ; -ST(0)
0052:    DC 21                   fsub     qword ptr [ecx]
0054:    DE C1                   faddp    st(1)
0056:    DD 19                   fstp     qword ptr [ecx]
0058:    DD 03                   fld      qword ptr [ebx]
005A:    D8 C8                   fmul     st(0)
005C:    DD 02                   fld      qword ptr [edx]
005E:    D8 C8                   fmul     st(0)
0060:    DD 01                   fld      qword ptr [ecx]
0062:    D8 C8                   fmul     st(0)
0064:    DE C1                   faddp    st(1)
0066:    DE C1                   faddp    st(1)
0068:    DD 55 E8                fst      qword ptr [ebp - 0x18]
006B:    D9 FA                   fsqrt    ; sqrt(ST(0))
006D:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0070:    DD 47 F0                fld      qword ptr [edi - 0x10]
0073:    DD 03                   fld      qword ptr [ebx]
0075:    D8 C9                   fmul     st(1)
0077:    DC 67 C8                fsub     qword ptr [edi - 0x38]
007A:    DD 1B                   fstp     qword ptr [ebx]
007C:    DD 02                   fld      qword ptr [edx]
007E:    D8 C9                   fmul     st(1)
0080:    DC 67 C0                fsub     qword ptr [edi - 0x40]
0083:    DD 1A                   fstp     qword ptr [edx]
0085:    DD 01                   fld      qword ptr [ecx]
0087:    D8 C9                   fmul     st(1)
0089:    DC 67 B8                fsub     qword ptr [edi - 0x48]
008C:    DD 19                   fstp     qword ptr [ecx]
008E:    DD D8                   fstp     st(0)
0090:    83 7F AC 00             cmp      dword ptr [edi - 0x54], 0
0094:    75 15                   jne      0xab    ; Conditional jump
0096:    90                      nop
0097:    DD 45 E0                fld      qword ptr [ebp - 0x20]
009A:    D9 E8                   fld1    ; Load 1.0
009C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
009E:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00A0:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A2:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A4:    0F 83 C9 00 00 00       jae      0x173    ; Conditional jump
00AA:    90                      nop
00AB:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00AE:    DD 47 E0                fld      qword ptr [edi - 0x20]
00B1:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00B3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00B5:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00B7:    0F 83 D1 00 00 00       jae      0x18e    ; Conditional jump
00BD:    90                      nop
00BE:    90                      nop
00BF:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00C2:    DC 67 E0                fsub     qword ptr [edi - 0x20]
00C5:    D9 E8                   fld1    ; Load 1.0
00C7:    DC 67 E0                fsub     qword ptr [edi - 0x20]
00CA:    DE F9                   fdivp    st(1)
00CC:    DD 55 D8                fst      qword ptr [ebp - 0x28]
00CF:    D9 E8                   fld1    ; Load 1.0
00D1:    D8 E1                   fsub     st(1)
00D3:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00D6:    DD D8                   fstp     st(0)
00D8:    90                      nop
00D9:    DD 02                   fld      qword ptr [edx]
00DB:    D8 C8                   fmul     st(0)
00DD:    DD 03                   fld      qword ptr [ebx]
00DF:    D8 C8                   fmul     st(0)
00E1:    DE C1                   faddp    st(1)
00E3:    DD 47 D0                fld      qword ptr [edi - 0x30]
00E6:    D9 E1                   fabs    ; |ST(0)|
00E8:    DE C1                   faddp    st(1)
00EA:    D9 FA                   fsqrt    ; sqrt(ST(0))
00EC:    DD 5D F8                fstp     qword ptr [ebp - 8]
00EF:    90                      nop
00F0:    90                      nop
00F1:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00F4:    D8 C8                   fmul     st(0)
00F6:    DD 47 D8                fld      qword ptr [edi - 0x28]
00F9:    DE F1                   fdivrp   st(1)
00FB:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00FE:    90                      nop
00FF:    DD 01                   fld      qword ptr [ecx]
0101:    DC 75 F8                fdiv     qword ptr [ebp - 8]
0104:    D8 C8                   fmul     st(0)
0106:    D9 E0                   fchs    ; -ST(0)
0108:    D9 E8                   fld1    ; Load 1.0
010A:    DE C1                   faddp    st(1)
010C:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
010F:    90                      nop
0110:    DD 03                   fld      qword ptr [ebx]
0112:    D8 C8                   fmul     st(0)
0114:    DD 02                   fld      qword ptr [edx]
0116:    D8 C8                   fmul     st(0)
0118:    D9 E0                   fchs    ; -ST(0)
011A:    DE C1                   faddp    st(1)
011C:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
011F:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0122:    90                      nop
0123:    DD 03                   fld      qword ptr [ebx]
0125:    DC 0A                   fmul     qword ptr [edx]
0127:    D9 E0                   fchs    ; -ST(0)
0129:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
012C:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
012F:    90                      nop
0130:    DD 01                   fld      qword ptr [ecx]
0132:    D8 C0                   fadd     st(0)
0134:    D9 E0                   fchs    ; -ST(0)
0136:    DC 4F B0                fmul     qword ptr [edi - 0x50]
0139:    DC 4D F8                fmul     qword ptr [ebp - 8]
013C:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
013F:    90                      nop
0140:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0143:    DD 01                   fld      qword ptr [ecx]
0145:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0148:    DE C1                   faddp    st(1)
014A:    DC 46 28                fadd     qword ptr [esi + 0x28]
014D:    DD 19                   fstp     qword ptr [ecx]
014F:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0152:    DD 02                   fld      qword ptr [edx]
0154:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0157:    DE C1                   faddp    st(1)
0159:    DC 46 20                fadd     qword ptr [esi + 0x20]
015C:    DD 1A                   fstp     qword ptr [edx]
015E:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0161:    DD 03                   fld      qword ptr [ebx]
0163:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0166:    DE C1                   faddp    st(1)
0168:    DC 46 18                fadd     qword ptr [esi + 0x18]
016B:    DD 1B                   fstp     qword ptr [ebx]
016D:    E9 95 00 00 00          jmp      0x207
0172:    90                      nop
0173:    DD 03                   fld      qword ptr [ebx]
0175:    DC 46 18                fadd     qword ptr [esi + 0x18]
0178:    DD 1B                   fstp     qword ptr [ebx]
017A:    DD 02                   fld      qword ptr [edx]
017C:    DC 46 20                fadd     qword ptr [esi + 0x20]
017F:    DD 1A                   fstp     qword ptr [edx]
0181:    DD 01                   fld      qword ptr [ecx]
0183:    DC 46 28                fadd     qword ptr [esi + 0x28]
0186:    DD 19                   fstp     qword ptr [ecx]
0188:    E9 7A 00 00 00          jmp      0x207
018D:    90                      nop
018E:    DD 02                   fld      qword ptr [edx]
0190:    D8 C8                   fmul     st(0)
0192:    DD 03                   fld      qword ptr [ebx]
0194:    D8 C8                   fmul     st(0)
0196:    DE C1                   faddp    st(1)
0198:    DD 47 D0                fld      qword ptr [edi - 0x30]
019B:    D9 E1                   fabs    ; |ST(0)|
019D:    DE C1                   faddp    st(1)
019F:    D9 FA                   fsqrt    ; sqrt(ST(0))
01A1:    DD 5D F8                fstp     qword ptr [ebp - 8]
01A4:    90                      nop
01A5:    DD 45 E8                fld      qword ptr [ebp - 0x18]
01A8:    D8 C8                   fmul     st(0)
01AA:    DD 47 D8                fld      qword ptr [edi - 0x28]
01AD:    DE F1                   fdivrp   st(1)
01AF:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
01B2:    90                      nop
01B3:    DD 01                   fld      qword ptr [ecx]
01B5:    DC 75 F8                fdiv     qword ptr [ebp - 8]
01B8:    D8 C8                   fmul     st(0)
01BA:    D9 E0                   fchs    ; -ST(0)
01BC:    D9 E8                   fld1    ; Load 1.0
01BE:    DE C1                   faddp    st(1)
01C0:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
01C3:    90                      nop
01C4:    DD 03                   fld      qword ptr [ebx]
01C6:    D8 C8                   fmul     st(0)
01C8:    DD 02                   fld      qword ptr [edx]
01CA:    D8 C8                   fmul     st(0)
01CC:    D9 E0                   fchs    ; -ST(0)
01CE:    DE C1                   faddp    st(1)
01D0:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
01D3:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
01D6:    DC 46 18                fadd     qword ptr [esi + 0x18]
01D9:    90                      nop
01DA:    DD 03                   fld      qword ptr [ebx]
01DC:    DC 0A                   fmul     qword ptr [edx]
01DE:    D9 E0                   fchs    ; -ST(0)
01E0:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
01E3:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
01E6:    DC 46 20                fadd     qword ptr [esi + 0x20]
01E9:    90                      nop
01EA:    DD 01                   fld      qword ptr [ecx]
01EC:    D8 C0                   fadd     st(0)
01EE:    D9 E0                   fchs    ; -ST(0)
01F0:    DC 4D F8                fmul     qword ptr [ebp - 8]
01F3:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
01F6:    DC 4F B0                fmul     qword ptr [edi - 0x50]
01F9:    DC 46 28                fadd     qword ptr [esi + 0x28]
01FC:    90                      nop
01FD:    DD 19                   fstp     qword ptr [ecx]
01FF:    DD 1A                   fstp     qword ptr [edx]
0201:    DD 1B                   fstp     qword ptr [ebx]
0203:    90                      nop
0204:    90                      nop
0205:    90                      nop
0206:    90                      nop
0207:    DD 03                   fld      qword ptr [ebx]
0209:    DC 47 C8                fadd     qword ptr [edi - 0x38]
020C:    DD 1B                   fstp     qword ptr [ebx]
020E:    DD 02                   fld      qword ptr [edx]
0210:    DC 47 C0                fadd     qword ptr [edi - 0x40]
0213:    DD 1A                   fstp     qword ptr [edx]
0215:    DD 01                   fld      qword ptr [ecx]
0217:    DC 47 B8                fadd     qword ptr [edi - 0x48]
021A:    DD 19                   fstp     qword ptr [ecx]
021C:    D9 D0                   fnop
021E:    8B C3                   mov      eax, ebx
0220:    5F                      pop      edi
0221:    5E                      pop      esi
0222:    5B                      pop      ebx
0223:    89 EC                   mov      esp, ebp
0225:    5D                      pop      ebp
0226:    C2 08 00                ret      8
