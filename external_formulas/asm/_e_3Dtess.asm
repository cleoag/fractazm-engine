; Disassembly: _e_3Dtess.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 768 bytes, decoded: 768 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 2C 00 00 00       sub      esp, 0x2c
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 03                   fld      qword ptr [ebx]
0018:    DC 4F C8                fmul     qword ptr [edi - 0x38]
001B:    DC 47 F0                fadd     qword ptr [edi - 0x10]
001E:    DD 1B                   fstp     qword ptr [ebx]
0020:    DD 02                   fld      qword ptr [edx]
0022:    DC 4F C8                fmul     qword ptr [edi - 0x38]
0025:    DC 47 E8                fadd     qword ptr [edi - 0x18]
0028:    DD 1A                   fstp     qword ptr [edx]
002A:    DD 01                   fld      qword ptr [ecx]
002C:    DC 4F C8                fmul     qword ptr [edi - 0x38]
002F:    DC 47 E0                fadd     qword ptr [edi - 0x20]
0032:    DD 19                   fstp     qword ptr [ecx]
0034:    D9 D0                   fnop
0036:    83 7F C4 01             cmp      dword ptr [edi - 0x3c], 1
003A:    75 0A                   jne      0x46    ; Conditional jump
003C:    DD 07                   fld      qword ptr [edi]
003E:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0041:    E9 28 00 00 00          jmp      0x6e
0046:    83 7F C4 02             cmp      dword ptr [edi - 0x3c], 2
004A:    75 0B                   jne      0x57    ; Conditional jump
004C:    DD 47 10                fld      qword ptr [edi + 0x10]
004F:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0052:    E9 17 00 00 00          jmp      0x6e
0057:    83 7F C4 03             cmp      dword ptr [edi - 0x3c], 3
005B:    75 0B                   jne      0x68    ; Conditional jump
005D:    DD 47 18                fld      qword ptr [edi + 0x18]
0060:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0063:    E9 06 00 00 00          jmp      0x6e
0068:    D9 E8                   fld1    ; Load 1.0
006A:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
006D:    90                      nop
006E:    D9 EE                   fldz    ; Load 0.0
0070:    DB 5D E8                fistp    dword ptr [ebp - 0x18]
0073:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0076:    D9 E8                   fld1    ; Load 1.0
0078:    DE C1                   faddp    st(1)
007A:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
007D:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0080:    D9 EE                   fldz    ; Load 0.0
0082:    DB 5D D4                fistp    dword ptr [ebp - 0x2c]
0085:    90                      nop
0086:    D9 D0                   fnop
0088:    83 45 D4 01             add      dword ptr [ebp - 0x2c], 1
008C:    DD 03                   fld      qword ptr [ebx]
008E:    D9 E1                   fabs    ; |ST(0)|
0090:    DD 5D F8                fstp     qword ptr [ebp - 8]
0093:    D9 D0                   fnop
0095:    D9 EE                   fldz    ; Load 0.0
0097:    DB 5D EC                fistp    dword ptr [ebp - 0x14]
009A:    DD 02                   fld      qword ptr [edx]
009C:    D9 E1                   fabs    ; |ST(0)|
009E:    DD 45 F8                fld      qword ptr [ebp - 8]
00A1:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00A3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A5:    80 E4 41                and      ah, 0x41
00A8:    7E 0E                   jle      0xb8    ; Conditional jump
00AA:    DD 02                   fld      qword ptr [edx]
00AC:    D9 E1                   fabs    ; |ST(0)|
00AE:    DD 5D F8                fstp     qword ptr [ebp - 8]
00B1:    D9 E8                   fld1    ; Load 1.0
00B3:    DB 5D EC                fistp    dword ptr [ebp - 0x14]
00B6:    D9 D0                   fnop
00B8:    DD 01                   fld      qword ptr [ecx]
00BA:    D9 E1                   fabs    ; |ST(0)|
00BC:    DD 45 F8                fld      qword ptr [ebp - 8]
00BF:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00C1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C3:    80 E4 41                and      ah, 0x41
00C6:    7E 0F                   jle      0xd7    ; Conditional jump
00C8:    DD 01                   fld      qword ptr [ecx]
00CA:    D9 E1                   fabs    ; |ST(0)|
00CC:    DD 5D F8                fstp     qword ptr [ebp - 8]
00CF:    DD 47 08                fld      qword ptr [edi + 8]
00D2:    DB 5D EC                fistp    dword ptr [ebp - 0x14]
00D5:    D9 D0                   fnop
00D7:    DD 45 F8                fld      qword ptr [ebp - 8]
00DA:    DD 45 E0                fld      qword ptr [ebp - 0x20]
00DD:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00DF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E1:    80 E4 41                and      ah, 0x41
00E4:    0F 8F A7 01 00 00       jg       0x291    ; Conditional jump
00EA:    DD 45 F8                fld      qword ptr [ebp - 8]
00ED:    DD 45 D8                fld      qword ptr [ebp - 0x28]
00F0:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00F2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F4:    80 E4 41                and      ah, 0x41
00F7:    0F 8E 94 01 00 00       jle      0x291    ; Conditional jump
00FD:    D9 E8                   fld1    ; Load 1.0
00FF:    DB 5D E8                fistp    dword ptr [ebp - 0x18]
0102:    83 7D EC 00             cmp      dword ptr [ebp - 0x14], 0
0106:    0F 85 7B 00 00 00       jne      0x187    ; Conditional jump
010C:    90                      nop
010D:    DD 45 F8                fld      qword ptr [ebp - 8]
0110:    DC 33                   fdiv     qword ptr [ebx]
0112:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0115:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0118:    DC 45 D8                fadd     qword ptr [ebp - 0x28]
011B:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
011E:    DC 0F                   fmul     qword ptr [edi]
0120:    D9 E0                   fchs    ; -ST(0)
0122:    DC 03                   fadd     qword ptr [ebx]
0124:    DC 4F 08                fmul     qword ptr [edi + 8]
0127:    DC 4F D0                fmul     qword ptr [edi - 0x30]
012A:    DD 45 E0                fld      qword ptr [ebp - 0x20]
012D:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
0130:    DE F9                   fdivp    st(1)
0132:    DD 1B                   fstp     qword ptr [ebx]
0134:    D9 D0                   fnop
0136:    DD 02                   fld      qword ptr [edx]
0138:    DD 45 E0                fld      qword ptr [ebp - 0x20]
013B:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
013E:    DE F9                   fdivp    st(1)
0140:    DD 1A                   fstp     qword ptr [edx]
0142:    D9 D0                   fnop
0144:    DD 02                   fld      qword ptr [edx]
0146:    DC 07                   fadd     qword ptr [edi]
0148:    D9 FC                   frndint    ; round(ST(0))
014A:    D9 E0                   fchs    ; -ST(0)
014C:    DC 07                   fadd     qword ptr [edi]
014E:    DC 02                   fadd     qword ptr [edx]
0150:    DC 4F 08                fmul     qword ptr [edi + 8]
0153:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0156:    DD 1A                   fstp     qword ptr [edx]
0158:    D9 D0                   fnop
015A:    D9 D0                   fnop
015C:    DD 01                   fld      qword ptr [ecx]
015E:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0161:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
0164:    DE F9                   fdivp    st(1)
0166:    DD 19                   fstp     qword ptr [ecx]
0168:    D9 D0                   fnop
016A:    DD 01                   fld      qword ptr [ecx]
016C:    DC 07                   fadd     qword ptr [edi]
016E:    D9 FC                   frndint    ; round(ST(0))
0170:    D9 E0                   fchs    ; -ST(0)
0172:    DC 07                   fadd     qword ptr [edi]
0174:    DC 01                   fadd     qword ptr [ecx]
0176:    DC 4F 08                fmul     qword ptr [edi + 8]
0179:    DC 4F D0                fmul     qword ptr [edi - 0x30]
017C:    DD 19                   fstp     qword ptr [ecx]
017E:    D9 D0                   fnop
0180:    D9 D0                   fnop
0182:    E9 4C 01 00 00          jmp      0x2d3
0187:    83 7D EC 01             cmp      dword ptr [ebp - 0x14], 1
018B:    0F 85 7B 00 00 00       jne      0x20c    ; Conditional jump
0191:    90                      nop
0192:    DD 45 F8                fld      qword ptr [ebp - 8]
0195:    DC 32                   fdiv     qword ptr [edx]
0197:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
019A:    DD 45 E0                fld      qword ptr [ebp - 0x20]
019D:    DC 45 D8                fadd     qword ptr [ebp - 0x28]
01A0:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
01A3:    DC 0F                   fmul     qword ptr [edi]
01A5:    D9 E0                   fchs    ; -ST(0)
01A7:    DC 02                   fadd     qword ptr [edx]
01A9:    DC 4F 08                fmul     qword ptr [edi + 8]
01AC:    DC 4F D0                fmul     qword ptr [edi - 0x30]
01AF:    DD 45 E0                fld      qword ptr [ebp - 0x20]
01B2:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
01B5:    DE F9                   fdivp    st(1)
01B7:    DD 1A                   fstp     qword ptr [edx]
01B9:    D9 D0                   fnop
01BB:    DD 03                   fld      qword ptr [ebx]
01BD:    DD 45 E0                fld      qword ptr [ebp - 0x20]
01C0:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
01C3:    DE F9                   fdivp    st(1)
01C5:    DD 1B                   fstp     qword ptr [ebx]
01C7:    D9 D0                   fnop
01C9:    DD 03                   fld      qword ptr [ebx]
01CB:    DC 07                   fadd     qword ptr [edi]
01CD:    D9 FC                   frndint    ; round(ST(0))
01CF:    D9 E0                   fchs    ; -ST(0)
01D1:    DC 07                   fadd     qword ptr [edi]
01D3:    DC 03                   fadd     qword ptr [ebx]
01D5:    DC 4F 08                fmul     qword ptr [edi + 8]
01D8:    DC 4F D0                fmul     qword ptr [edi - 0x30]
01DB:    DD 1B                   fstp     qword ptr [ebx]
01DD:    D9 D0                   fnop
01DF:    D9 D0                   fnop
01E1:    DD 01                   fld      qword ptr [ecx]
01E3:    DD 45 E0                fld      qword ptr [ebp - 0x20]
01E6:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
01E9:    DE F9                   fdivp    st(1)
01EB:    DD 19                   fstp     qword ptr [ecx]
01ED:    D9 D0                   fnop
01EF:    DD 01                   fld      qword ptr [ecx]
01F1:    DC 07                   fadd     qword ptr [edi]
01F3:    D9 FC                   frndint    ; round(ST(0))
01F5:    D9 E0                   fchs    ; -ST(0)
01F7:    DC 07                   fadd     qword ptr [edi]
01F9:    DC 01                   fadd     qword ptr [ecx]
01FB:    DC 4F 08                fmul     qword ptr [edi + 8]
01FE:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0201:    DD 19                   fstp     qword ptr [ecx]
0203:    D9 D0                   fnop
0205:    D9 D0                   fnop
0207:    E9 C7 00 00 00          jmp      0x2d3
020C:    90                      nop
020D:    90                      nop
020E:    90                      nop
020F:    90                      nop
0210:    90                      nop
0211:    90                      nop
0212:    90                      nop
0213:    90                      nop
0214:    90                      nop
0215:    90                      nop
0216:    90                      nop
0217:    DD 45 F8                fld      qword ptr [ebp - 8]
021A:    DC 31                   fdiv     qword ptr [ecx]
021C:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
021F:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0222:    DC 45 D8                fadd     qword ptr [ebp - 0x28]
0225:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0228:    DC 0F                   fmul     qword ptr [edi]
022A:    D9 E0                   fchs    ; -ST(0)
022C:    DC 01                   fadd     qword ptr [ecx]
022E:    DC 4F 08                fmul     qword ptr [edi + 8]
0231:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0234:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0237:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
023A:    DE F9                   fdivp    st(1)
023C:    DD 19                   fstp     qword ptr [ecx]
023E:    D9 D0                   fnop
0240:    DD 02                   fld      qword ptr [edx]
0242:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0245:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
0248:    DE F9                   fdivp    st(1)
024A:    DD 1A                   fstp     qword ptr [edx]
024C:    D9 D0                   fnop
024E:    DD 02                   fld      qword ptr [edx]
0250:    DC 07                   fadd     qword ptr [edi]
0252:    D9 FC                   frndint    ; round(ST(0))
0254:    D9 E0                   fchs    ; -ST(0)
0256:    DC 07                   fadd     qword ptr [edi]
0258:    DC 02                   fadd     qword ptr [edx]
025A:    DC 4F 08                fmul     qword ptr [edi + 8]
025D:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0260:    DD 1A                   fstp     qword ptr [edx]
0262:    D9 D0                   fnop
0264:    D9 D0                   fnop
0266:    DD 03                   fld      qword ptr [ebx]
0268:    DD 45 E0                fld      qword ptr [ebp - 0x20]
026B:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
026E:    DE F9                   fdivp    st(1)
0270:    DD 1B                   fstp     qword ptr [ebx]
0272:    D9 D0                   fnop
0274:    DD 03                   fld      qword ptr [ebx]
0276:    DC 07                   fadd     qword ptr [edi]
0278:    D9 FC                   frndint    ; round(ST(0))
027A:    D9 E0                   fchs    ; -ST(0)
027C:    DC 07                   fadd     qword ptr [edi]
027E:    DC 03                   fadd     qword ptr [ebx]
0280:    DC 4F 08                fmul     qword ptr [edi + 8]
0283:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0286:    DD 1B                   fstp     qword ptr [ebx]
0288:    D9 D0                   fnop
028A:    D9 D0                   fnop
028C:    EB 45                   jmp      0x2d3
028E:    90                      nop
028F:    90                      nop
0290:    90                      nop
0291:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0294:    90                      nop
0295:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0298:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
029B:    90                      nop
029C:    DD 45 E0                fld      qword ptr [ebp - 0x20]
029F:    DE E1                   fsubrp   st(1)
02A1:    DC 0F                   fmul     qword ptr [edi]
02A3:    DC 45 E0                fadd     qword ptr [ebp - 0x20]
02A6:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
02A9:    90                      nop
02AA:    83 7D E8 00             cmp      dword ptr [ebp - 0x18], 0
02AE:    75 17                   jne      0x2c7    ; Conditional jump
02B0:    8B 47 C0                mov      eax, dword ptr [edi - 0x40]
02B3:    3B 45 D4                cmp      eax, dword ptr [ebp - 0x2c]
02B6:    7E 1B                   jle      0x2d3    ; Conditional jump
02B8:    90                      nop
02B9:    90                      nop
02BA:    90                      nop
02BB:    90                      nop
02BC:    DD 45 E0                fld      qword ptr [ebp - 0x20]
02BF:    DC 65 D8                fsub     qword ptr [ebp - 0x28]
02C2:    DD 47 20                fld      qword ptr [edi + 0x20]
02C5:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
02C7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
02C9:    80 E4 41                and      ah, 0x41
02CC:    0F 8F B4 FD FF FF       jg       0x86    ; Conditional jump
02D2:    90                      nop
02D3:    DD 03                   fld      qword ptr [ebx]
02D5:    DC 4F D8                fmul     qword ptr [edi - 0x28]
02D8:    DC 67 F0                fsub     qword ptr [edi - 0x10]
02DB:    DD 1B                   fstp     qword ptr [ebx]
02DD:    DD 02                   fld      qword ptr [edx]
02DF:    DC 4F D8                fmul     qword ptr [edi - 0x28]
02E2:    DC 67 E8                fsub     qword ptr [edi - 0x18]
02E5:    DD 1A                   fstp     qword ptr [edx]
02E7:    DD 01                   fld      qword ptr [ecx]
02E9:    DC 4F D8                fmul     qword ptr [edi - 0x28]
02EC:    DC 67 E0                fsub     qword ptr [edi - 0x20]
02EF:    DD 19                   fstp     qword ptr [ecx]
02F1:    D9 D0                   fnop
02F3:    D9 D0                   fnop
02F5:    8B C3                   mov      eax, ebx
02F7:    5F                      pop      edi
02F8:    5E                      pop      esi
02F9:    5B                      pop      ebx
02FA:    8B E5                   mov      esp, ebp
02FC:    5D                      pop      ebp
02FD:    C2 08 00                ret      8
