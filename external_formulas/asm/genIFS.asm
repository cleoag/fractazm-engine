; Disassembly: genIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 143 bytes, decoded: 143 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 01                   fld      qword ptr [ecx]
000D:    83 7F E4 00             cmp      dword ptr [edi - 0x1c], 0
0011:    74 02                   je       0x15    ; Conditional jump
0013:    D9 E1                   fabs    ; |ST(0)|
0015:    D8 47 D8                fadd     dword ptr [edi - 0x28]
0018:    DD 02                   fld      qword ptr [edx]
001A:    83 7F E8 00             cmp      dword ptr [edi - 0x18], 0
001E:    74 02                   je       0x22    ; Conditional jump
0020:    D9 E1                   fabs    ; |ST(0)|
0022:    D8 47 DC                fadd     dword ptr [edi - 0x24]
0025:    DD 00                   fld      qword ptr [eax]
0027:    83 7F EC 00             cmp      dword ptr [edi - 0x14], 0
002B:    74 02                   je       0x2f    ; Conditional jump
002D:    D9 E1                   fabs    ; |ST(0)|
002F:    D8 47 E0                fadd     dword ptr [edi - 0x20]
0032:    D9 C0                   fld      st(0)
0034:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0037:    D9 C2                   fld      st(2)
0039:    D8 4F B8                fmul     dword ptr [edi - 0x48]
003C:    DE C1                   faddp    st(1)
003E:    D9 C3                   fld      st(3)
0040:    D8 4F B4                fmul     dword ptr [edi - 0x4c]
0043:    DE C1                   faddp    st(1)
0045:    D9 C1                   fld      st(1)
0047:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
004A:    D9 C3                   fld      st(3)
004C:    D8 4F D0                fmul     dword ptr [edi - 0x30]
004F:    DE C1                   faddp    st(1)
0051:    D9 C4                   fld      st(4)
0053:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0056:    DE C1                   faddp    st(1)
0058:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
005A:    D8 4F C8                fmul     dword ptr [edi - 0x38]
005D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
005F:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0062:    DE C3                   faddp    st(3)
0064:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0066:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0069:    DE C2                   faddp    st(2)
006B:    DD 47 F0                fld      qword ptr [edi - 0x10]
006E:    DC CB                   fmul     st(3), st(0)
0070:    DC CA                   fmul     st(2), st(0)
0072:    DC C9                   fmul     st(1), st(0)
0074:    DC 49 08                fmul     qword ptr [ecx + 8]
0077:    DD 59 08                fstp     qword ptr [ecx + 8]
007A:    DC 46 18                fadd     qword ptr [esi + 0x18]
007D:    DD 18                   fstp     qword ptr [eax]
007F:    DC 46 20                fadd     qword ptr [esi + 0x20]
0082:    DD 1A                   fstp     qword ptr [edx]
0084:    DC 46 28                fadd     qword ptr [esi + 0x28]
0087:    DD 19                   fstp     qword ptr [ecx]
0089:    5F                      pop      edi
008A:    5E                      pop      esi
008B:    5D                      pop      ebp
008C:    C2 08 00                ret      8
