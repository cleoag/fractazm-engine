; Disassembly: _RotatedFolding.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 173 bytes, decoded: 173 bytes (100.0%)
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
004D:    D9 C0                   fld      st(0)
004F:    DC 47 CC                fadd     qword ptr [edi - 0x34]
0052:    D9 E1                   fabs    ; |ST(0)|
0054:    D9 C1                   fld      st(1)
0056:    DC 67 CC                fsub     qword ptr [edi - 0x34]
0059:    D9 E1                   fabs    ; |ST(0)|
005B:    D9 E0                   fchs    ; -ST(0)
005D:    DE C1                   faddp    st(1)
005F:    DE E9                   fsubp    st(1)
0061:    D9 E0                   fchs    ; -ST(0)
0063:    D9 D0                   fnop
0065:    D9 C0                   fld      st(0)
0067:    D8 4F EC                fmul     dword ptr [edi - 0x14]
006A:    D9 C2                   fld      st(2)
006C:    D8 4F E0                fmul     dword ptr [edi - 0x20]
006F:    DE C1                   faddp    st(1)
0071:    D9 C3                   fld      st(3)
0073:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0076:    DE C1                   faddp    st(1)
0078:    D9 C1                   fld      st(1)
007A:    D8 4F F4                fmul     dword ptr [edi - 0xc]
007D:    D9 C3                   fld      st(3)
007F:    D8 4F E8                fmul     dword ptr [edi - 0x18]
0082:    DE C1                   faddp    st(1)
0084:    D9 C4                   fld      st(4)
0086:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0089:    DE C1                   faddp    st(1)
008B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
008D:    D8 4F F0                fmul     dword ptr [edi - 0x10]
0090:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0092:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
0095:    DE C3                   faddp    st(3)
0097:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0099:    D8 4F D8                fmul     dword ptr [edi - 0x28]
009C:    DE C2                   faddp    st(2)
009E:    DD 1B                   fstp     qword ptr [ebx]
00A0:    DD 1A                   fstp     qword ptr [edx]
00A2:    DD 19                   fstp     qword ptr [ecx]
00A4:    89 D8                   mov      eax, ebx
00A6:    5B                      pop      ebx
00A7:    5F                      pop      edi
00A8:    5E                      pop      esi
00A9:    5D                      pop      ebp
00AA:    C2 08 00                ret      8
