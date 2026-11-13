; Disassembly: ABoxVSShapes.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 1450 bytes, decoded: 1450 bytes (100.0%)
; DEoption: 11

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 50 00 00 00       sub      esp, 0x50
0009:    56                      push     esi
000A:    57                      push     edi
000B:    53                      push     ebx
000C:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
000F:    89 C3                   mov      ebx, eax
0011:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
0014:    81 C7 80 00 00 00       add      edi, 0x80
001A:    83 7F 50 00             cmp      dword ptr [edi + 0x50], 0
001E:    7F 09                   jg       0x29    ; Conditional jump
0020:    FF 47 50                inc      dword ptr [edi + 0x50]
0023:    DD 46 F0                fld      qword ptr [esi - 0x10]
0026:    DD 5F 48                fstp     qword ptr [edi + 0x48]
0029:    DD 47 48                fld      qword ptr [edi + 0x48]
002C:    D9 E8                   fld1    ; Load 1.0
002E:    D9 C1                   fld      st(1)
0030:    D9 E1                   fabs    ; |ST(0)|
0032:    DE E1                   fsubrp   st(1)
0034:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0037:    DE C1                   faddp    st(1)
0039:    DD 5F 48                fstp     qword ptr [edi + 0x48]
003C:    DD 46 D8                fld      qword ptr [esi - 0x28]
003F:    D9 C0                   fld      st(0)
0041:    D9 C0                   fld      st(0)
0043:    DC C2                   fadd     st(2), st(0)
0045:    D9 E0                   fchs    ; -ST(0)
0047:    DD 03                   fld      qword ptr [ebx]
0049:    D8 D2                   fcom     st(2)
004B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
004F:    72 06                   jb       0x57    ; Conditional jump
0051:    D9 E0                   fchs    ; -ST(0)
0053:    D8 C3                   fadd     st(3)
0055:    EB 0C                   jmp      0x63
0057:    D8 D1                   fcom     st(1)
0059:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005D:    73 04                   jae      0x63    ; Conditional jump
005F:    D9 E0                   fchs    ; -ST(0)
0061:    D8 E3                   fsub     st(3)
0063:    DD 02                   fld      qword ptr [edx]
0065:    D8 D3                   fcom     st(3)
0067:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0069:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006B:    72 06                   jb       0x73    ; Conditional jump
006D:    D9 E0                   fchs    ; -ST(0)
006F:    D8 C4                   fadd     st(4)
0071:    EB 0C                   jmp      0x7f
0073:    D8 D2                   fcom     st(2)
0075:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0077:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0079:    73 04                   jae      0x7f    ; Conditional jump
007B:    D9 E0                   fchs    ; -ST(0)
007D:    D8 E4                   fsub     st(4)
007F:    DD 01                   fld      qword ptr [ecx]
0081:    D8 D4                   fcom     st(4)
0083:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0085:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0087:    72 06                   jb       0x8f    ; Conditional jump
0089:    D9 E0                   fchs    ; -ST(0)
008B:    D8 C5                   fadd     st(5)
008D:    EB 0C                   jmp      0x9b
008F:    D8 D3                   fcom     st(3)
0091:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0093:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0095:    73 04                   jae      0x9b    ; Conditional jump
0097:    D9 E0                   fchs    ; -ST(0)
0099:    D8 E5                   fsub     st(5)
009B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
009D:    DD D8                   fstp     st(0)
009F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A1:    DD D8                   fstp     st(0)
00A3:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A5:    DD D8                   fstp     st(0)
00A7:    90                      nop
00A8:    83 7E CC 01             cmp      dword ptr [esi - 0x34], 1
00AC:    75 1F                   jne      0xcd    ; Conditional jump
00AE:    D9 C0                   fld      st(0)
00B0:    D8 C8                   fmul     st(0)
00B2:    D9 C2                   fld      st(2)
00B4:    D8 C8                   fmul     st(0)
00B6:    DE C1                   faddp    st(1)
00B8:    D9 FA                   fsqrt    ; sqrt(ST(0))
00BA:    DC 66 C4                fsub     qword ptr [esi - 0x3c]
00BD:    D8 C8                   fmul     st(0)
00BF:    D9 C3                   fld      st(3)
00C1:    D8 C8                   fmul     st(0)
00C3:    DE C1                   faddp    st(1)
00C5:    DC 4E BC                fmul     qword ptr [esi - 0x44]
00C8:    E9 74 03 00 00          jmp      0x441
00CD:    90                      nop
00CE:    83 7E CC 02             cmp      dword ptr [esi - 0x34], 2
00D2:    75 41                   jne      0x115    ; Conditional jump
00D4:    D9 C2                   fld      st(2)
00D6:    D9 E1                   fabs    ; |ST(0)|
00D8:    DD 46 C4                fld      qword ptr [esi - 0x3c]
00DB:    D8 D1                   fcom     st(1)
00DD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DF:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00E1:    76 2A                   jbe      0x10d    ; Conditional jump
00E3:    DE E1                   fsubrp   st(1)
00E5:    D8 C8                   fmul     st(0)
00E7:    90                      nop
00E8:    D9 C1                   fld      st(1)
00EA:    D8 C8                   fmul     st(0)
00EC:    D9 C3                   fld      st(3)
00EE:    D8 C8                   fmul     st(0)
00F0:    DE C1                   faddp    st(1)
00F2:    DC 4E BC                fmul     qword ptr [esi - 0x44]
00F5:    90                      nop
00F6:    90                      nop
00F7:    D8 D1                   fcom     st(1)
00F9:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00FB:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00FD:    73 07                   jae      0x106    ; Conditional jump
00FF:    DD D8                   fstp     st(0)
0101:    E9 3B 03 00 00          jmp      0x441
0106:    DD D9                   fstp     st(1)
0108:    E9 34 03 00 00          jmp      0x441
010D:    DD D8                   fstp     st(0)
010F:    DD D8                   fstp     st(0)
0111:    D9 EE                   fldz    ; Load 0.0
0113:    EB D3                   jmp      0xe8
0115:    90                      nop
0116:    83 7E CC 03             cmp      dword ptr [esi - 0x34], 3
011A:    75 41                   jne      0x15d    ; Conditional jump
011C:    D9 C2                   fld      st(2)
011E:    D9 E1                   fabs    ; |ST(0)|
0120:    DD 46 C4                fld      qword ptr [esi - 0x3c]
0123:    D8 D1                   fcom     st(1)
0125:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0127:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0129:    76 2A                   jbe      0x155    ; Conditional jump
012B:    DE E1                   fsubrp   st(1)
012D:    D8 C8                   fmul     st(0)
012F:    90                      nop
0130:    D9 C1                   fld      st(1)
0132:    D8 C8                   fmul     st(0)
0134:    D9 C3                   fld      st(3)
0136:    D8 C8                   fmul     st(0)
0138:    DE C1                   faddp    st(1)
013A:    DC 4E BC                fmul     qword ptr [esi - 0x44]
013D:    90                      nop
013E:    90                      nop
013F:    D8 D1                   fcom     st(1)
0141:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0143:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0145:    76 07                   jbe      0x14e    ; Conditional jump
0147:    DD D8                   fstp     st(0)
0149:    E9 F3 02 00 00          jmp      0x441
014E:    DD D9                   fstp     st(1)
0150:    E9 EC 02 00 00          jmp      0x441
0155:    DD D8                   fstp     st(0)
0157:    DD D8                   fstp     st(0)
0159:    D9 EE                   fldz    ; Load 0.0
015B:    EB D3                   jmp      0x130
015D:    90                      nop
015E:    83 7E CC 04             cmp      dword ptr [esi - 0x34], 4
0162:    75 41                   jne      0x1a5    ; Conditional jump
0164:    D9 C2                   fld      st(2)
0166:    D9 E1                   fabs    ; |ST(0)|
0168:    DD 46 C4                fld      qword ptr [esi - 0x3c]
016B:    D8 D1                   fcom     st(1)
016D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
016F:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0171:    76 2A                   jbe      0x19d    ; Conditional jump
0173:    DE E1                   fsubrp   st(1)
0175:    90                      nop
0176:    90                      nop
0177:    90                      nop
0178:    D9 C1                   fld      st(1)
017A:    D8 C8                   fmul     st(0)
017C:    D9 C3                   fld      st(3)
017E:    D8 C8                   fmul     st(0)
0180:    DE C1                   faddp    st(1)
0182:    DC 4E BC                fmul     qword ptr [esi - 0x44]
0185:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0187:    D8 D1                   fcom     st(1)
0189:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
018B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
018D:    73 07                   jae      0x196    ; Conditional jump
018F:    DD D8                   fstp     st(0)
0191:    E9 AB 02 00 00          jmp      0x441
0196:    DE F9                   fdivp    st(1)
0198:    E9 A4 02 00 00          jmp      0x441
019D:    DD D8                   fstp     st(0)
019F:    DD D8                   fstp     st(0)
01A1:    D9 E8                   fld1    ; Load 1.0
01A3:    EB D3                   jmp      0x178
01A5:    90                      nop
01A6:    83 7E CC 05             cmp      dword ptr [esi - 0x34], 5
01AA:    75 41                   jne      0x1ed    ; Conditional jump
01AC:    D9 C2                   fld      st(2)
01AE:    D9 E1                   fabs    ; |ST(0)|
01B0:    DD 46 C4                fld      qword ptr [esi - 0x3c]
01B3:    D8 D1                   fcom     st(1)
01B5:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01B7:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01B9:    76 2A                   jbe      0x1e5    ; Conditional jump
01BB:    DE E1                   fsubrp   st(1)
01BD:    90                      nop
01BE:    90                      nop
01BF:    90                      nop
01C0:    D9 C1                   fld      st(1)
01C2:    D8 C8                   fmul     st(0)
01C4:    D9 C3                   fld      st(3)
01C6:    D8 C8                   fmul     st(0)
01C8:    DE C1                   faddp    st(1)
01CA:    DC 4E BC                fmul     qword ptr [esi - 0x44]
01CD:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
01CF:    D8 D1                   fcom     st(1)
01D1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01D3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01D5:    76 07                   jbe      0x1de    ; Conditional jump
01D7:    DD D8                   fstp     st(0)
01D9:    E9 63 02 00 00          jmp      0x441
01DE:    DE F9                   fdivp    st(1)
01E0:    E9 5C 02 00 00          jmp      0x441
01E5:    DD D8                   fstp     st(0)
01E7:    DD D8                   fstp     st(0)
01E9:    D9 E8                   fld1    ; Load 1.0
01EB:    EB D3                   jmp      0x1c0
01ED:    90                      nop
01EE:    83 7E CC 06             cmp      dword ptr [esi - 0x34], 6
01F2:    75 4A                   jne      0x23e    ; Conditional jump
01F4:    D9 C2                   fld      st(2)
01F6:    D8 C8                   fmul     st(0)
01F8:    DD 46 C4                fld      qword ptr [esi - 0x3c]
01FB:    D8 D1                   fcom     st(1)
01FD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01FF:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0201:    76 33                   jbe      0x236    ; Conditional jump
0203:    DE E1                   fsubrp   st(1)
0205:    D9 C1                   fld      st(1)
0207:    D8 C8                   fmul     st(0)
0209:    D9 C3                   fld      st(3)
020B:    D8 C8                   fmul     st(0)
020D:    D8 D1                   fcom     st(1)
020F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0211:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0213:    73 04                   jae      0x219    ; Conditional jump
0215:    DD D8                   fstp     st(0)
0217:    EB 02                   jmp      0x21b
0219:    DD D9                   fstp     st(1)
021B:    DC 4E BC                fmul     qword ptr [esi - 0x44]
021E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0220:    D8 D1                   fcom     st(1)
0222:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0224:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0226:    73 07                   jae      0x22f    ; Conditional jump
0228:    DD D8                   fstp     st(0)
022A:    E9 12 02 00 00          jmp      0x441
022F:    DE F9                   fdivp    st(1)
0231:    E9 0B 02 00 00          jmp      0x441
0236:    DD D8                   fstp     st(0)
0238:    DD D8                   fstp     st(0)
023A:    D9 E8                   fld1    ; Load 1.0
023C:    EB C7                   jmp      0x205
023E:    90                      nop
023F:    83 7E CC 07             cmp      dword ptr [esi - 0x34], 7
0243:    75 4A                   jne      0x28f    ; Conditional jump
0245:    D9 C2                   fld      st(2)
0247:    D8 C8                   fmul     st(0)
0249:    DD 46 C4                fld      qword ptr [esi - 0x3c]
024C:    D8 D1                   fcom     st(1)
024E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0250:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0252:    76 33                   jbe      0x287    ; Conditional jump
0254:    DE E1                   fsubrp   st(1)
0256:    D9 C1                   fld      st(1)
0258:    D8 C8                   fmul     st(0)
025A:    D9 C3                   fld      st(3)
025C:    D8 C8                   fmul     st(0)
025E:    D8 D1                   fcom     st(1)
0260:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0262:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0264:    73 04                   jae      0x26a    ; Conditional jump
0266:    DD D8                   fstp     st(0)
0268:    EB 02                   jmp      0x26c
026A:    DD D9                   fstp     st(1)
026C:    DC 4E BC                fmul     qword ptr [esi - 0x44]
026F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0271:    D8 D1                   fcom     st(1)
0273:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0275:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0277:    76 07                   jbe      0x280    ; Conditional jump
0279:    DD D8                   fstp     st(0)
027B:    E9 C1 01 00 00          jmp      0x441
0280:    DE F9                   fdivp    st(1)
0282:    E9 BA 01 00 00          jmp      0x441
0287:    DD D8                   fstp     st(0)
0289:    DD D8                   fstp     st(0)
028B:    D9 E8                   fld1    ; Load 1.0
028D:    EB C7                   jmp      0x256
028F:    90                      nop
0290:    83 7E CC 08             cmp      dword ptr [esi - 0x34], 8
0294:    75 20                   jne      0x2b6    ; Conditional jump
0296:    D9 C2                   fld      st(2)
0298:    D8 C8                   fmul     st(0)
029A:    D9 C0                   fld      st(0)
029C:    DC 66 C4                fsub     qword ptr [esi - 0x3c]
029F:    DE C9                   fmulp    st(1)
02A1:    D9 C1                   fld      st(1)
02A3:    D8 C8                   fmul     st(0)
02A5:    D9 C3                   fld      st(3)
02A7:    D8 C8                   fmul     st(0)
02A9:    DE C1                   faddp    st(1)
02AB:    DE C1                   faddp    st(1)
02AD:    DC 4E BC                fmul     qword ptr [esi - 0x44]
02B0:    E9 8C 01 00 00          jmp      0x441
02B5:    90                      nop
02B6:    83 7E CC 0A             cmp      dword ptr [esi - 0x34], 0xa
02BA:    75 21                   jne      0x2dd    ; Conditional jump
02BC:    D9 C2                   fld      st(2)
02BE:    D8 C8                   fmul     st(0)
02C0:    D8 C8                   fmul     st(0)
02C2:    D9 C2                   fld      st(2)
02C4:    D8 C8                   fmul     st(0)
02C6:    D8 C8                   fmul     st(0)
02C8:    DE C1                   faddp    st(1)
02CA:    D9 C1                   fld      st(1)
02CC:    D8 C8                   fmul     st(0)
02CE:    D8 C8                   fmul     st(0)
02D0:    DE C1                   faddp    st(1)
02D2:    DC 4E BC                fmul     qword ptr [esi - 0x44]
02D5:    DC 66 C4                fsub     qword ptr [esi - 0x3c]
02D8:    E9 64 01 00 00          jmp      0x441
02DD:    90                      nop
02DE:    83 7E CC 0B             cmp      dword ptr [esi - 0x34], 0xb
02E2:    75 5F                   jne      0x343    ; Conditional jump
02E4:    D9 C2                   fld      st(2)
02E6:    D8 C8                   fmul     st(0)
02E8:    DD 5D F8                fstp     qword ptr [ebp - 8]
02EB:    D9 C1                   fld      st(1)
02ED:    D8 C8                   fmul     st(0)
02EF:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
02F2:    D9 C0                   fld      st(0)
02F4:    D8 C8                   fmul     st(0)
02F6:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
02F9:    D9 C2                   fld      st(2)
02FB:    D8 CA                   fmul     st(2)
02FD:    D8 C9                   fmul     st(1)
02FF:    DC 0E                   fmul     qword ptr [esi]
0301:    DD 45 F8                fld      qword ptr [ebp - 8]
0304:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0307:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
030A:    DE C1                   faddp    st(1)
030C:    DD 45 F8                fld      qword ptr [ebp - 8]
030F:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0312:    DD 45 F8                fld      qword ptr [ebp - 8]
0315:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0318:    DE C1                   faddp    st(1)
031A:    DD 45 E8                fld      qword ptr [ebp - 0x18]
031D:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0320:    DE C1                   faddp    st(1)
0322:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
0325:    DE C1                   faddp    st(1)
0327:    DD 45 F8                fld      qword ptr [ebp - 8]
032A:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
032D:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
0330:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
0333:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
0336:    DE C1                   faddp    st(1)
0338:    DC 46 BC                fadd     qword ptr [esi - 0x44]
033B:    DC 4E 08                fmul     qword ptr [esi + 8]
033E:    E9 FE 00 00 00          jmp      0x441
0343:    90                      nop
0344:    83 7E CC 09             cmp      dword ptr [esi - 0x34], 9
0348:    75 37                   jne      0x381    ; Conditional jump
034A:    D9 C2                   fld      st(2)
034C:    D8 C8                   fmul     st(0)
034E:    D8 C8                   fmul     st(0)
0350:    D9 C2                   fld      st(2)
0352:    D8 C8                   fmul     st(0)
0354:    D8 C8                   fmul     st(0)
0356:    DE C1                   faddp    st(1)
0358:    D9 C1                   fld      st(1)
035A:    D8 C8                   fmul     st(0)
035C:    D8 C8                   fmul     st(0)
035E:    DE C1                   faddp    st(1)
0360:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0363:    D9 C2                   fld      st(2)
0365:    D8 C8                   fmul     st(0)
0367:    D9 C2                   fld      st(2)
0369:    D8 C8                   fmul     st(0)
036B:    DE C1                   faddp    st(1)
036D:    D9 C1                   fld      st(1)
036F:    D8 C8                   fmul     st(0)
0371:    DE C1                   faddp    st(1)
0373:    DC 4E BC                fmul     qword ptr [esi - 0x44]
0376:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
0379:    DC 46 C4                fadd     qword ptr [esi - 0x3c]
037C:    E9 C0 00 00 00          jmp      0x441
0381:    90                      nop
0382:    83 7E CC 0C             cmp      dword ptr [esi - 0x34], 0xc
0386:    75 2D                   jne      0x3b5    ; Conditional jump
0388:    D9 C2                   fld      st(2)
038A:    D9 E0                   fchs    ; -ST(0)
038C:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
038F:    D9 C1                   fld      st(1)
0391:    D8 C8                   fmul     st(0)
0393:    D9 C3                   fld      st(3)
0395:    D8 C8                   fmul     st(0)
0397:    DE C1                   faddp    st(1)
0399:    D9 FA                   fsqrt    ; sqrt(ST(0))
039B:    DC 4E BC                fmul     qword ptr [esi - 0x44]
039E:    DE C1                   faddp    st(1)
03A0:    D9 EE                   fldz    ; Load 0.0
03A2:    D8 D9                   fcomp    st(1)
03A4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
03A6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
03A8:    76 04                   jbe      0x3ae    ; Conditional jump
03AA:    DD D8                   fstp     st(0)
03AC:    D9 EE                   fldz    ; Load 0.0
03AE:    D8 C8                   fmul     st(0)
03B0:    E9 8C 00 00 00          jmp      0x441
03B5:    90                      nop
03B6:    83 7E CC 0D             cmp      dword ptr [esi - 0x34], 0xd
03BA:    75 2B                   jne      0x3e7    ; Conditional jump
03BC:    D9 C2                   fld      st(2)
03BE:    D9 E0                   fchs    ; -ST(0)
03C0:    D9 E8                   fld1    ; Load 1.0
03C2:    D8 D9                   fcomp    st(1)
03C4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
03C6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
03C8:    73 06                   jae      0x3d0    ; Conditional jump
03CA:    DD D8                   fstp     st(0)
03CC:    D9 E8                   fld1    ; Load 1.0
03CE:    90                      nop
03CF:    90                      nop
03D0:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
03D3:    D9 C1                   fld      st(1)
03D5:    D8 C8                   fmul     st(0)
03D7:    D9 C3                   fld      st(3)
03D9:    D8 C8                   fmul     st(0)
03DB:    DE C1                   faddp    st(1)
03DD:    DE C1                   faddp    st(1)
03DF:    DC 4E BC                fmul     qword ptr [esi - 0x44]
03E2:    E9 5A 00 00 00          jmp      0x441
03E7:    90                      nop
03E8:    83 7E CC 0E             cmp      dword ptr [esi - 0x34], 0xe
03EC:    75 38                   jne      0x426    ; Conditional jump
03EE:    D9 C2                   fld      st(2)
03F0:    D8 C8                   fmul     st(0)
03F2:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
03F5:    D9 C2                   fld      st(2)
03F7:    D8 C8                   fmul     st(0)
03F9:    DE C1                   faddp    st(1)
03FB:    90                      nop
03FC:    D9 C3                   fld      st(3)
03FE:    D8 C8                   fmul     st(0)
0400:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
0403:    D9 C2                   fld      st(2)
0405:    D8 C8                   fmul     st(0)
0407:    DE C1                   faddp    st(1)
0409:    90                      nop
040A:    D8 D1                   fcom     st(1)
040C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
040E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0410:    73 0A                   jae      0x41c    ; Conditional jump
0412:    DD D8                   fstp     st(0)
0414:    DC 4E BC                fmul     qword ptr [esi - 0x44]
0417:    E9 25 00 00 00          jmp      0x441
041C:    DD D9                   fstp     st(1)
041E:    DC 4E BC                fmul     qword ptr [esi - 0x44]
0421:    E9 1B 00 00 00          jmp      0x441
0426:    90                      nop
0427:    E9 63 00 00 00          jmp      0x48f
042C:    90                      nop
042D:    90                      nop
042E:    90                      nop
042F:    90                      nop
0430:    90                      nop
0431:    90                      nop
0432:    90                      nop
0433:    90                      nop
0434:    90                      nop
0435:    90                      nop
0436:    90                      nop
0437:    90                      nop
0438:    90                      nop
0439:    90                      nop
043A:    90                      nop
043B:    90                      nop
043C:    90                      nop
043D:    90                      nop
043E:    90                      nop
043F:    90                      nop
0440:    90                      nop
0441:    DC 56 E0                fcom     qword ptr [esi - 0x20]
0444:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0446:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0448:    73 07                   jae      0x451    ; Conditional jump
044A:    DD D8                   fstp     st(0)
044C:    DD 46 E8                fld      qword ptr [esi - 0x18]
044F:    EB 16                   jmp      0x467
0451:    D9 E8                   fld1    ; Load 1.0
0453:    D8 D1                   fcom     st(1)
0455:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0457:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0459:    72 07                   jb       0x462    ; Conditional jump
045B:    DD D8                   fstp     st(0)
045D:    DC 7F 48                fdivr    qword ptr [edi + 0x48]
0460:    EB 05                   jmp      0x467
0462:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0464:    DD 47 48                fld      qword ptr [edi + 0x48]
0467:    DD 41 08                fld      qword ptr [ecx + 8]
046A:    D8 C9                   fmul     st(1)
046C:    DD 59 08                fstp     qword ptr [ecx + 8]
046F:    DC CB                   fmul     st(3), st(0)
0471:    DC CA                   fmul     st(2), st(0)
0473:    DE C9                   fmulp    st(1)
0475:    DC 47 A8                fadd     qword ptr [edi - 0x58]
0478:    DD 19                   fstp     qword ptr [ecx]
047A:    DC 47 A0                fadd     qword ptr [edi - 0x60]
047D:    DD 1A                   fstp     qword ptr [edx]
047F:    DC 47 98                fadd     qword ptr [edi - 0x68]
0482:    DD 1B                   fstp     qword ptr [ebx]
0484:    89 D8                   mov      eax, ebx
0486:    5B                      pop      ebx
0487:    5F                      pop      edi
0488:    5E                      pop      esi
0489:    89 EC                   mov      esp, ebp
048B:    5D                      pop      ebp
048C:    C2 08 00                ret      8
048F:    90                      nop
0490:    83 7E CC 0F             cmp      dword ptr [esi - 0x34], 0xf
0494:    75 2C                   jne      0x4c2    ; Conditional jump
0496:    D9 C0                   fld      st(0)
0498:    D8 C8                   fmul     st(0)
049A:    D9 C2                   fld      st(2)
049C:    D8 C8                   fmul     st(0)
049E:    DE C1                   faddp    st(1)
04A0:    DC 4E BC                fmul     qword ptr [esi - 0x44]
04A3:    D9 C3                   fld      st(3)
04A5:    D9 E1                   fabs    ; |ST(0)|
04A7:    DC 56 C4                fcom     qword ptr [esi - 0x3c]
04AA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
04AC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
04AE:    73 06                   jae      0x4b6    ; Conditional jump
04B0:    DD D8                   fstp     st(0)
04B2:    D9 EE                   fldz    ; Load 0.0
04B4:    EB 05                   jmp      0x4bb
04B6:    DC 66 C4                fsub     qword ptr [esi - 0x3c]
04B9:    D8 C8                   fmul     st(0)
04BB:    DE C1                   faddp    st(1)
04BD:    E9 7F FF FF FF          jmp      0x441
04C2:    90                      nop
04C3:    83 7E CC 10             cmp      dword ptr [esi - 0x34], 0x10
04C7:    0F 85 A9 00 00 00       jne      0x576    ; Conditional jump
04CD:    D9 C0                   fld      st(0)
04CF:    D8 C8                   fmul     st(0)
04D1:    D9 C2                   fld      st(2)
04D3:    DC 66 C4                fsub     qword ptr [esi - 0x3c]
04D6:    D8 C8                   fmul     st(0)
04D8:    DE C1                   faddp    st(1)
04DA:    D9 C3                   fld      st(3)
04DC:    DC 66 C4                fsub     qword ptr [esi - 0x3c]
04DF:    D8 C8                   fmul     st(0)
04E1:    DE C1                   faddp    st(1)
04E3:    DD 5D F8                fstp     qword ptr [ebp - 8]
04E6:    90                      nop
04E7:    D9 C0                   fld      st(0)
04E9:    DC 66 C4                fsub     qword ptr [esi - 0x3c]
04EC:    D8 C8                   fmul     st(0)
04EE:    D9 C2                   fld      st(2)
04F0:    D8 C8                   fmul     st(0)
04F2:    DE C1                   faddp    st(1)
04F4:    D9 C3                   fld      st(3)
04F6:    DC 66 C4                fsub     qword ptr [esi - 0x3c]
04F9:    D8 C8                   fmul     st(0)
04FB:    DE C1                   faddp    st(1)
04FD:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0500:    90                      nop
0501:    D9 C0                   fld      st(0)
0503:    DC 66 C4                fsub     qword ptr [esi - 0x3c]
0506:    D8 C8                   fmul     st(0)
0508:    D9 C2                   fld      st(2)
050A:    DC 66 C4                fsub     qword ptr [esi - 0x3c]
050D:    D8 C8                   fmul     st(0)
050F:    DE C1                   faddp    st(1)
0511:    D9 C3                   fld      st(3)
0513:    D8 C8                   fmul     st(0)
0515:    DE C1                   faddp    st(1)
0517:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
051A:    90                      nop
051B:    D9 C0                   fld      st(0)
051D:    DC 46 C4                fadd     qword ptr [esi - 0x3c]
0520:    D8 C8                   fmul     st(0)
0522:    D9 C2                   fld      st(2)
0524:    DC 46 C4                fadd     qword ptr [esi - 0x3c]
0527:    D8 C8                   fmul     st(0)
0529:    DE C1                   faddp    st(1)
052B:    D9 C3                   fld      st(3)
052D:    DC 46 C4                fadd     qword ptr [esi - 0x3c]
0530:    D8 C8                   fmul     st(0)
0532:    DE C1                   faddp    st(1)
0534:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0537:    90                      nop
0538:    DD 45 F8                fld      qword ptr [ebp - 8]
053B:    DD 45 F0                fld      qword ptr [ebp - 0x10]
053E:    D8 D1                   fcom     st(1)
0540:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0542:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0544:    73 04                   jae      0x54a    ; Conditional jump
0546:    DD D8                   fstp     st(0)
0548:    EB 02                   jmp      0x54c
054A:    DD D9                   fstp     st(1)
054C:    DD 45 E8                fld      qword ptr [ebp - 0x18]
054F:    D8 D1                   fcom     st(1)
0551:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0553:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0555:    73 04                   jae      0x55b    ; Conditional jump
0557:    DD D8                   fstp     st(0)
0559:    EB 02                   jmp      0x55d
055B:    DD D9                   fstp     st(1)
055D:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0560:    D8 D1                   fcom     st(1)
0562:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0564:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0566:    73 04                   jae      0x56c    ; Conditional jump
0568:    DD D8                   fstp     st(0)
056A:    EB 02                   jmp      0x56e
056C:    DD D9                   fstp     st(1)
056E:    DC 4E BC                fmul     qword ptr [esi - 0x44]
0571:    E9 CB FE FF FF          jmp      0x441
0576:    83 7E CC 11             cmp      dword ptr [esi - 0x34], 0x11
057A:    75 29                   jne      0x5a5    ; Conditional jump
057C:    D9 C2                   fld      st(2)
057E:    D9 E1                   fabs    ; |ST(0)|
0580:    D8 C0                   fadd     st(0)
0582:    D8 C0                   fadd     st(0)
0584:    D9 C3                   fld      st(3)
0586:    D8 C8                   fmul     st(0)
0588:    D8 C0                   fadd     st(0)
058A:    D9 C3                   fld      st(3)
058C:    D8 C8                   fmul     st(0)
058E:    DE C1                   faddp    st(1)
0590:    D9 C2                   fld      st(2)
0592:    D8 C8                   fmul     st(0)
0594:    DE C1                   faddp    st(1)
0596:    DE F1                   fdivrp   st(1)
0598:    D8 C8                   fmul     st(0)
059A:    DC 4E BC                fmul     qword ptr [esi - 0x44]
059D:    DC 46 C4                fadd     qword ptr [esi - 0x3c]
05A0:    E9 9C FE FF FF          jmp      0x441
05A5:    E9 97 FE FF FF          jmp      0x441
