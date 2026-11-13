; Disassembly: Beth641.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 207 bytes, decoded: 207 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 41 08                fld      qword ptr [ecx + 8]
000E:    D8 C8                   fmul     st(0)
0010:    DD 01                   fld      qword ptr [ecx]
0012:    D8 C8                   fmul     st(0)
0014:    D9 C0                   fld      st(0)
0016:    D8 C8                   fmul     st(0)
0018:    DD 02                   fld      qword ptr [edx]
001A:    D8 C8                   fmul     st(0)
001C:    D9 C0                   fld      st(0)
001E:    D8 C8                   fmul     st(0)
0020:    DD 00                   fld      qword ptr [eax]
0022:    D8 C8                   fmul     st(0)
0024:    D9 C5                   fld      st(5)
0026:    D8 C5                   fadd     st(5)
0028:    D8 C3                   fadd     st(3)
002A:    D8 C9                   fmul     st(1)
002C:    DC 4F 10                fmul     qword ptr [edi + 0x10]
002F:    D9 C6                   fld      st(6)
0031:    D8 CF                   fmul     st(7)
0033:    D8 C3                   fadd     st(3)
0035:    D8 C5                   fadd     st(5)
0037:    DC 4F 08                fmul     qword ptr [edi + 8]
003A:    DE C1                   faddp    st(1)
003C:    D9 C1                   fld      st(1)
003E:    D8 CA                   fmul     st(2)
0040:    DC 0F                   fmul     qword ptr [edi]
0042:    DE C1                   faddp    st(1)
0044:    DC 08                   fmul     qword ptr [eax]
0046:    DC 46 18                fadd     qword ptr [esi + 0x18]
0049:    DD 18                   fstp     qword ptr [eax]
004B:    D9 C5                   fld      st(5)
004D:    D8 CE                   fmul     st(6)
004F:    D9 C1                   fld      st(1)
0051:    D8 CA                   fmul     st(2)
0053:    DE C1                   faddp    st(1)
0055:    D8 C4                   fadd     st(4)
0057:    DC 4F 08                fmul     qword ptr [edi + 8]
005A:    D9 C6                   fld      st(6)
005C:    D8 C6                   fadd     st(6)
005E:    D8 C2                   fadd     st(2)
0060:    D8 CC                   fmul     st(4)
0062:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0065:    DE C1                   faddp    st(1)
0067:    D9 C2                   fld      st(2)
0069:    DC 0F                   fmul     qword ptr [edi]
006B:    DE C1                   faddp    st(1)
006D:    DC 0A                   fmul     qword ptr [edx]
006F:    DC 46 20                fadd     qword ptr [esi + 0x20]
0072:    DD 1A                   fstp     qword ptr [edx]
0074:    D9 C5                   fld      st(5)
0076:    D8 CE                   fmul     st(6)
0078:    D9 C1                   fld      st(1)
007A:    D8 CA                   fmul     st(2)
007C:    DE C1                   faddp    st(1)
007E:    D8 C2                   fadd     st(2)
0080:    DC 4F 08                fmul     qword ptr [edi + 8]
0083:    D9 C6                   fld      st(6)
0085:    D8 C4                   fadd     st(4)
0087:    D8 C2                   fadd     st(2)
0089:    D8 CE                   fmul     st(6)
008B:    DC 4F 10                fmul     qword ptr [edi + 0x10]
008E:    DE C1                   faddp    st(1)
0090:    D9 C4                   fld      st(4)
0092:    DC 0F                   fmul     qword ptr [edi]
0094:    DE C1                   faddp    st(1)
0096:    DC 09                   fmul     qword ptr [ecx]
0098:    DC 46 28                fadd     qword ptr [esi + 0x28]
009B:    DD 19                   fstp     qword ptr [ecx]
009D:    D9 C0                   fld      st(0)
009F:    D8 C9                   fmul     st(1)
00A1:    DE C2                   faddp    st(2)
00A3:    DE C4                   faddp    st(4)
00A5:    DE C2                   faddp    st(2)
00A7:    DE C2                   faddp    st(2)
00A9:    DC 4F 08                fmul     qword ptr [edi + 8]
00AC:    D9 C2                   fld      st(2)
00AE:    D8 CB                   fmul     st(3)
00B0:    DC 0F                   fmul     qword ptr [edi]
00B2:    DE C1                   faddp    st(1)
00B4:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00B6:    DE C9                   fmulp    st(1)
00B8:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00BB:    DE C1                   faddp    st(1)
00BD:    DC 49 08                fmul     qword ptr [ecx + 8]
00C0:    DC 46 C8                fadd     qword ptr [esi - 0x38]
00C3:    DC 47 F0                fadd     qword ptr [edi - 0x10]
00C6:    DD 59 08                fstp     qword ptr [ecx + 8]
00C9:    5F                      pop      edi
00CA:    5E                      pop      esi
00CB:    5D                      pop      ebp
00CC:    C2 08 00                ret      8
