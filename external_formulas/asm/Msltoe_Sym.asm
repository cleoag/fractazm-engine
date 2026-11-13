; Disassembly: Msltoe_Sym.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 338 bytes, decoded: 338 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    83 C4 B4                add      esp, -0x4c
0006:    53                      push     ebx
0007:    56                      push     esi
0008:    57                      push     edi
0009:    8B F9                   mov      edi, ecx
000B:    8B F2                   mov      esi, edx
000D:    89 45 FC                mov      dword ptr [ebp - 4], eax
0010:    8B 45 08                mov      eax, dword ptr [ebp + 8]
0013:    89 45 B4                mov      dword ptr [ebp - 0x4c], eax
0016:    8B 45 B4                mov      eax, dword ptr [ebp - 0x4c]
0019:    8B 58 30                mov      ebx, dword ptr [eax + 0x30]
001C:    DD 07                   fld      qword ptr [edi]
001E:    DD 06                   fld      qword ptr [esi]
0020:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0022:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0025:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0028:    8D 43 20                lea      eax, [ebx + 0x20]
002B:    DC 00                   fadd     qword ptr [eax]
002D:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0030:    9B                      wait
0031:    33 C0                   xor      eax, eax
0033:    89 45 E8                mov      dword ptr [ebp - 0x18], eax
0036:    89 45 EC                mov      dword ptr [ebp - 0x14], eax
0039:    EB 18                   jmp      0x53
003B:    DD 45 F0                fld      qword ptr [ebp - 0x10]
003E:    8D 43 08                lea      eax, [ebx + 8]
0041:    DC 20                   fsub     qword ptr [eax]
0043:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0046:    9B                      wait
0047:    DD 45 E8                fld      qword ptr [ebp - 0x18]
004A:    8D 43 08                lea      eax, [ebx + 8]
004D:    DC 20                   fsub     qword ptr [eax]
004F:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0052:    9B                      wait
0053:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0056:    DC 1B                   fcomp    qword ptr [ebx]
0058:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005A:    9E                      sahf
005B:    77 DE                   ja       0x3b    ; Conditional jump
005D:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0060:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0062:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0065:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0068:    DD 06                   fld      qword ptr [esi]
006A:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
006D:    DD 07                   fld      qword ptr [edi]
006F:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0072:    DE E9                   fsubp    st(1)
0074:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0077:    9B                      wait
0078:    DD 06                   fld      qword ptr [esi]
007A:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
007D:    DD 07                   fld      qword ptr [edi]
007F:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0082:    DE C1                   faddp    st(1)
0084:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
0087:    9B                      wait
0088:    8B 45 C0                mov      eax, dword ptr [ebp - 0x40]
008B:    89 06                   mov      dword ptr [esi], eax
008D:    8B 45 C4                mov      eax, dword ptr [ebp - 0x3c]
0090:    89 46 04                mov      dword ptr [esi + 4], eax
0093:    8B 45 B8                mov      eax, dword ptr [ebp - 0x48]
0096:    89 07                   mov      dword ptr [edi], eax
0098:    8B 45 BC                mov      eax, dword ptr [ebp - 0x44]
009B:    89 47 04                mov      dword ptr [edi + 4], eax
009E:    DD 07                   fld      qword ptr [edi]
00A0:    DC 0F                   fmul     qword ptr [edi]
00A2:    8B 45 FC                mov      eax, dword ptr [ebp - 4]
00A5:    DD 00                   fld      qword ptr [eax]
00A7:    8B 45 FC                mov      eax, dword ptr [ebp - 4]
00AA:    DC 08                   fmul     qword ptr [eax]
00AC:    DD 06                   fld      qword ptr [esi]
00AE:    DC 0E                   fmul     qword ptr [esi]
00B0:    DE C1                   faddp    st(1)
00B2:    DD 07                   fld      qword ptr [edi]
00B4:    DC 0F                   fmul     qword ptr [edi]
00B6:    DE C1                   faddp    st(1)
00B8:    DE F9                   fdivp    st(1)
00BA:    8D 43 10                lea      eax, [ebx + 0x10]
00BD:    DC 28                   fsubr    qword ptr [eax]
00BF:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
00C2:    9B                      wait
00C3:    8B 45 FC                mov      eax, dword ptr [ebp - 4]
00C6:    DD 00                   fld      qword ptr [eax]
00C8:    8B 45 FC                mov      eax, dword ptr [ebp - 4]
00CB:    DC 08                   fmul     qword ptr [eax]
00CD:    DD 06                   fld      qword ptr [esi]
00CF:    DC 0E                   fmul     qword ptr [esi]
00D1:    DE E9                   fsubp    st(1)
00D3:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00D6:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
00D9:    9B                      wait
00DA:    8D 43 18                lea      eax, [ebx + 0x18]
00DD:    DD 00                   fld      qword ptr [eax]
00DF:    8B 45 FC                mov      eax, dword ptr [ebp - 4]
00E2:    DC 08                   fmul     qword ptr [eax]
00E4:    DC 0E                   fmul     qword ptr [esi]
00E6:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00E9:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
00EC:    9B                      wait
00ED:    8B 45 FC                mov      eax, dword ptr [ebp - 4]
00F0:    DD 00                   fld      qword ptr [eax]
00F2:    8B 45 FC                mov      eax, dword ptr [ebp - 4]
00F5:    DC 08                   fmul     qword ptr [eax]
00F7:    DD 06                   fld      qword ptr [esi]
00F9:    DC 0E                   fmul     qword ptr [esi]
00FB:    DE C1                   faddp    st(1)
00FD:    D9 FA                   fsqrt    ; sqrt(ST(0))
00FF:    83 C3 18                add      ebx, 0x18
0102:    DD 03                   fld      qword ptr [ebx]
0104:    DC 0F                   fmul     qword ptr [edi]
0106:    DE C9                   fmulp    st(1)
0108:    DD 5D B8                fstp     qword ptr [ebp - 0x48]
010B:    9B                      wait
010C:    DD 45 C8                fld      qword ptr [ebp - 0x38]
010F:    8B 45 B4                mov      eax, dword ptr [ebp - 0x4c]
0112:    DC 40 18                fadd     qword ptr [eax + 0x18]
0115:    8B 45 FC                mov      eax, dword ptr [ebp - 4]
0118:    DD 18                   fstp     qword ptr [eax]
011A:    9B                      wait
011B:    DD 45 C0                fld      qword ptr [ebp - 0x40]
011E:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0121:    DD 45 B8                fld      qword ptr [ebp - 0x48]
0124:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0127:    DE C1                   faddp    st(1)
0129:    8B 45 B4                mov      eax, dword ptr [ebp - 0x4c]
012C:    DC 40 20                fadd     qword ptr [eax + 0x20]
012F:    DD 1E                   fstp     qword ptr [esi]
0131:    9B                      wait
0132:    DD 45 B8                fld      qword ptr [ebp - 0x48]
0135:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0138:    DD 45 C0                fld      qword ptr [ebp - 0x40]
013B:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
013E:    DE E9                   fsubp    st(1)
0140:    8B 45 B4                mov      eax, dword ptr [ebp - 0x4c]
0143:    DC 40 28                fadd     qword ptr [eax + 0x28]
0146:    DD 1F                   fstp     qword ptr [edi]
0148:    9B                      wait
0149:    5F                      pop      edi
014A:    5E                      pop      esi
014B:    5B                      pop      ebx
014C:    8B E5                   mov      esp, ebp
014E:    5D                      pop      ebp
014F:    C2 08 00                ret      8
