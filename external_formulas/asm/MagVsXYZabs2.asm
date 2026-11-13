; Disassembly: MagVsXYZabs2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 399 bytes, decoded: 399 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 48 00 00 00       sub      esp, 0x48
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 47 E0                fld      qword ptr [edi - 0x20]
0019:    DD 5D F8                fstp     qword ptr [ebp - 8]
001C:    D9 D0                   fnop
001E:    DD 47 F0                fld      qword ptr [edi - 0x10]
0021:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0024:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0027:    D9 D0                   fnop
0029:    DD 03                   fld      qword ptr [ebx]
002B:    D8 C8                   fmul     st(0)
002D:    DD 02                   fld      qword ptr [edx]
002F:    D8 C8                   fmul     st(0)
0031:    DD 01                   fld      qword ptr [ecx]
0033:    D8 C8                   fmul     st(0)
0035:    DE C1                   faddp    st(1)
0037:    DE C1                   faddp    st(1)
0039:    DD 55 B8                fst      qword ptr [ebp - 0x48]
003C:    D9 FA                   fsqrt    ; sqrt(ST(0))
003E:    DD 03                   fld      qword ptr [ebx]
0040:    D8 C8                   fmul     st(0)
0042:    DC 4D F8                fmul     qword ptr [ebp - 8]
0045:    DC 45 B8                fadd     qword ptr [ebp - 0x48]
0048:    D9 FA                   fsqrt    ; sqrt(ST(0))
004A:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
004D:    DD 02                   fld      qword ptr [edx]
004F:    D8 C8                   fmul     st(0)
0051:    DC 4D F8                fmul     qword ptr [ebp - 8]
0054:    DC 45 B8                fadd     qword ptr [ebp - 0x48]
0057:    D9 FA                   fsqrt    ; sqrt(ST(0))
0059:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
005C:    DD 01                   fld      qword ptr [ecx]
005E:    D8 C8                   fmul     st(0)
0060:    DC 4D F8                fmul     qword ptr [ebp - 8]
0063:    DC 45 B8                fadd     qword ptr [ebp - 0x48]
0066:    D9 FA                   fsqrt    ; sqrt(ST(0))
0068:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
006B:    D9 D0                   fnop
006D:    DD 47 F0                fld      qword ptr [edi - 0x10]
0070:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0073:    D9 E1                   fabs    ; |ST(0)|
0075:    D9 F1                   fyl2x
0077:    D9 E8                   fld1    ; Load 1.0
0079:    D9 C1                   fld      st(1)
007B:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
007D:    D9 F0                   f2xm1
007F:    DE C1                   faddp    st(1)
0081:    D9 FD                   fscale
0083:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0085:    DD D8                   fstp     st(0)
0087:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
008A:    D9 D0                   fnop
008C:    DD 47 F0                fld      qword ptr [edi - 0x10]
008F:    DD 45 C8                fld      qword ptr [ebp - 0x38]
0092:    D9 E1                   fabs    ; |ST(0)|
0094:    D9 F1                   fyl2x
0096:    D9 E8                   fld1    ; Load 1.0
0098:    D9 C1                   fld      st(1)
009A:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
009C:    D9 F0                   f2xm1
009E:    DE C1                   faddp    st(1)
00A0:    D9 FD                   fscale
00A2:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00A4:    DD D8                   fstp     st(0)
00A6:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
00A9:    D9 D0                   fnop
00AB:    DD 47 F0                fld      qword ptr [edi - 0x10]
00AE:    DD 45 C0                fld      qword ptr [ebp - 0x40]
00B1:    D9 E1                   fabs    ; |ST(0)|
00B3:    D9 F1                   fyl2x
00B5:    D9 E8                   fld1    ; Load 1.0
00B7:    D9 C1                   fld      st(1)
00B9:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
00BB:    D9 F0                   f2xm1
00BD:    DE C1                   faddp    st(1)
00BF:    D9 FD                   fscale
00C1:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00C3:    DD D8                   fstp     st(0)
00C5:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
00C8:    D9 D0                   fnop
00CA:    90                      nop
00CB:    90                      nop
00CC:    90                      nop
00CD:    D9 D0                   fnop
00CF:    DD 03                   fld      qword ptr [ebx]
00D1:    D9 E1                   fabs    ; |ST(0)|
00D3:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00D6:    D9 C1                   fld      st(1)
00D8:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00DA:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00DC:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
00DF:    D9 D0                   fnop
00E1:    DD 02                   fld      qword ptr [edx]
00E3:    D9 E1                   fabs    ; |ST(0)|
00E5:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00E8:    D9 C1                   fld      st(1)
00EA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00EC:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00EE:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
00F1:    D9 D0                   fnop
00F3:    DD 01                   fld      qword ptr [ecx]
00F5:    D9 E1                   fabs    ; |ST(0)|
00F7:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00FA:    D9 C1                   fld      st(1)
00FC:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00FE:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0100:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0103:    D9 D0                   fnop
0105:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0108:    DC 4F E8                fmul     qword ptr [edi - 0x18]
010B:    DC 47 D8                fadd     qword ptr [edi - 0x28]
010E:    D9 FF                   fcos    ; cos(ST(0))
0110:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0113:    DD 46 18                fld      qword ptr [esi + 0x18]
0116:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0119:    D9 EE                   fldz    ; Load 0.0
011B:    D8 D9                   fcomp    st(1)
011D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
011F:    80 E4 41                and      ah, 0x41
0122:    75 04                   jne      0x128    ; Conditional jump
0124:    DE C1                   faddp    st(1)
0126:    EB 02                   jmp      0x12a
0128:    DE E9                   fsubp    st(1)
012A:    DD 1B                   fstp     qword ptr [ebx]
012C:    90                      nop
012D:    D9 D0                   fnop
012F:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0132:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0135:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0138:    D9 FF                   fcos    ; cos(ST(0))
013A:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
013D:    DD 46 20                fld      qword ptr [esi + 0x20]
0140:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0143:    D9 EE                   fldz    ; Load 0.0
0145:    D8 D9                   fcomp    st(1)
0147:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0149:    80 E4 41                and      ah, 0x41
014C:    75 04                   jne      0x152    ; Conditional jump
014E:    DE C1                   faddp    st(1)
0150:    EB 02                   jmp      0x154
0152:    DE E9                   fsubp    st(1)
0154:    DD 1A                   fstp     qword ptr [edx]
0156:    90                      nop
0157:    D9 D0                   fnop
0159:    DD 45 D8                fld      qword ptr [ebp - 0x28]
015C:    DC 4F E8                fmul     qword ptr [edi - 0x18]
015F:    DC 47 D8                fadd     qword ptr [edi - 0x28]
0162:    D9 FF                   fcos    ; cos(ST(0))
0164:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0167:    DD 46 28                fld      qword ptr [esi + 0x28]
016A:    DC 4F C8                fmul     qword ptr [edi - 0x38]
016D:    D9 EE                   fldz    ; Load 0.0
016F:    D8 D9                   fcomp    st(1)
0171:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0173:    80 E4 41                and      ah, 0x41
0176:    75 04                   jne      0x17c    ; Conditional jump
0178:    DE C1                   faddp    st(1)
017A:    EB 02                   jmp      0x17e
017C:    DE E9                   fsubp    st(1)
017E:    DD 19                   fstp     qword ptr [ecx]
0180:    90                      nop
0181:    90                      nop
0182:    DD D8                   fstp     st(0)
0184:    8B C3                   mov      eax, ebx
0186:    5F                      pop      edi
0187:    5E                      pop      esi
0188:    5B                      pop      ebx
0189:    8B E5                   mov      esp, ebp
018B:    5D                      pop      ebp
018C:    C2 08 00                ret      8
