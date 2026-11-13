; Disassembly: _RotatedAbs.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 157 bytes, decoded: 157 bytes (100.0%)
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
004D:    DC 47 CC                fadd     qword ptr [edi - 0x34]
0050:    D9 E1                   fabs    ; |ST(0)|
0052:    DC 67 C4                fsub     qword ptr [edi - 0x3c]
0055:    D9 C0                   fld      st(0)
0057:    D8 4F EC                fmul     dword ptr [edi - 0x14]
005A:    D9 C2                   fld      st(2)
005C:    D8 4F E0                fmul     dword ptr [edi - 0x20]
005F:    DE C1                   faddp    st(1)
0061:    D9 C3                   fld      st(3)
0063:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0066:    DE C1                   faddp    st(1)
0068:    D9 C1                   fld      st(1)
006A:    D8 4F F4                fmul     dword ptr [edi - 0xc]
006D:    D9 C3                   fld      st(3)
006F:    D8 4F E8                fmul     dword ptr [edi - 0x18]
0072:    DE C1                   faddp    st(1)
0074:    D9 C4                   fld      st(4)
0076:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0079:    DE C1                   faddp    st(1)
007B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007D:    D8 4F F0                fmul     dword ptr [edi - 0x10]
0080:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0082:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
0085:    DE C3                   faddp    st(3)
0087:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0089:    D8 4F D8                fmul     dword ptr [edi - 0x28]
008C:    DE C2                   faddp    st(2)
008E:    DD 1B                   fstp     qword ptr [ebx]
0090:    DD 1A                   fstp     qword ptr [edx]
0092:    DD 19                   fstp     qword ptr [ecx]
0094:    89 D8                   mov      eax, ebx
0096:    5B                      pop      ebx
0097:    5F                      pop      edi
0098:    5E                      pop      esi
0099:    5D                      pop      ebp
009A:    C2 08 00                ret      8
