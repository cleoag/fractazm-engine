; Disassembly: MandyCousin.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 126 bytes, decoded: 126 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 00                   fld      qword ptr [eax]
000D:    D8 C8                   fmul     st(0)
000F:    DD 02                   fld      qword ptr [edx]
0011:    D8 C8                   fmul     st(0)
0013:    D9 E0                   fchs    ; -ST(0)
0015:    DE C1                   faddp    st(1)
0017:    DD 01                   fld      qword ptr [ecx]
0019:    D8 C8                   fmul     st(0)
001B:    D9 E0                   fchs    ; -ST(0)
001D:    DE C1                   faddp    st(1)
001F:    DD 41 08                fld      qword ptr [ecx + 8]
0022:    D8 C8                   fmul     st(0)
0024:    D9 D0                   fnop
0026:    DE C1                   faddp    st(1)
0028:    DC 4F F0                fmul     qword ptr [edi - 0x10]
002B:    DC 46 18                fadd     qword ptr [esi + 0x18]
002E:    D9 D0                   fnop
0030:    DD 00                   fld      qword ptr [eax]
0032:    DC 0A                   fmul     qword ptr [edx]
0034:    DD 01                   fld      qword ptr [ecx]
0036:    DC 49 08                fmul     qword ptr [ecx + 8]
0039:    DE C1                   faddp    st(1)
003B:    DC 4F E8                fmul     qword ptr [edi - 0x18]
003E:    DC 46 20                fadd     qword ptr [esi + 0x20]
0041:    D9 D0                   fnop
0043:    DD 00                   fld      qword ptr [eax]
0045:    DC 09                   fmul     qword ptr [ecx]
0047:    DC 4F E0                fmul     qword ptr [edi - 0x20]
004A:    DD 02                   fld      qword ptr [edx]
004C:    DC 49 08                fmul     qword ptr [ecx + 8]
004F:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0052:    DE C1                   faddp    st(1)
0054:    DC 46 28                fadd     qword ptr [esi + 0x28]
0057:    D9 D0                   fnop
0059:    DD 00                   fld      qword ptr [eax]
005B:    DC 49 08                fmul     qword ptr [ecx + 8]
005E:    DD 02                   fld      qword ptr [edx]
0060:    DC 09                   fmul     qword ptr [ecx]
0062:    DE C1                   faddp    st(1)
0064:    DC 4F D0                fmul     qword ptr [edi - 0x30]
0067:    DC 46 C8                fadd     qword ptr [esi - 0x38]
006A:    DC 47 C8                fadd     qword ptr [edi - 0x38]
006D:    DD 59 08                fstp     qword ptr [ecx + 8]
0070:    DD 19                   fstp     qword ptr [ecx]
0072:    DD 1A                   fstp     qword ptr [edx]
0074:    DD 18                   fstp     qword ptr [eax]
0076:    90                      nop
0077:    90                      nop
0078:    5F                      pop      edi
0079:    5E                      pop      esi
007A:    5D                      pop      ebp
007B:    C2 08 00                ret      8
