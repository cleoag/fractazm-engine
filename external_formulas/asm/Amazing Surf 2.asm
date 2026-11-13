; Disassembly: Amazing Surf 2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 536 bytes, decoded: 536 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 7D 08                mov      edi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 77 30                mov      esi, dword ptr [edi + 0x30]
000E:    81 C7 80 00 00 00       add      edi, 0x80
0014:    DD 46 D8                fld      qword ptr [esi - 0x28]
0017:    D9 C0                   fld      st(0)
0019:    DC 0B                   fmul     qword ptr [ebx]
001B:    8B 46 B0                mov      eax, dword ptr [esi - 0x50]
001E:    83 E0 01                and      eax, 1
0021:    74 02                   je       0x25    ; Conditional jump
0023:    D9 E1                   fabs    ; |ST(0)|
0025:    DD 1B                   fstp     qword ptr [ebx]
0027:    DC 0A                   fmul     qword ptr [edx]
0029:    8B 46 B0                mov      eax, dword ptr [esi - 0x50]
002C:    83 E0 02                and      eax, 2
002F:    74 02                   je       0x33    ; Conditional jump
0031:    D9 E1                   fabs    ; |ST(0)|
0033:    DD 1A                   fstp     qword ptr [edx]
0035:    90                      nop
0036:    90                      nop
0037:    83 EC 20                sub      esp, 0x20
003A:    8B 43 04                mov      eax, dword ptr [ebx + 4]
003D:    25 00 00 F0 FF          and      eax, 0xfff00000
0042:    89 45 0C                mov      dword ptr [ebp + 0xc], eax
0045:    DD 03                   fld      qword ptr [ebx]
0047:    DD 46 10                fld      qword ptr [esi + 0x10]
004A:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
004C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0050:    72 45                   jb       0x97    ; Conditional jump
0052:    DD 03                   fld      qword ptr [ebx]
0054:    DD 46 18                fld      qword ptr [esi + 0x18]
0057:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0059:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005D:    73 38                   jae      0x97    ; Conditional jump
005F:    8B 46 B0                mov      eax, dword ptr [esi - 0x50]
0062:    83 E0 04                and      eax, 4
0065:    74 1E                   je       0x85    ; Conditional jump
0067:    DD 03                   fld      qword ptr [ebx]
0069:    DD 46 20                fld      qword ptr [esi + 0x20]
006C:    90                      nop
006D:    D8 C9                   fmul     st(1)
006F:    D9 C1                   fld      st(1)
0071:    D9 E1                   fabs    ; |ST(0)|
0073:    DE F9                   fdivp    st(1)
0075:    DE C1                   faddp    st(1)
0077:    DD 1B                   fstp     qword ptr [ebx]
0079:    8B 43 04                mov      eax, dword ptr [ebx + 4]
007C:    25 00 00 F0 FF          and      eax, 0xfff00000
0081:    89 45 0C                mov      dword ptr [ebp + 0xc], eax
0084:    90                      nop
0085:    B8 FF FF 0F 00          mov      eax, 0xfffff
008A:    21 43 04                and      dword ptr [ebx + 4], eax
008D:    B8 00 00 F0 3F          mov      eax, 0x3ff00000
0092:    31 43 04                xor      dword ptr [ebx + 4], eax
0095:    EB 10                   jmp      0xa7
0097:    8B 45 0C                mov      eax, dword ptr [ebp + 0xc]
009A:    25 00 00 00 80          and      eax, 0x80000000
009F:    35 00 00 F0 3F          xor      eax, 0x3ff00000
00A4:    89 45 0C                mov      dword ptr [ebp + 0xc], eax
00A7:    DD 03                   fld      qword ptr [ebx]
00A9:    D9 E1                   fabs    ; |ST(0)|
00AB:    DC 06                   fadd     qword ptr [esi]
00AD:    D9 E1                   fabs    ; |ST(0)|
00AF:    D9 E0                   fchs    ; -ST(0)
00B1:    DC 46 08                fadd     qword ptr [esi + 8]
00B4:    DD 1B                   fstp     qword ptr [ebx]
00B6:    B8 00 00 00 00          mov      eax, 0
00BB:    89 45 08                mov      dword ptr [ebp + 8], eax
00BE:    DD 45 08                fld      qword ptr [ebp + 8]
00C1:    DC 0B                   fmul     qword ptr [ebx]
00C3:    DD 1B                   fstp     qword ptr [ebx]
00C5:    8B 45 0C                mov      eax, dword ptr [ebp + 0xc]
00C8:    25 00 00 10 00          and      eax, 0x100000
00CD:    C1 E0 0B                shl      eax, 0xb
00D0:    31 43 04                xor      dword ptr [ebx + 4], eax
00D3:    D9 D0                   fnop
00D5:    8B 42 04                mov      eax, dword ptr [edx + 4]
00D8:    25 00 00 F0 FF          and      eax, 0xfff00000
00DD:    89 45 0C                mov      dword ptr [ebp + 0xc], eax
00E0:    DD 02                   fld      qword ptr [edx]
00E2:    DD 46 10                fld      qword ptr [esi + 0x10]
00E5:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00E7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E9:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00EB:    72 45                   jb       0x132    ; Conditional jump
00ED:    DD 02                   fld      qword ptr [edx]
00EF:    DD 46 18                fld      qword ptr [esi + 0x18]
00F2:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00F4:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F6:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00F8:    73 38                   jae      0x132    ; Conditional jump
00FA:    8B 46 B0                mov      eax, dword ptr [esi - 0x50]
00FD:    83 E0 08                and      eax, 8
0100:    74 1E                   je       0x120    ; Conditional jump
0102:    DD 02                   fld      qword ptr [edx]
0104:    DD 46 20                fld      qword ptr [esi + 0x20]
0107:    90                      nop
0108:    D8 C9                   fmul     st(1)
010A:    D9 C1                   fld      st(1)
010C:    D9 E1                   fabs    ; |ST(0)|
010E:    DE F9                   fdivp    st(1)
0110:    DE C1                   faddp    st(1)
0112:    DD 1A                   fstp     qword ptr [edx]
0114:    8B 42 04                mov      eax, dword ptr [edx + 4]
0117:    25 00 00 F0 FF          and      eax, 0xfff00000
011C:    89 45 0C                mov      dword ptr [ebp + 0xc], eax
011F:    90                      nop
0120:    B8 FF FF 0F 00          mov      eax, 0xfffff
0125:    21 42 04                and      dword ptr [edx + 4], eax
0128:    B8 00 00 F0 3F          mov      eax, 0x3ff00000
012D:    31 42 04                xor      dword ptr [edx + 4], eax
0130:    EB 10                   jmp      0x142
0132:    8B 45 0C                mov      eax, dword ptr [ebp + 0xc]
0135:    25 00 00 00 80          and      eax, 0x80000000
013A:    35 00 00 F0 3F          xor      eax, 0x3ff00000
013F:    89 45 0C                mov      dword ptr [ebp + 0xc], eax
0142:    DD 02                   fld      qword ptr [edx]
0144:    D9 E1                   fabs    ; |ST(0)|
0146:    DC 06                   fadd     qword ptr [esi]
0148:    D9 E1                   fabs    ; |ST(0)|
014A:    D9 E0                   fchs    ; -ST(0)
014C:    DC 46 08                fadd     qword ptr [esi + 8]
014F:    DD 1A                   fstp     qword ptr [edx]
0151:    B8 00 00 00 00          mov      eax, 0
0156:    89 45 08                mov      dword ptr [ebp + 8], eax
0159:    DD 45 08                fld      qword ptr [ebp + 8]
015C:    DC 0A                   fmul     qword ptr [edx]
015E:    DD 1A                   fstp     qword ptr [edx]
0160:    8B 45 0C                mov      eax, dword ptr [ebp + 0xc]
0163:    25 00 00 10 00          and      eax, 0x100000
0168:    C1 E0 0B                shl      eax, 0xb
016B:    31 42 04                xor      dword ptr [edx + 4], eax
016E:    90                      nop
016F:    83 EC E0                sub      esp, -0x20
0172:    90                      nop
0173:    90                      nop
0174:    DD 03                   fld      qword ptr [ebx]
0176:    DC 76 D8                fdiv     qword ptr [esi - 0x28]
0179:    DD 02                   fld      qword ptr [edx]
017B:    DC 76 D8                fdiv     qword ptr [esi - 0x28]
017E:    90                      nop
017F:    DD 01                   fld      qword ptr [ecx]
0181:    90                      nop
0182:    D9 C0                   fld      st(0)
0184:    D8 4E BC                fmul     dword ptr [esi - 0x44]
0187:    D9 C2                   fld      st(2)
0189:    D8 4E B8                fmul     dword ptr [esi - 0x48]
018C:    DE C1                   faddp    st(1)
018E:    D9 C3                   fld      st(3)
0190:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
0193:    DE C1                   faddp    st(1)
0195:    D9 C1                   fld      st(1)
0197:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
019A:    D9 C3                   fld      st(3)
019C:    D8 4E D0                fmul     dword ptr [esi - 0x30]
019F:    DE C1                   faddp    st(1)
01A1:    D9 C4                   fld      st(4)
01A3:    D8 4E CC                fmul     dword ptr [esi - 0x34]
01A6:    DE C1                   faddp    st(1)
01A8:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
01AA:    D8 4E C8                fmul     dword ptr [esi - 0x38]
01AD:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
01AF:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
01B2:    DE C3                   faddp    st(3)
01B4:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
01B6:    D8 4E C0                fmul     dword ptr [esi - 0x40]
01B9:    DE C2                   faddp    st(2)
01BB:    90                      nop
01BC:    D9 C0                   fld      st(0)
01BE:    D8 C9                   fmul     st(1)
01C0:    D9 C2                   fld      st(2)
01C2:    D8 CB                   fmul     st(3)
01C4:    DE C1                   faddp    st(1)
01C6:    D9 C3                   fld      st(3)
01C8:    D8 CC                   fmul     st(4)
01CA:    DE C1                   faddp    st(1)
01CC:    DC 56 E0                fcom     qword ptr [esi - 0x20]
01CF:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01D1:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01D3:    73 07                   jae      0x1dc    ; Conditional jump
01D5:    DD D8                   fstp     st(0)
01D7:    DD 46 E8                fld      qword ptr [esi - 0x18]
01DA:    EB 16                   jmp      0x1f2
01DC:    D9 E8                   fld1    ; Load 1.0
01DE:    D8 D1                   fcom     st(1)
01E0:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01E2:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01E4:    72 07                   jb       0x1ed    ; Conditional jump
01E6:    DD D8                   fstp     st(0)
01E8:    DC 7E F0                fdivr    qword ptr [esi - 0x10]
01EB:    EB 05                   jmp      0x1f2
01ED:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
01EF:    DD 46 F0                fld      qword ptr [esi - 0x10]
01F2:    DD 41 08                fld      qword ptr [ecx + 8]
01F5:    D8 C9                   fmul     st(1)
01F7:    DD 59 08                fstp     qword ptr [ecx + 8]
01FA:    DC CB                   fmul     st(3), st(0)
01FC:    DC CA                   fmul     st(2), st(0)
01FE:    DE C9                   fmulp    st(1)
0200:    DC 47 A8                fadd     qword ptr [edi - 0x58]
0203:    DD 19                   fstp     qword ptr [ecx]
0205:    DC 47 A0                fadd     qword ptr [edi - 0x60]
0208:    DD 1A                   fstp     qword ptr [edx]
020A:    DC 47 98                fadd     qword ptr [edi - 0x68]
020D:    DD 1B                   fstp     qword ptr [ebx]
020F:    89 D8                   mov      eax, ebx
0211:    5B                      pop      ebx
0212:    5F                      pop      edi
0213:    5E                      pop      esi
0214:    5D                      pop      ebp
0215:    C2 08 00                ret      8
