; Disassembly: _HexaTransform.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 144 bytes, decoded: 144 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 00                   fld      qword ptr [eax]
000D:    D9 E1                   fabs    ; |ST(0)|
000F:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0012:    90                      nop
0013:    DD 02                   fld      qword ptr [edx]
0015:    DC 0F                   fmul     qword ptr [edi]
0017:    DD 00                   fld      qword ptr [eax]
0019:    DC 4F 08                fmul     qword ptr [edi + 8]
001C:    DE C1                   faddp    st(1)
001E:    D9 E1                   fabs    ; |ST(0)|
0020:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0023:    90                      nop
0024:    DD 01                   fld      qword ptr [ecx]
0026:    D9 E1                   fabs    ; |ST(0)|
0028:    DC 67 F0                fsub     qword ptr [edi - 0x10]
002B:    90                      nop
002C:    DD 02                   fld      qword ptr [edx]
002E:    DC 0F                   fmul     qword ptr [edi]
0030:    DD 00                   fld      qword ptr [eax]
0032:    DC 4F 08                fmul     qword ptr [edi + 8]
0035:    DE E1                   fsubrp   st(1)
0037:    D9 E1                   fabs    ; |ST(0)|
0039:    DC 67 F0                fsub     qword ptr [edi - 0x10]
003C:    90                      nop
003D:    DD 59 08                fstp     qword ptr [ecx + 8]
0040:    DD 19                   fstp     qword ptr [ecx]
0042:    DD 1A                   fstp     qword ptr [edx]
0044:    DD 18                   fstp     qword ptr [eax]
0046:    DD 46 18                fld      qword ptr [esi + 0x18]
0049:    D9 E1                   fabs    ; |ST(0)|
004B:    DC 67 F0                fsub     qword ptr [edi - 0x10]
004E:    90                      nop
004F:    DD 46 20                fld      qword ptr [esi + 0x20]
0052:    DC 0F                   fmul     qword ptr [edi]
0054:    DD 46 18                fld      qword ptr [esi + 0x18]
0057:    DC 4F 08                fmul     qword ptr [edi + 8]
005A:    DE C1                   faddp    st(1)
005C:    D9 E1                   fabs    ; |ST(0)|
005E:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0061:    90                      nop
0062:    DD 46 28                fld      qword ptr [esi + 0x28]
0065:    D9 E1                   fabs    ; |ST(0)|
0067:    DC 67 F0                fsub     qword ptr [edi - 0x10]
006A:    90                      nop
006B:    DD 46 20                fld      qword ptr [esi + 0x20]
006E:    DC 0F                   fmul     qword ptr [edi]
0070:    DD 46 18                fld      qword ptr [esi + 0x18]
0073:    DC 4F 08                fmul     qword ptr [edi + 8]
0076:    DE E1                   fsubrp   st(1)
0078:    D9 E1                   fabs    ; |ST(0)|
007A:    DC 67 F0                fsub     qword ptr [edi - 0x10]
007D:    90                      nop
007E:    DD 5E C8                fstp     qword ptr [esi - 0x38]
0081:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0084:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0087:    DD 5E 18                fstp     qword ptr [esi + 0x18]
008A:    5F                      pop      edi
008B:    5E                      pop      esi
008C:    5D                      pop      ebp
008D:    C2 08 00                ret      8
