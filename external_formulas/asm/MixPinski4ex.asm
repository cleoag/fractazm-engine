; Disassembly: MixPinski4ex.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 378 bytes, decoded: 378 bytes (100.0%)
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
0075:    DD 47 80                fld      qword ptr [edi - 0x80]
0078:    DE C1                   faddp    st(1)
007A:    DD 47 88                fld      qword ptr [edi - 0x78]
007D:    DE C2                   faddp    st(2)
007F:    DD 87 78 FF FF FF       fld      qword ptr [edi - 0x88]
0085:    DE C3                   faddp    st(3)
0087:    DD 87 70 FF FF FF       fld      qword ptr [edi - 0x90]
008D:    DE C4                   faddp    st(4)
008F:    90                      nop
0090:    83 EF 28                sub      edi, 0x28
0093:    D9 C0                   fld      st(0)
0095:    D8 4F F4                fmul     dword ptr [edi - 0xc]
0098:    D9 C2                   fld      st(2)
009A:    D8 4F F0                fmul     dword ptr [edi - 0x10]
009D:    DE C1                   faddp    st(1)
009F:    D9 C3                   fld      st(3)
00A1:    D8 4F EC                fmul     dword ptr [edi - 0x14]
00A4:    DE C1                   faddp    st(1)
00A6:    D9 C4                   fld      st(4)
00A8:    D8 4F E8                fmul     dword ptr [edi - 0x18]
00AB:    DE C1                   faddp    st(1)
00AD:    DD 1B                   fstp     qword ptr [ebx]
00AF:    D9 C0                   fld      st(0)
00B1:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
00B4:    D9 C2                   fld      st(2)
00B6:    D8 4F E0                fmul     dword ptr [edi - 0x20]
00B9:    DE C1                   faddp    st(1)
00BB:    D9 C3                   fld      st(3)
00BD:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00C0:    DE C1                   faddp    st(1)
00C2:    D9 C4                   fld      st(4)
00C4:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00C7:    DE C1                   faddp    st(1)
00C9:    DD 1A                   fstp     qword ptr [edx]
00CB:    D9 C0                   fld      st(0)
00CD:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00D0:    D9 C2                   fld      st(2)
00D2:    D8 4F D0                fmul     dword ptr [edi - 0x30]
00D5:    DE C1                   faddp    st(1)
00D7:    D9 C3                   fld      st(3)
00D9:    D8 4F CC                fmul     dword ptr [edi - 0x34]
00DC:    DE C1                   faddp    st(1)
00DE:    D9 C4                   fld      st(4)
00E0:    D8 4F C8                fmul     dword ptr [edi - 0x38]
00E3:    DE C1                   faddp    st(1)
00E5:    DD 19                   fstp     qword ptr [ecx]
00E7:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
00EA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00EC:    D8 4F C0                fmul     dword ptr [edi - 0x40]
00EF:    DE C1                   faddp    st(1)
00F1:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00F3:    D8 4F BC                fmul     dword ptr [edi - 0x44]
00F6:    DE C1                   faddp    st(1)
00F8:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00FA:    D8 4F B8                fmul     dword ptr [edi - 0x48]
00FD:    DE C1                   faddp    st(1)
00FF:    83 C7 28                add      edi, 0x28
0102:    DD 87 70 FF FF FF       fld      qword ptr [edi - 0x90]
0108:    DE E9                   fsubp    st(1)
010A:    DD 47 F0                fld      qword ptr [edi - 0x10]
010D:    DC C9                   fmul     st(1), st(0)
010F:    D9 E8                   fld1    ; Load 1.0
0111:    DE E9                   fsubp    st(1)
0113:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0116:    DE E9                   fsubp    st(1)
0118:    DD 59 08                fstp     qword ptr [ecx + 8]
011B:    DD 01                   fld      qword ptr [ecx]
011D:    DC A7 78 FF FF FF       fsub     qword ptr [edi - 0x88]
0123:    DD 02                   fld      qword ptr [edx]
0125:    DC 67 88                fsub     qword ptr [edi - 0x78]
0128:    DD 03                   fld      qword ptr [ebx]
012A:    DC 67 80                fsub     qword ptr [edi - 0x80]
012D:    90                      nop
012E:    DD 47 F0                fld      qword ptr [edi - 0x10]
0131:    DD 86 E8 00 00 00       fld      qword ptr [esi + 0xe8]
0137:    D8 C9                   fmul     st(1)
0139:    DD 9E E8 00 00 00       fstp     qword ptr [esi + 0xe8]
013F:    90                      nop
0140:    90                      nop
0141:    DC CA                   fmul     st(2), st(0)
0143:    DC C9                   fmul     st(1), st(0)
0145:    D9 C0                   fld      st(0)
0147:    D9 E8                   fld1    ; Load 1.0
0149:    DE E9                   fsubp    st(1)
014B:    D9 C0                   fld      st(0)
014D:    D9 C0                   fld      st(0)
014F:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0152:    DE ED                   fsubp    st(5)
0154:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0157:    DE EB                   fsubp    st(3)
0159:    D8 F1                   fdiv     st(1)
015B:    DC 4F D8                fmul     qword ptr [edi - 0x28]
015E:    DC EC                   fsub     st(4), st(0)
0160:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
0162:    D9 E1                   fabs    ; |ST(0)|
0164:    DE EC                   fsubp    st(4)
0166:    DE CB                   fmulp    st(3)
0168:    DD 1A                   fstp     qword ptr [edx]
016A:    DD 1B                   fstp     qword ptr [ebx]
016C:    DD 19                   fstp     qword ptr [ecx]
016E:    90                      nop
016F:    90                      nop
0170:    90                      nop
0171:    89 D8                   mov      eax, ebx
0173:    5F                      pop      edi
0174:    5E                      pop      esi
0175:    5B                      pop      ebx
0176:    5D                      pop      ebp
0177:    C2 08 00                ret      8
