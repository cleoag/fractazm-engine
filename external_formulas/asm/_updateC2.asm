; Disassembly: _updateC2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 265 bytes, decoded: 265 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 01                   fld      qword ptr [ecx]
0010:    DD 02                   fld      qword ptr [edx]
0012:    DD 03                   fld      qword ptr [ebx]
0014:    D9 C0                   fld      st(0)
0016:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0019:    D9 C2                   fld      st(2)
001B:    D8 4F D8                fmul     dword ptr [edi - 0x28]
001E:    DE C1                   faddp    st(1)
0020:    D9 C3                   fld      st(3)
0022:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0025:    DE C1                   faddp    st(1)
0027:    D9 C1                   fld      st(1)
0029:    D8 4F F4                fmul     dword ptr [edi - 0xc]
002C:    D9 C3                   fld      st(3)
002E:    D8 4F F0                fmul     dword ptr [edi - 0x10]
0031:    DE C1                   faddp    st(1)
0033:    D9 C4                   fld      st(4)
0035:    D8 4F EC                fmul     dword ptr [edi - 0x14]
0038:    DE C1                   faddp    st(1)
003A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003C:    D8 4F E8                fmul     dword ptr [edi - 0x18]
003F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0041:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
0044:    DE C3                   faddp    st(3)
0046:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0048:    D8 4F E0                fmul     dword ptr [edi - 0x20]
004B:    DE C2                   faddp    st(2)
004D:    DC 4F CC                fmul     qword ptr [edi - 0x34]
0050:    DC 47 C4                fadd     qword ptr [edi - 0x3c]
0053:    DD 1B                   fstp     qword ptr [ebx]
0055:    DC 4F CC                fmul     qword ptr [edi - 0x34]
0058:    DC 47 BC                fadd     qword ptr [edi - 0x44]
005B:    DD 1A                   fstp     qword ptr [edx]
005D:    DC 4F CC                fmul     qword ptr [edi - 0x34]
0060:    DC 47 B4                fadd     qword ptr [edi - 0x4c]
0063:    DD 19                   fstp     qword ptr [ecx]
0065:    DD 41 08                fld      qword ptr [ecx + 8]
0068:    DC 4F CC                fmul     qword ptr [edi - 0x34]
006B:    DD 59 08                fstp     qword ptr [ecx + 8]
006E:    90                      nop
006F:    DD 03                   fld      qword ptr [ebx]
0071:    DD 46 18                fld      qword ptr [esi + 0x18]
0074:    D8 D1                   fcom     st(1)
0076:    9B                      wait
0077:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0079:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
007B:    76 02                   jbe      0x7f    ; Conditional jump
007D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
007F:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0082:    DD 1B                   fstp     qword ptr [ebx]
0084:    90                      nop
0085:    DD 02                   fld      qword ptr [edx]
0087:    DD 46 20                fld      qword ptr [esi + 0x20]
008A:    D8 D1                   fcom     st(1)
008C:    9B                      wait
008D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
008F:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0091:    76 02                   jbe      0x95    ; Conditional jump
0093:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0095:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0098:    DD 1A                   fstp     qword ptr [edx]
009A:    90                      nop
009B:    DD 01                   fld      qword ptr [ecx]
009D:    DD 46 28                fld      qword ptr [esi + 0x28]
00A0:    D8 D1                   fcom     st(1)
00A2:    9B                      wait
00A3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A5:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A7:    76 02                   jbe      0xab    ; Conditional jump
00A9:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00AB:    DD 5E 28                fstp     qword ptr [esi + 0x28]
00AE:    DD 19                   fstp     qword ptr [ecx]
00B0:    90                      nop
00B1:    DD 01                   fld      qword ptr [ecx]
00B3:    DD 02                   fld      qword ptr [edx]
00B5:    DD 03                   fld      qword ptr [ebx]
00B7:    D9 C0                   fld      st(0)
00B9:    D8 4F EC                fmul     dword ptr [edi - 0x14]
00BC:    D9 C2                   fld      st(2)
00BE:    D8 4F E0                fmul     dword ptr [edi - 0x20]
00C1:    DE C1                   faddp    st(1)
00C3:    D9 C3                   fld      st(3)
00C5:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00C8:    DE C1                   faddp    st(1)
00CA:    D9 C1                   fld      st(1)
00CC:    D8 4F F4                fmul     dword ptr [edi - 0xc]
00CF:    D9 C3                   fld      st(3)
00D1:    D8 4F E8                fmul     dword ptr [edi - 0x18]
00D4:    DE C1                   faddp    st(1)
00D6:    D9 C4                   fld      st(4)
00D8:    D8 4F DC                fmul     dword ptr [edi - 0x24]
00DB:    DE C1                   faddp    st(1)
00DD:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00DF:    D8 4F F0                fmul     dword ptr [edi - 0x10]
00E2:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E4:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
00E7:    DE C3                   faddp    st(3)
00E9:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00EB:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00EE:    DE C2                   faddp    st(2)
00F0:    DC 67 C4                fsub     qword ptr [edi - 0x3c]
00F3:    DD 1B                   fstp     qword ptr [ebx]
00F5:    DC 67 BC                fsub     qword ptr [edi - 0x44]
00F8:    DD 1A                   fstp     qword ptr [edx]
00FA:    DC 67 B4                fsub     qword ptr [edi - 0x4c]
00FD:    DD 19                   fstp     qword ptr [ecx]
00FF:    90                      nop
0100:    89 D8                   mov      eax, ebx
0102:    5B                      pop      ebx
0103:    5F                      pop      edi
0104:    5E                      pop      esi
0105:    5D                      pop      ebp
0106:    C2 08 00                ret      8
