; Disassembly: _IFS_tess.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 796 bytes, decoded: 796 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 50 00 00 00       sub      esp, 0x50
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 03                   fld      qword ptr [ebx]
0018:    DC 47 F0                fadd     qword ptr [edi - 0x10]
001B:    DD 1B                   fstp     qword ptr [ebx]
001D:    DD 02                   fld      qword ptr [edx]
001F:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0022:    DD 1A                   fstp     qword ptr [edx]
0024:    DD 01                   fld      qword ptr [ecx]
0026:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0029:    DD 19                   fstp     qword ptr [ecx]
002B:    D9 D0                   fnop
002D:    DD 02                   fld      qword ptr [edx]
002F:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0032:    DD 01                   fld      qword ptr [ecx]
0034:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0037:    D9 EE                   fldz    ; Load 0.0
0039:    DB 5D C4                fistp    dword ptr [ebp - 0x3c]
003C:    D9 EE                   fldz    ; Load 0.0
003E:    DB 5D B0                fistp    dword ptr [ebp - 0x50]
0041:    DD 47 C0                fld      qword ptr [edi - 0x40]
0044:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0047:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0049:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
004C:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
004F:    90                      nop
0050:    DD 03                   fld      qword ptr [ebx]
0052:    DD 55 BC                fst      qword ptr [ebp - 0x44]
0055:    D9 E1                   fabs    ; |ST(0)|
0057:    DD 47 C8                fld      qword ptr [edi - 0x38]
005A:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
005C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005E:    80 E4 41                and      ah, 0x41
0061:    0F 87 87 02 00 00       ja       0x2ee    ; Conditional jump
0067:    D9 D0                   fnop
0069:    DD 45 F0                fld      qword ptr [ebp - 0x10]
006C:    D9 E1                   fabs    ; |ST(0)|
006E:    DD 47 C8                fld      qword ptr [edi - 0x38]
0071:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0073:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0075:    80 E4 41                and      ah, 0x41
0078:    0F 87 17 00 00 00       ja       0x95    ; Conditional jump
007E:    D9 D0                   fnop
0080:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0083:    D9 E1                   fabs    ; |ST(0)|
0085:    DD 47 C8                fld      qword ptr [edi - 0x38]
0088:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
008A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
008C:    80 E4 41                and      ah, 0x41
008F:    0F 86 59 02 00 00       jbe      0x2ee    ; Conditional jump
0095:    83 7F B8 04             cmp      dword ptr [edi - 0x48], 4
0099:    0F 8C E3 00 00 00       jl       0x182    ; Conditional jump
009F:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00A2:    D9 E1                   fabs    ; |ST(0)|
00A4:    DD 47 C8                fld      qword ptr [edi - 0x38]
00A7:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00A9:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AB:    80 E4 41                and      ah, 0x41
00AE:    0F 87 17 00 00 00       ja       0xcb    ; Conditional jump
00B4:    D9 D0                   fnop
00B6:    DD 45 E8                fld      qword ptr [ebp - 0x18]
00B9:    D9 E1                   fabs    ; |ST(0)|
00BB:    DD 47 C8                fld      qword ptr [edi - 0x38]
00BE:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00C0:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C2:    80 E4 41                and      ah, 0x41
00C5:    0F 86 46 01 00 00       jbe      0x211    ; Conditional jump
00CB:    D9 D0                   fnop
00CD:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00D0:    D9 E1                   fabs    ; |ST(0)|
00D2:    DD 55 F8                fst      qword ptr [ebp - 8]
00D5:    D9 EE                   fldz    ; Load 0.0
00D7:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00D9:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DB:    80 E4 41                and      ah, 0x41
00DE:    80 F4 40                xor      ah, 0x40
00E1:    74 09                   je       0xec    ; Conditional jump
00E3:    DD 45 F8                fld      qword ptr [ebp - 8]
00E6:    DC 75 F0                fdiv     qword ptr [ebp - 0x10]
00E9:    DD 5D F8                fstp     qword ptr [ebp - 8]
00EC:    90                      nop
00ED:    DD 45 F8                fld      qword ptr [ebp - 8]
00F0:    DC 4F D0                fmul     qword ptr [edi - 0x30]
00F3:    D9 E0                   fchs    ; -ST(0)
00F5:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
00F8:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00FB:    D9 D0                   fnop
00FD:    DD 47 D0                fld      qword ptr [edi - 0x30]
0100:    DC 0F                   fmul     qword ptr [edi]
0102:    D9 E0                   fchs    ; -ST(0)
0104:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0107:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
010A:    DD 45 F8                fld      qword ptr [ebp - 8]
010D:    D9 EE                   fldz    ; Load 0.0
010F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0111:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0113:    80 E4 41                and      ah, 0x41
0116:    76 37                   jbe      0x14f    ; Conditional jump
0118:    90                      nop
0119:    90                      nop
011A:    90                      nop
011B:    90                      nop
011C:    DD 45 E0                fld      qword ptr [ebp - 0x20]
011F:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0122:    D9 D0                   fnop
0124:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0127:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
012A:    D9 E0                   fchs    ; -ST(0)
012C:    DE C1                   faddp    st(1)
012E:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0131:    D9 D0                   fnop
0133:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0136:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0139:    D9 D0                   fnop
013B:    DD 45 D8                fld      qword ptr [ebp - 0x28]
013E:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0141:    D9 D0                   fnop
0143:    DE C1                   faddp    st(1)
0145:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0148:    D9 D0                   fnop
014A:    E9 2C 00 00 00          jmp      0x17b
014F:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0152:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0155:    D9 D0                   fnop
0157:    DD 45 D8                fld      qword ptr [ebp - 0x28]
015A:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
015D:    D9 D0                   fnop
015F:    DE C1                   faddp    st(1)
0161:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0164:    D9 D0                   fnop
0166:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0169:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
016C:    D9 E0                   fchs    ; -ST(0)
016E:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0171:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0174:    D9 D0                   fnop
0176:    DE C1                   faddp    st(1)
0178:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
017B:    D9 D0                   fnop
017D:    E9 C8 00 00 00          jmp      0x24a
0182:    D9 E8                   fld1    ; Load 1.0
0184:    DD 5D F8                fstp     qword ptr [ebp - 8]
0187:    D9 D0                   fnop
0189:    DD 45 F0                fld      qword ptr [ebp - 0x10]
018C:    DD 45 E8                fld      qword ptr [ebp - 0x18]
018F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0191:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0193:    80 E4 41                and      ah, 0x41
0196:    0F 86 12 00 00 00       jbe      0x1ae    ; Conditional jump
019C:    DD 45 F0                fld      qword ptr [ebp - 0x10]
019F:    DD 47 D0                fld      qword ptr [edi - 0x30]
01A2:    DC 4F 08                fmul     qword ptr [edi + 8]
01A5:    D9 E0                   fchs    ; -ST(0)
01A7:    DE C1                   faddp    st(1)
01A9:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
01AC:    EB 18                   jmp      0x1c6
01AE:    DD 45 E8                fld      qword ptr [ebp - 0x18]
01B1:    DD 47 D0                fld      qword ptr [edi - 0x30]
01B4:    DC 4F 08                fmul     qword ptr [edi + 8]
01B7:    D9 E0                   fchs    ; -ST(0)
01B9:    DE C1                   faddp    st(1)
01BB:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
01BE:    DD 45 F8                fld      qword ptr [ebp - 8]
01C1:    D9 E0                   fchs    ; -ST(0)
01C3:    DD 5D F8                fstp     qword ptr [ebp - 8]
01C6:    DD 45 F0                fld      qword ptr [ebp - 0x10]
01C9:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
01CC:    D9 E0                   fchs    ; -ST(0)
01CE:    DD 45 E8                fld      qword ptr [ebp - 0x18]
01D1:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
01D4:    D9 E0                   fchs    ; -ST(0)
01D6:    DE C1                   faddp    st(1)
01D8:    D9 D0                   fnop
01DA:    DD 45 F0                fld      qword ptr [ebp - 0x10]
01DD:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
01E0:    D9 D0                   fnop
01E2:    DD 45 E8                fld      qword ptr [ebp - 0x18]
01E5:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
01E8:    D9 E0                   fchs    ; -ST(0)
01EA:    DE C1                   faddp    st(1)
01EC:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
01EF:    D9 D0                   fnop
01F1:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
01F4:    DD 45 F8                fld      qword ptr [ebp - 8]
01F7:    D9 EE                   fldz    ; Load 0.0
01F9:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
01FB:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01FD:    80 E4 41                and      ah, 0x41
0200:    0F 87 42 00 00 00       ja       0x248    ; Conditional jump
0206:    83 7F B8 01             cmp      dword ptr [edi - 0x48], 1
020A:    75 12                   jne      0x21e    ; Conditional jump
020C:    DD 45 F0                fld      qword ptr [ebp - 0x10]
020F:    D9 E0                   fchs    ; -ST(0)
0211:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0214:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0217:    D9 E0                   fchs    ; -ST(0)
0219:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
021C:    EB 2A                   jmp      0x248
021E:    83 7F B8 02             cmp      dword ptr [edi - 0x48], 2
0222:    75 0E                   jne      0x232    ; Conditional jump
0224:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0227:    DD 45 E8                fld      qword ptr [ebp - 0x18]
022A:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
022D:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0230:    EB 16                   jmp      0x248
0232:    83 7F B8 03             cmp      dword ptr [edi - 0x48], 3
0236:    75 10                   jne      0x248    ; Conditional jump
0238:    DD 45 F0                fld      qword ptr [ebp - 0x10]
023B:    DD 45 E8                fld      qword ptr [ebp - 0x18]
023E:    D9 E0                   fchs    ; -ST(0)
0240:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0243:    D9 E0                   fchs    ; -ST(0)
0245:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0248:    D9 D0                   fnop
024A:    DD 45 F0                fld      qword ptr [ebp - 0x10]
024D:    DC 77 08                fdiv     qword ptr [edi + 8]
0250:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0253:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0256:    DC 77 08                fdiv     qword ptr [edi + 8]
0259:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
025C:    D9 D0                   fnop
025E:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0261:    D9 E1                   fabs    ; |ST(0)|
0263:    DD 47 C8                fld      qword ptr [edi - 0x38]
0266:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0268:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
026A:    80 E4 41                and      ah, 0x41
026D:    77 2C                   ja       0x29b    ; Conditional jump
026F:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0272:    D9 E1                   fabs    ; |ST(0)|
0274:    DD 47 C8                fld      qword ptr [edi - 0x38]
0277:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0279:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
027B:    80 E4 41                and      ah, 0x41
027E:    77 1B                   ja       0x29b    ; Conditional jump
0280:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0283:    DD 1A                   fstp     qword ptr [edx]
0285:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0288:    DD 19                   fstp     qword ptr [ecx]
028A:    DD 45 BC                fld      qword ptr [ebp - 0x44]
028D:    DC 77 08                fdiv     qword ptr [edi + 8]
0290:    DD 5D BC                fstp     qword ptr [ebp - 0x44]
0293:    D9 E8                   fld1    ; Load 1.0
0295:    DB 5D B0                fistp    dword ptr [ebp - 0x50]
0298:    EB 2E                   jmp      0x2c8
029A:    90                      nop
029B:    DD 45 E0                fld      qword ptr [ebp - 0x20]
029E:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
02A1:    DD 45 D8                fld      qword ptr [ebp - 0x28]
02A4:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
02A7:    8B 45 C4                mov      eax, dword ptr [ebp - 0x3c]
02AA:    39 47 1C                cmp      dword ptr [edi + 0x1c], eax
02AD:    76 19                   jbe      0x2c8    ; Conditional jump
02AF:    39 47 BC                cmp      dword ptr [edi - 0x44], eax
02B2:    90                      nop
02B3:    76 13                   jbe      0x2c8    ; Conditional jump
02B5:    83 45 C4 01             add      dword ptr [ebp - 0x3c], 1
02B9:    DD 45 BC                fld      qword ptr [ebp - 0x44]
02BC:    DC 77 08                fdiv     qword ptr [edi + 8]
02BF:    DD 5D BC                fstp     qword ptr [ebp - 0x44]
02C2:    E9 CE FD FF FF          jmp      0x95
02C7:    90                      nop
02C8:    83 7D B0 01             cmp      dword ptr [ebp - 0x50], 1
02CC:    74 20                   je       0x2ee    ; Conditional jump
02CE:    90                      nop
02CF:    90                      nop
02D0:    90                      nop
02D1:    90                      nop
02D2:    DD 47 18                fld      qword ptr [edi + 0x18]
02D5:    DC 0A                   fmul     qword ptr [edx]
02D7:    DD 1A                   fstp     qword ptr [edx]
02D9:    DD 47 18                fld      qword ptr [edi + 0x18]
02DC:    DC 09                   fmul     qword ptr [ecx]
02DE:    DD 19                   fstp     qword ptr [ecx]
02E0:    DD 47 18                fld      qword ptr [edi + 0x18]
02E3:    DC 0B                   fmul     qword ptr [ebx]
02E5:    90                      nop
02E6:    DD 1B                   fstp     qword ptr [ebx]
02E8:    90                      nop
02E9:    E9 23 00 00 00          jmp      0x311
02EE:    DD 45 BC                fld      qword ptr [ebp - 0x44]
02F1:    DC 4F D8                fmul     qword ptr [edi - 0x28]
02F4:    DC 67 F0                fsub     qword ptr [edi - 0x10]
02F7:    DD 1B                   fstp     qword ptr [ebx]
02F9:    DD 02                   fld      qword ptr [edx]
02FB:    DC 4F D8                fmul     qword ptr [edi - 0x28]
02FE:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0301:    DD 1A                   fstp     qword ptr [edx]
0303:    DD 01                   fld      qword ptr [ecx]
0305:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0308:    DC 67 E0                fsub     qword ptr [edi - 0x20]
030B:    DD 19                   fstp     qword ptr [ecx]
030D:    D9 D0                   fnop
030F:    D9 D0                   fnop
0311:    8B C3                   mov      eax, ebx
0313:    5F                      pop      edi
0314:    5E                      pop      esi
0315:    5B                      pop      ebx
0316:    8B E5                   mov      esp, ebp
0318:    5D                      pop      ebp
0319:    C2 08 00                ret      8
