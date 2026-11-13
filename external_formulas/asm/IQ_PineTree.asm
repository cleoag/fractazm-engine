; Disassembly: IQ_PineTree.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 176 bytes, decoded: 176 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 40 00 00 00       sub      esp, 0x40
0009:    53                      push     ebx
000A:    56                      push     esi
000B:    57                      push     edi
000C:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000F:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0012:    8B D8                   mov      ebx, eax
0014:    D9 D0                   fnop
0016:    DD 03                   fld      qword ptr [ebx]
0018:    D8 C8                   fmul     st(0)
001A:    DD 02                   fld      qword ptr [edx]
001C:    D8 C8                   fmul     st(0)
001E:    DD 01                   fld      qword ptr [ecx]
0020:    D8 C8                   fmul     st(0)
0022:    DE C1                   faddp    st(1)
0024:    DE C1                   faddp    st(1)
0026:    D9 FA                   fsqrt    ; sqrt(ST(0))
0028:    DD 55 F8                fst      qword ptr [ebp - 8]
002B:    DD 47 F0                fld      qword ptr [edi - 0x10]
002E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0030:    D9 E1                   fabs    ; |ST(0)|
0032:    D9 F1                   fyl2x
0034:    D9 E8                   fld1    ; Load 1.0
0036:    D9 C1                   fld      st(1)
0038:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
003A:    D9 F0                   f2xm1
003C:    DE C1                   faddp    st(1)
003E:    D9 FD                   fscale
0040:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0042:    DD D8                   fstp     st(0)
0044:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0047:    DD 02                   fld      qword ptr [edx]
0049:    D8 C8                   fmul     st(0)
004B:    DD 01                   fld      qword ptr [ecx]
004D:    D8 C8                   fmul     st(0)
004F:    DE C1                   faddp    st(1)
0051:    D9 FA                   fsqrt    ; sqrt(ST(0))
0053:    DD 55 E8                fst      qword ptr [ebp - 0x18]
0056:    DD 03                   fld      qword ptr [ebx]
0058:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
005A:    DC 4F F0                fmul     qword ptr [edi - 0x10]
005D:    D9 FB                   fsincos    ; ST(0)=cos, push sin
005F:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0062:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0065:    90                      nop
0066:    DD 02                   fld      qword ptr [edx]
0068:    DD 01                   fld      qword ptr [ecx]
006A:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
006C:    DC 4F E8                fmul     qword ptr [edi - 0x18]
006F:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0071:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0074:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0077:    D9 D0                   fnop
0079:    DD 45 D8                fld      qword ptr [ebp - 0x28]
007C:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
007F:    DC 46 18                fadd     qword ptr [esi + 0x18]
0082:    DD 1B                   fstp     qword ptr [ebx]
0084:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0087:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
008A:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
008D:    DC 46 20                fadd     qword ptr [esi + 0x20]
0090:    DD 1A                   fstp     qword ptr [edx]
0092:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0095:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0098:    DC 4F E0                fmul     qword ptr [edi - 0x20]
009B:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
009E:    DC 46 28                fadd     qword ptr [esi + 0x28]
00A1:    DD 19                   fstp     qword ptr [ecx]
00A3:    D9 D0                   fnop
00A5:    8B C3                   mov      eax, ebx
00A7:    5F                      pop      edi
00A8:    5E                      pop      esi
00A9:    5B                      pop      ebx
00AA:    8B E5                   mov      esp, ebp
00AC:    5D                      pop      ebp
00AD:    C2 08 00                ret      8
