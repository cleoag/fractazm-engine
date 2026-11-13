; Disassembly: _FoldingVar.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 145 bytes, decoded: 145 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 01                   fld      qword ptr [ecx]
000D:    DD 02                   fld      qword ptr [edx]
000F:    DD 00                   fld      qword ptr [eax]
0011:    D9 C0                   fld      st(0)
0013:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0016:    D9 C2                   fld      st(2)
0018:    DC 4F E0                fmul     qword ptr [edi - 0x20]
001B:    DE C1                   faddp    st(1)
001D:    D9 C3                   fld      st(3)
001F:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0022:    DE C1                   faddp    st(1)
0024:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0027:    D9 C1                   fld      st(1)
0029:    D8 E1                   fsub     st(1)
002B:    D9 E1                   fabs    ; |ST(0)|
002D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002F:    D8 C2                   fadd     st(2)
0031:    D9 E1                   fabs    ; |ST(0)|
0033:    DE E1                   fsubrp   st(1)
0035:    D8 E1                   fsub     st(1)
0037:    D9 C1                   fld      st(1)
0039:    DC 4F C8                fmul     qword ptr [edi - 0x38]
003C:    D9 C3                   fld      st(3)
003E:    DC 4F C0                fmul     qword ptr [edi - 0x40]
0041:    DE C1                   faddp    st(1)
0043:    D9 C4                   fld      st(4)
0045:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0048:    DE C1                   faddp    st(1)
004A:    DC 47 D0                fadd     qword ptr [edi - 0x30]
004D:    D9 C3                   fld      st(3)
004F:    D8 E1                   fsub     st(1)
0051:    D9 E1                   fabs    ; |ST(0)|
0053:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0055:    D8 C4                   fadd     st(4)
0057:    D9 E1                   fabs    ; |ST(0)|
0059:    DE E1                   fsubrp   st(1)
005B:    D8 E3                   fsub     st(3)
005D:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
005F:    DC 4F A8                fmul     qword ptr [edi - 0x58]
0062:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0064:    DC 4F A0                fmul     qword ptr [edi - 0x60]
0067:    DE C3                   faddp    st(3)
0069:    D9 C3                   fld      st(3)
006B:    DC 4F 98                fmul     qword ptr [edi - 0x68]
006E:    DC 47 B0                fadd     qword ptr [edi - 0x50]
0071:    DE C3                   faddp    st(3)
0073:    D9 C2                   fld      st(2)
0075:    D8 E4                   fsub     st(4)
0077:    D9 E1                   fabs    ; |ST(0)|
0079:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
007B:    D8 C4                   fadd     st(4)
007D:    D9 E1                   fabs    ; |ST(0)|
007F:    DE E3                   fsubrp   st(3)
0081:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0083:    DE EA                   fsubp    st(2)
0085:    DD 1A                   fstp     qword ptr [edx]
0087:    DD 19                   fstp     qword ptr [ecx]
0089:    DD 18                   fstp     qword ptr [eax]
008B:    5F                      pop      edi
008C:    5E                      pop      esi
008D:    5D                      pop      ebp
008E:    C2 08 00                ret      8
