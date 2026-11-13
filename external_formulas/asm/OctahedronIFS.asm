; Disassembly: OctahedronIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 237 bytes, decoded: 237 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    89 C3                   mov      ebx, eax
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 01                   fld      qword ptr [ecx]
000F:    DD 02                   fld      qword ptr [edx]
0011:    DD 03                   fld      qword ptr [ebx]
0013:    D9 C0                   fld      st(0)
0015:    D8 4E BC                fmul     dword ptr [esi - 0x44]
0018:    D9 C2                   fld      st(2)
001A:    D8 4E B8                fmul     dword ptr [esi - 0x48]
001D:    DE C1                   faddp    st(1)
001F:    D9 C3                   fld      st(3)
0021:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
0024:    DE C1                   faddp    st(1)
0026:    D9 C1                   fld      st(1)
0028:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
002B:    D9 C3                   fld      st(3)
002D:    D8 4E D0                fmul     dword ptr [esi - 0x30]
0030:    DE C1                   faddp    st(1)
0032:    D9 C4                   fld      st(4)
0034:    D8 4E CC                fmul     dword ptr [esi - 0x34]
0037:    DE C1                   faddp    st(1)
0039:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003B:    D8 4E C8                fmul     dword ptr [esi - 0x38]
003E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0040:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
0043:    DE C3                   faddp    st(3)
0045:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0047:    D8 4E C0                fmul     dword ptr [esi - 0x40]
004A:    DE C2                   faddp    st(2)
004C:    D8 D1                   fcom     st(1)
004E:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0050:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0052:    73 02                   jae      0x56    ; Conditional jump
0054:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0056:    D9 E0                   fchs    ; -ST(0)
0058:    D8 D1                   fcom     st(1)
005A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005E:    72 02                   jb       0x62    ; Conditional jump
0060:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0062:    D9 E0                   fchs    ; -ST(0)
0064:    D8 D2                   fcom     st(2)
0066:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0068:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006A:    73 02                   jae      0x6e    ; Conditional jump
006C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
006E:    D9 E0                   fchs    ; -ST(0)
0070:    D8 D2                   fcom     st(2)
0072:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0074:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0076:    72 02                   jb       0x7a    ; Conditional jump
0078:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007A:    D9 E0                   fchs    ; -ST(0)
007C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
007E:    D9 C1                   fld      st(1)
0080:    D8 4E 98                fmul     dword ptr [esi - 0x68]
0083:    D9 C1                   fld      st(1)
0085:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
0088:    DE C1                   faddp    st(1)
008A:    D9 C3                   fld      st(3)
008C:    D8 4E 90                fmul     dword ptr [esi - 0x70]
008F:    DE C1                   faddp    st(1)
0091:    D9 C2                   fld      st(2)
0093:    D8 4E B0                fmul     dword ptr [esi - 0x50]
0096:    D9 C2                   fld      st(2)
0098:    D8 4E AC                fmul     dword ptr [esi - 0x54]
009B:    DE C1                   faddp    st(1)
009D:    D9 C4                   fld      st(4)
009F:    D8 4E A8                fmul     dword ptr [esi - 0x58]
00A2:    DE C1                   faddp    st(1)
00A4:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A6:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
00A9:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00AB:    D8 4E A0                fmul     dword ptr [esi - 0x60]
00AE:    DE C2                   faddp    st(2)
00B0:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00B2:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
00B5:    DE C1                   faddp    st(1)
00B7:    DD 46 F0                fld      qword ptr [esi - 0x10]
00BA:    DD 41 08                fld      qword ptr [ecx + 8]
00BD:    D8 C9                   fmul     st(1)
00BF:    DD 59 08                fstp     qword ptr [ecx + 8]
00C2:    DC CB                   fmul     st(3), st(0)
00C4:    DC CA                   fmul     st(2), st(0)
00C6:    DC C9                   fmul     st(1), st(0)
00C8:    D9 E8                   fld1    ; Load 1.0
00CA:    DE E9                   fsubp    st(1)
00CC:    D9 C0                   fld      st(0)
00CE:    D9 C0                   fld      st(0)
00D0:    DC 4E E8                fmul     qword ptr [esi - 0x18]
00D3:    DE EC                   fsubp    st(4)
00D5:    DC 4E E0                fmul     qword ptr [esi - 0x20]
00D8:    DE EA                   fsubp    st(2)
00DA:    DC 4E D8                fmul     qword ptr [esi - 0x28]
00DD:    DE EB                   fsubp    st(3)
00DF:    DD 1A                   fstp     qword ptr [edx]
00E1:    DD 1B                   fstp     qword ptr [ebx]
00E3:    DD 19                   fstp     qword ptr [ecx]
00E5:    89 D8                   mov      eax, ebx
00E7:    5E                      pop      esi
00E8:    5B                      pop      ebx
00E9:    5D                      pop      ebp
00EA:    C2 08 00                ret      8
