; Disassembly: RandCubesIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 1122 bytes, decoded: 1122 bytes (100.0%)
; DEoption: 20

0000:    83 C4 C4                add      esp, -0x3c
0003:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0008:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000D:    66 0F 59 47 E0          mulpd    xmm0, xmmword ptr [edi - 0x20]
0012:    F2 0F 59 4F E0          mulsd    xmm1, qword ptr [edi - 0x20]
0017:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
001C:    F2 0F 58 4F D0          addsd    xmm1, qword ptr [edi - 0x30]
0021:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0025:    74 19                   je       0x40    ; Conditional jump
0027:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002C:    F2 0F 59 6F E0          mulsd    xmm5, qword ptr [edi - 0x20]
0031:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0036:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
003B:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0040:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0045:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
004B:    DD 04 24                fld      qword ptr [esp]
004E:    DB 54 24 18             fist     dword ptr [esp + 0x18]
0052:    DA 64 24 18             fisub    dword ptr [esp + 0x18]
0056:    D9 E1                   fabs    ; |ST(0)|
0058:    DD 1C 24                fstp     qword ptr [esp]
005B:    DD 44 24 08             fld      qword ptr [esp + 8]
005F:    DB 54 24 1C             fist     dword ptr [esp + 0x1c]
0063:    DA 64 24 1C             fisub    dword ptr [esp + 0x1c]
0067:    D9 E1                   fabs    ; |ST(0)|
0069:    DD 5C 24 08             fstp     qword ptr [esp + 8]
006D:    8B 44 24 1C             mov      eax, dword ptr [esp + 0x1c]
0071:    35 41 B6 00 00          xor      eax, 0xb641
0076:    89 44 24 1C             mov      dword ptr [esp + 0x1c], eax
007A:    8B 54 24 18             mov      edx, dword ptr [esp + 0x18]
007E:    81 F2 81 9D 00 00       xor      edx, 0x9d81
0084:    89 54 24 18             mov      dword ptr [esp + 0x18], edx
0088:    F7 E2                   mul      edx
008A:    F7 D0                   not      eax
008C:    89 44 24 24             mov      dword ptr [esp + 0x24], eax
0090:    25 5A 5A 00 00          and      eax, 0x5a5a
0095:    81 CA 5A 5A 00 00       or       edx, 0x5a5a
009B:    31 D0                   xor      eax, edx
009D:    89 44 24 20             mov      dword ptr [esp + 0x20], eax
00A1:    8B 44 24 1C             mov      eax, dword ptr [esp + 0x1c]
00A5:    8B 54 24 18             mov      edx, dword ptr [esp + 0x18]
00A9:    81 E2 5A 5A 00 00       and      edx, 0x5a5a
00AF:    0D 5A 5A 00 00          or       eax, 0x5a5a
00B4:    31 D0                   xor      eax, edx
00B6:    89 44 24 18             mov      dword ptr [esp + 0x18], eax
00BA:    8B 44 24 20             mov      eax, dword ptr [esp + 0x20]
00BE:    89 44 24 1C             mov      dword ptr [esp + 0x1c], eax
00C2:    8B 44 24 1C             mov      eax, dword ptr [esp + 0x1c]
00C6:    2B 47 A4                sub      eax, dword ptr [edi - 0x5c]
00C9:    8B 54 24 1C             mov      edx, dword ptr [esp + 0x1c]
00CD:    F7 E2                   mul      edx
00CF:    03 44 24 24             add      eax, dword ptr [esp + 0x24]
00D3:    89 C2                   mov      edx, eax
00D5:    C1 E8 08                shr      eax, 8
00D8:    C1 E2 08                shl      edx, 8
00DB:    31 D0                   xor      eax, edx
00DD:    25 FF FF 00 00          and      eax, 0xffff
00E2:    89 44 24 1C             mov      dword ptr [esp + 0x1c], eax
00E6:    8B 44 24 18             mov      eax, dword ptr [esp + 0x18]
00EA:    2B 47 A8                sub      eax, dword ptr [edi - 0x58]
00ED:    8B 54 24 18             mov      edx, dword ptr [esp + 0x18]
00F1:    F7 E2                   mul      edx
00F3:    03 44 24 24             add      eax, dword ptr [esp + 0x24]
00F7:    89 C2                   mov      edx, eax
00F9:    C1 E8 08                shr      eax, 8
00FC:    C1 E2 08                shl      edx, 8
00FF:    31 D0                   xor      eax, edx
0101:    25 FF FF 00 00          and      eax, 0xffff
0106:    89 44 24 18             mov      dword ptr [esp + 0x18], eax
010A:    8B 44 24 18             mov      eax, dword ptr [esp + 0x18]
010E:    8B 54 24 1C             mov      edx, dword ptr [esp + 0x1c]
0112:    C1 E8 06                shr      eax, 6
0115:    C1 E2 06                shl      edx, 6
0118:    31 D0                   xor      eax, edx
011A:    25 FF FF 00 00          and      eax, 0xffff
011F:    89 44 24 24             mov      dword ptr [esp + 0x24], eax
0123:    8B 44 24 18             mov      eax, dword ptr [esp + 0x18]
0127:    8B 54 24 1C             mov      edx, dword ptr [esp + 0x1c]
012B:    25 AA AA 00 00          and      eax, 0xaaaa
0130:    81 E2 55 55 00 00       and      edx, 0x5555
0136:    31 D0                   xor      eax, edx
0138:    89 44 24 28             mov      dword ptr [esp + 0x28], eax
013C:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0140:    DB 44 24 28             fild     dword ptr [esp + 0x28]
0144:    DC 0F                   fmul     qword ptr [edi]
0146:    DC 4F 94                fmul     qword ptr [edi - 0x6c]
0149:    DE E9                   fsubp    st(1)
014B:    DD 54 24 10             fst      qword ptr [esp + 0x10]
014F:    D9 E1                   fabs    ; |ST(0)|
0151:    DB 44 24 24             fild     dword ptr [esp + 0x24]
0155:    DC 0F                   fmul     qword ptr [edi]
0157:    DC 4F AC                fmul     qword ptr [edi - 0x54]
015A:    DE C1                   faddp    st(1)
015C:    DB 44 24 24             fild     dword ptr [esp + 0x24]
0160:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0163:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0169:    8B 44 24 18             mov      eax, dword ptr [esp + 0x18]
016D:    83 E0 40                and      eax, 0x40
0170:    75 05                   jne      0x177    ; Conditional jump
0172:    8B 47 A0                mov      eax, dword ptr [edi - 0x60]
0175:    EB 03                   jmp      0x17a
0177:    8B 47 9C                mov      eax, dword ptr [edi - 0x64]
017A:    83 F8 00                cmp      eax, 0
017D:    0F 84 7E 02 00 00       je       0x401    ; Conditional jump
0183:    83 F8 01                cmp      eax, 1
0186:    75 3E                   jne      0x1c6    ; Conditional jump
0188:    DD 04 24                fld      qword ptr [esp]
018B:    D8 C8                   fmul     st(0)
018D:    DD 44 24 08             fld      qword ptr [esp + 8]
0191:    D8 C8                   fmul     st(0)
0193:    DE C1                   faddp    st(1)
0195:    D9 FA                   fsqrt    ; sqrt(ST(0))
0197:    DB 44 24 18             fild     dword ptr [esp + 0x18]
019B:    DC 0F                   fmul     qword ptr [edi]
019D:    DC 4F F0                fmul     qword ptr [edi - 0x10]
01A0:    DE C1                   faddp    st(1)
01A2:    D9 E8                   fld1    ; Load 1.0
01A4:    DC 67 F0                fsub     qword ptr [edi - 0x10]
01A7:    DC 4F 08                fmul     qword ptr [edi + 8]
01AA:    DE C1                   faddp    st(1)
01AC:    DB 44 24 1C             fild     dword ptr [esp + 0x1c]
01B0:    DC 0F                   fmul     qword ptr [edi]
01B2:    DC 4F B8                fmul     qword ptr [edi - 0x48]
01B5:    DD 44 24 10             fld      qword ptr [esp + 0x10]
01B9:    D9 E1                   fabs    ; |ST(0)|
01BB:    D9 E0                   fchs    ; -ST(0)
01BD:    DE C9                   fmulp    st(1)
01BF:    DE C1                   faddp    st(1)
01C1:    E9 7C 02 00 00          jmp      0x442
01C6:    83 F8 02                cmp      eax, 2
01C9:    75 3F                   jne      0x20a    ; Conditional jump
01CB:    DD 04 24                fld      qword ptr [esp]
01CE:    D8 C8                   fmul     st(0)
01D0:    DD 44 24 08             fld      qword ptr [esp + 8]
01D4:    D8 C8                   fmul     st(0)
01D6:    DE C1                   faddp    st(1)
01D8:    DD 44 24 10             fld      qword ptr [esp + 0x10]
01DC:    D8 C8                   fmul     st(0)
01DE:    DB 44 24 18             fild     dword ptr [esp + 0x18]
01E2:    DC 4F 18                fmul     qword ptr [edi + 0x18]
01E5:    DC 4F F0                fmul     qword ptr [edi - 0x10]
01E8:    DE C9                   fmulp    st(1)
01EA:    DE E9                   fsubp    st(1)
01EC:    D9 FA                   fsqrt    ; sqrt(ST(0))
01EE:    DB 44 24 1C             fild     dword ptr [esp + 0x1c]
01F2:    DC 0F                   fmul     qword ptr [edi]
01F4:    DD 47 B8                fld      qword ptr [edi - 0x48]
01F7:    DE C9                   fmulp    st(1)
01F9:    DE C1                   faddp    st(1)
01FB:    D9 E8                   fld1    ; Load 1.0
01FD:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0200:    DC 4F 08                fmul     qword ptr [edi + 8]
0203:    DE C1                   faddp    st(1)
0205:    E9 38 02 00 00          jmp      0x442
020A:    83 F8 03                cmp      eax, 3
020D:    75 4B                   jne      0x25a    ; Conditional jump
020F:    DD 04 24                fld      qword ptr [esp]
0212:    DB 44 24 18             fild     dword ptr [esp + 0x18]
0216:    DC 0F                   fmul     qword ptr [edi]
0218:    DC 4F F0                fmul     qword ptr [edi - 0x10]
021B:    DD 44 24 10             fld      qword ptr [esp + 0x10]
021F:    D9 E1                   fabs    ; |ST(0)|
0221:    DE C9                   fmulp    st(1)
0223:    DE E9                   fsubp    st(1)
0225:    DD 44 24 08             fld      qword ptr [esp + 8]
0229:    DB 44 24 1C             fild     dword ptr [esp + 0x1c]
022D:    DC 0F                   fmul     qword ptr [edi]
022F:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0232:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0236:    D9 E1                   fabs    ; |ST(0)|
0238:    DE C9                   fmulp    st(1)
023A:    DE E9                   fsubp    st(1)
023C:    D8 D1                   fcom     st(1)
023E:    9B                      wait
023F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0241:    80 E4 41                and      ah, 0x41
0244:    75 0A                   jne      0x250    ; Conditional jump
0246:    DD D9                   fstp     st(1)
0248:    DC 47 08                fadd     qword ptr [edi + 8]
024B:    E9 F2 01 00 00          jmp      0x442
0250:    DD D8                   fstp     st(0)
0252:    DC 47 08                fadd     qword ptr [edi + 8]
0255:    E9 E8 01 00 00          jmp      0x442
025A:    83 F8 04                cmp      eax, 4
025D:    0F 85 84 00 00 00       jne      0x2e7    ; Conditional jump
0263:    8B 44 24 18             mov      eax, dword ptr [esp + 0x18]
0267:    3B 44 24 1C             cmp      eax, dword ptr [esp + 0x1c]
026B:    7C 0E                   jl       0x27b    ; Conditional jump
026D:    DD 04 24                fld      qword ptr [esp]
0270:    DD 44 24 08             fld      qword ptr [esp + 8]
0274:    DD 1C 24                fstp     qword ptr [esp]
0277:    DD 5C 24 08             fstp     qword ptr [esp + 8]
027B:    DD 04 24                fld      qword ptr [esp]
027E:    DB 44 24 18             fild     dword ptr [esp + 0x18]
0282:    DC 0F                   fmul     qword ptr [edi]
0284:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0287:    DE C1                   faddp    st(1)
0289:    D9 E8                   fld1    ; Load 1.0
028B:    DC 67 F0                fsub     qword ptr [edi - 0x10]
028E:    DC 4F 08                fmul     qword ptr [edi + 8]
0291:    DE C1                   faddp    st(1)
0293:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0297:    DB 44 24 24             fild     dword ptr [esp + 0x24]
029B:    DC 0F                   fmul     qword ptr [edi]
029D:    DC 4F AC                fmul     qword ptr [edi - 0x54]
02A0:    DE E9                   fsubp    st(1)
02A2:    D9 C0                   fld      st(0)
02A4:    D9 E1                   fabs    ; |ST(0)|
02A6:    DE E9                   fsubp    st(1)
02A8:    DC 4F 08                fmul     qword ptr [edi + 8]
02AB:    D8 C8                   fmul     st(0)
02AD:    DD 44 24 08             fld      qword ptr [esp + 8]
02B1:    D8 C8                   fmul     st(0)
02B3:    DE C1                   faddp    st(1)
02B5:    D9 FA                   fsqrt    ; sqrt(ST(0))
02B7:    DB 44 24 1C             fild     dword ptr [esp + 0x1c]
02BB:    DC 0F                   fmul     qword ptr [edi]
02BD:    DC 4F B8                fmul     qword ptr [edi - 0x48]
02C0:    DE C1                   faddp    st(1)
02C2:    D9 E8                   fld1    ; Load 1.0
02C4:    DC 67 B8                fsub     qword ptr [edi - 0x48]
02C7:    DC 4F 08                fmul     qword ptr [edi + 8]
02CA:    DE C1                   faddp    st(1)
02CC:    D8 D1                   fcom     st(1)
02CE:    9B                      wait
02CF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
02D1:    80 E4 41                and      ah, 0x41
02D4:    75 04                   jne      0x2da    ; Conditional jump
02D6:    DD D9                   fstp     st(1)
02D8:    EB 02                   jmp      0x2dc
02DA:    DD D8                   fstp     st(0)
02DC:    DD D9                   fstp     st(1)
02DE:    DD 44 24 10             fld      qword ptr [esp + 0x10]
02E2:    E9 5B 01 00 00          jmp      0x442
02E7:    83 F8 05                cmp      eax, 5
02EA:    0F 85 A3 00 00 00       jne      0x393    ; Conditional jump
02F0:    DD D8                   fstp     st(0)
02F2:    DD 44 24 10             fld      qword ptr [esp + 0x10]
02F6:    8B 44 24 18             mov      eax, dword ptr [esp + 0x18]
02FA:    3B 44 24 1C             cmp      eax, dword ptr [esp + 0x1c]
02FE:    7C 1E                   jl       0x31e    ; Conditional jump
0300:    DD 04 24                fld      qword ptr [esp]
0303:    DD 44 24 08             fld      qword ptr [esp + 8]
0307:    DD 1C 24                fstp     qword ptr [esp]
030A:    DD 5C 24 08             fstp     qword ptr [esp + 8]
030E:    8B 44 24 18             mov      eax, dword ptr [esp + 0x18]
0312:    8B 54 24 1C             mov      edx, dword ptr [esp + 0x1c]
0316:    89 54 24 18             mov      dword ptr [esp + 0x18], edx
031A:    89 44 24 1C             mov      dword ptr [esp + 0x1c], eax
031E:    DB 44 24 18             fild     dword ptr [esp + 0x18]
0322:    DC 4F 28                fmul     qword ptr [edi + 0x28]
0325:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0328:    D9 E1                   fabs    ; |ST(0)|
032A:    D9 5C 24 18             fstp     dword ptr [esp + 0x18]
032E:    DB 44 24 1C             fild     dword ptr [esp + 0x1c]
0332:    DC 4F 20                fmul     qword ptr [edi + 0x20]
0335:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0338:    D9 5C 24 1C             fstp     dword ptr [esp + 0x1c]
033C:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0340:    DB 44 24 24             fild     dword ptr [esp + 0x24]
0344:    DC 0F                   fmul     qword ptr [edi]
0346:    DC 4F AC                fmul     qword ptr [edi - 0x54]
0349:    DE E9                   fsubp    st(1)
034B:    D9 C0                   fld      st(0)
034D:    D9 E1                   fabs    ; |ST(0)|
034F:    DE E9                   fsubp    st(1)
0351:    DC 4F 08                fmul     qword ptr [edi + 8]
0354:    D8 C8                   fmul     st(0)
0356:    DD 54 24 10             fst      qword ptr [esp + 0x10]
035A:    DD 44 24 08             fld      qword ptr [esp + 8]
035E:    D8 C8                   fmul     st(0)
0360:    DE C1                   faddp    st(1)
0362:    DD 04 24                fld      qword ptr [esp]
0365:    D8 C8                   fmul     st(0)
0367:    DE C1                   faddp    st(1)
0369:    D9 44 24 18             fld      dword ptr [esp + 0x18]
036D:    D9 44 24 1C             fld      dword ptr [esp + 0x1c]
0371:    D8 C8                   fmul     st(0)
0373:    DE C9                   fmulp    st(1)
0375:    DE C1                   faddp    st(1)
0377:    D9 FA                   fsqrt    ; sqrt(ST(0))
0379:    DD 44 24 10             fld      qword ptr [esp + 0x10]
037D:    DD 04 24                fld      qword ptr [esp]
0380:    D8 C8                   fmul     st(0)
0382:    DE C1                   faddp    st(1)
0384:    D9 FA                   fsqrt    ; sqrt(ST(0))
0386:    D9 FA                   fsqrt    ; sqrt(ST(0))
0388:    D8 4C 24 1C             fmul     dword ptr [esp + 0x1c]
038C:    DE C1                   faddp    st(1)
038E:    E9 AF 00 00 00          jmp      0x442
0393:    83 F8 06                cmp      eax, 6
0396:    75 69                   jne      0x401    ; Conditional jump
0398:    DB 44 24 1C             fild     dword ptr [esp + 0x1c]
039C:    DC 4F 20                fmul     qword ptr [edi + 0x20]
039F:    DC 4F B8                fmul     qword ptr [edi - 0x48]
03A2:    D9 5C 24 1C             fstp     dword ptr [esp + 0x1c]
03A6:    DD 04 24                fld      qword ptr [esp]
03A9:    DB 44 24 18             fild     dword ptr [esp + 0x18]
03AD:    DC 0F                   fmul     qword ptr [edi]
03AF:    DC 4F F0                fmul     qword ptr [edi - 0x10]
03B2:    D9 E8                   fld1    ; Load 1.0
03B4:    DC 67 F0                fsub     qword ptr [edi - 0x10]
03B7:    DC 4F 08                fmul     qword ptr [edi + 8]
03BA:    DE C1                   faddp    st(1)
03BC:    D8 64 24 1C             fsub     dword ptr [esp + 0x1c]
03C0:    D9 54 24 18             fst      dword ptr [esp + 0x18]
03C4:    DE C1                   faddp    st(1)
03C6:    D9 C0                   fld      st(0)
03C8:    D9 E1                   fabs    ; |ST(0)|
03CA:    DE C1                   faddp    st(1)
03CC:    DC 4F 08                fmul     qword ptr [edi + 8]
03CF:    D8 C8                   fmul     st(0)
03D1:    DD 44 24 08             fld      qword ptr [esp + 8]
03D5:    D8 44 24 18             fadd     dword ptr [esp + 0x18]
03D9:    D9 C0                   fld      st(0)
03DB:    D9 E1                   fabs    ; |ST(0)|
03DD:    DE C1                   faddp    st(1)
03DF:    DC 4F 08                fmul     qword ptr [edi + 8]
03E2:    D8 C8                   fmul     st(0)
03E4:    DE C1                   faddp    st(1)
03E6:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
03E8:    D8 44 24 18             fadd     dword ptr [esp + 0x18]
03EC:    D9 C0                   fld      st(0)
03EE:    D9 E1                   fabs    ; |ST(0)|
03F0:    DE C1                   faddp    st(1)
03F2:    DC 4F 08                fmul     qword ptr [edi + 8]
03F5:    D8 C8                   fmul     st(0)
03F7:    DE C1                   faddp    st(1)
03F9:    D9 FA                   fsqrt    ; sqrt(ST(0))
03FB:    D8 44 24 1C             fadd     dword ptr [esp + 0x1c]
03FF:    EB 51                   jmp      0x452
0401:    DD 04 24                fld      qword ptr [esp]
0404:    DB 44 24 18             fild     dword ptr [esp + 0x18]
0408:    DC 0F                   fmul     qword ptr [edi]
040A:    DC 4F F0                fmul     qword ptr [edi - 0x10]
040D:    DE C1                   faddp    st(1)
040F:    D9 E8                   fld1    ; Load 1.0
0411:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0414:    DC 4F 08                fmul     qword ptr [edi + 8]
0417:    DE C1                   faddp    st(1)
0419:    DD 44 24 08             fld      qword ptr [esp + 8]
041D:    DB 44 24 1C             fild     dword ptr [esp + 0x1c]
0421:    DC 0F                   fmul     qword ptr [edi]
0423:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0426:    DE C1                   faddp    st(1)
0428:    D9 E8                   fld1    ; Load 1.0
042A:    DC 67 B8                fsub     qword ptr [edi - 0x48]
042D:    DC 4F 08                fmul     qword ptr [edi + 8]
0430:    DE C1                   faddp    st(1)
0432:    D8 D1                   fcom     st(1)
0434:    9B                      wait
0435:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0437:    80 E4 41                and      ah, 0x41
043A:    75 04                   jne      0x440    ; Conditional jump
043C:    DD D9                   fstp     st(1)
043E:    EB 02                   jmp      0x442
0440:    DD D8                   fstp     st(0)
0442:    D8 D1                   fcom     st(1)
0444:    9B                      wait
0445:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0447:    80 E4 41                and      ah, 0x41
044A:    75 04                   jne      0x450    ; Conditional jump
044C:    DD D9                   fstp     st(1)
044E:    EB 02                   jmp      0x452
0450:    DD D8                   fstp     st(0)
0452:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0456:    75 03                   jne      0x45b    ; Conditional jump
0458:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
045B:    DD 5E E0                fstp     qword ptr [esi - 0x20]
045E:    83 C4 3C                add      esp, 0x3c
0461:    C3                      ret
