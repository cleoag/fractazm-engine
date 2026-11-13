; Disassembly: Sierpinski4ex.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 379 bytes, decoded: 379 bytes (100.0%)
; DEoption: 6

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    57                      push     edi
0006:    89 C3                   mov      ebx, eax
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 41 08                fld      qword ptr [ecx + 8]
0011:    DD 01                   fld      qword ptr [ecx]
0013:    DD 02                   fld      qword ptr [edx]
0015:    DD 03                   fld      qword ptr [ebx]
0017:    90                      nop
0018:    D9 E0                   fchs    ; -ST(0)
001A:    D8 D1                   fcom     st(1)
001C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
001E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0020:    72 02                   jb       0x24    ; Conditional jump
0022:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0024:    D8 D2                   fcom     st(2)
0026:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0028:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
002A:    72 02                   jb       0x2e    ; Conditional jump
002C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
002E:    D9 E0                   fchs    ; -ST(0)
0030:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0032:    D9 E0                   fchs    ; -ST(0)
0034:    D8 D2                   fcom     st(2)
0036:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0038:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
003A:    72 02                   jb       0x3e    ; Conditional jump
003C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003E:    D9 E0                   fchs    ; -ST(0)
0040:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0042:    90                      nop
0043:    D9 E0                   fchs    ; -ST(0)
0045:    D8 D3                   fcom     st(3)
0047:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0049:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
004B:    72 02                   jb       0x4f    ; Conditional jump
004D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004F:    D9 E0                   fchs    ; -ST(0)
0051:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0053:    D9 E0                   fchs    ; -ST(0)
0055:    D8 D3                   fcom     st(3)
0057:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0059:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005B:    72 02                   jb       0x5f    ; Conditional jump
005D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
005F:    D9 E0                   fchs    ; -ST(0)
0061:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0063:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0065:    D9 E0                   fchs    ; -ST(0)
0067:    D8 D3                   fcom     st(3)
0069:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006D:    72 02                   jb       0x71    ; Conditional jump
006F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0071:    D9 E0                   fchs    ; -ST(0)
0073:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0075:    90                      nop
0076:    DD 47 80                fld      qword ptr [edi - 0x80]
0079:    DE C1                   faddp    st(1)
007B:    DD 47 88                fld      qword ptr [edi - 0x78]
007E:    DE C2                   faddp    st(2)
0080:    DD 87 78 FF FF FF       fld      qword ptr [edi - 0x88]
0086:    DE C3                   faddp    st(3)
0088:    DD 87 70 FF FF FF       fld      qword ptr [edi - 0x90]
008E:    DE C4                   faddp    st(4)
0090:    90                      nop
0091:    83 EF 28                sub      edi, 0x28
0094:    D9 C0                   fld      st(0)
0096:    D8 4F F4                fmul     dword ptr [edi - 0xc]
0099:    D9 C2                   fld      st(2)
009B:    D8 4F F0                fmul     dword ptr [edi - 0x10]
009E:    DE C1                   faddp    st(1)
00A0:    D9 C3                   fld      st(3)
00A2:    D8 4F EC                fmul     dword ptr [edi - 0x14]
00A5:    DE C1                   faddp    st(1)
00A7:    D9 C4                   fld      st(4)
00A9:    D8 4F E8                fmul     dword ptr [edi - 0x18]
00AC:    DE C1                   faddp    st(1)
00AE:    DD 1B                   fstp     qword ptr [ebx]
00B0:    D9 C0                   fld      st(0)
00B2:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
00B5:    D9 C2                   fld      st(2)
00B7:    D8 4F E0                fmul     dword ptr [edi - 0x20]
00BA:    DE C1                   faddp    st(1)
00BC:    D9 C3                   fld      st(3)
00BE:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00C1:    DE C1                   faddp    st(1)
00C3:    D9 C4                   fld      st(4)
00C5:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00C8:    DE C1                   faddp    st(1)
00CA:    DD 1A                   fstp     qword ptr [edx]
00CC:    D9 C0                   fld      st(0)
00CE:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00D1:    D9 C2                   fld      st(2)
00D3:    D8 4F D0                fmul     dword ptr [edi - 0x30]
00D6:    DE C1                   faddp    st(1)
00D8:    D9 C3                   fld      st(3)
00DA:    D8 4F CC                fmul     dword ptr [edi - 0x34]
00DD:    DE C1                   faddp    st(1)
00DF:    D9 C4                   fld      st(4)
00E1:    D8 4F C8                fmul     dword ptr [edi - 0x38]
00E4:    DE C1                   faddp    st(1)
00E6:    DD 19                   fstp     qword ptr [ecx]
00E8:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
00EB:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00ED:    D8 4F C0                fmul     dword ptr [edi - 0x40]
00F0:    DE C1                   faddp    st(1)
00F2:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00F4:    D8 4F BC                fmul     dword ptr [edi - 0x44]
00F7:    DE C1                   faddp    st(1)
00F9:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00FB:    D8 4F B8                fmul     dword ptr [edi - 0x48]
00FE:    DE C1                   faddp    st(1)
0100:    DD 59 08                fstp     qword ptr [ecx + 8]
0103:    83 C7 28                add      edi, 0x28
0106:    DD 41 08                fld      qword ptr [ecx + 8]
0109:    DC A7 70 FF FF FF       fsub     qword ptr [edi - 0x90]
010F:    DD 01                   fld      qword ptr [ecx]
0111:    DC A7 78 FF FF FF       fsub     qword ptr [edi - 0x88]
0117:    DD 02                   fld      qword ptr [edx]
0119:    DC 67 88                fsub     qword ptr [edi - 0x78]
011C:    DD 03                   fld      qword ptr [ebx]
011E:    DC 67 80                fsub     qword ptr [edi - 0x80]
0121:    DD 47 F0                fld      qword ptr [edi - 0x10]
0124:    90                      nop
0125:    90                      nop
0126:    90                      nop
0127:    D9 C0                   fld      st(0)
0129:    DC 8E E8 00 00 00       fmul     qword ptr [esi + 0xe8]
012F:    DD 9E E8 00 00 00       fstp     qword ptr [esi + 0xe8]
0135:    DC CC                   fmul     st(4), st(0)
0137:    DC CB                   fmul     st(3), st(0)
0139:    DC CA                   fmul     st(2), st(0)
013B:    DC C9                   fmul     st(1), st(0)
013D:    D9 E8                   fld1    ; Load 1.0
013F:    DE E9                   fsubp    st(1)
0141:    D9 E0                   fchs    ; -ST(0)
0143:    DD 47 E8                fld      qword ptr [edi - 0x18]
0146:    D8 C9                   fmul     st(1)
0148:    D8 C2                   fadd     st(2)
014A:    DD 1B                   fstp     qword ptr [ebx]
014C:    DD 47 E0                fld      qword ptr [edi - 0x20]
014F:    D8 C9                   fmul     st(1)
0151:    D8 C3                   fadd     st(3)
0153:    DD 1A                   fstp     qword ptr [edx]
0155:    DD 47 D8                fld      qword ptr [edi - 0x28]
0158:    D8 C9                   fmul     st(1)
015A:    D8 C4                   fadd     st(4)
015C:    DD 19                   fstp     qword ptr [ecx]
015E:    DD 47 D0                fld      qword ptr [edi - 0x30]
0161:    D8 C9                   fmul     st(1)
0163:    D8 C5                   fadd     st(5)
0165:    DD 59 08                fstp     qword ptr [ecx + 8]
0168:    DD D8                   fstp     st(0)
016A:    DD D8                   fstp     st(0)
016C:    DD D8                   fstp     st(0)
016E:    DD D8                   fstp     st(0)
0170:    DD D8                   fstp     st(0)
0172:    89 D8                   mov      eax, ebx
0174:    5F                      pop      edi
0175:    5E                      pop      esi
0176:    5B                      pop      ebx
0177:    5D                      pop      ebp
0178:    C2 08 00                ret      8
