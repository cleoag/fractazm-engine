; Disassembly: Makin-NSR-tri.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 211 bytes, decoded: 211 bytes (100.0%)

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
0016:    DD 02                   fld      qword ptr [edx]
0018:    DD 03                   fld      qword ptr [ebx]
001A:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
001C:    DC 4F F0                fmul     qword ptr [edi - 0x10]
001F:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0021:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0024:    DD 5D F8                fstp     qword ptr [ebp - 8]
0027:    DD 01                   fld      qword ptr [ecx]
0029:    DD 02                   fld      qword ptr [edx]
002B:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
002D:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0030:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0032:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
0035:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0038:    DD 03                   fld      qword ptr [ebx]
003A:    DD 01                   fld      qword ptr [ecx]
003C:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
003E:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0041:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0043:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0046:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0049:    90                      nop
004A:    DD 03                   fld      qword ptr [ebx]
004C:    D8 C8                   fmul     st(0)
004E:    DD 02                   fld      qword ptr [edx]
0050:    D8 C8                   fmul     st(0)
0052:    DD 01                   fld      qword ptr [ecx]
0054:    D8 C8                   fmul     st(0)
0056:    DE C1                   faddp    st(1)
0058:    DE C1                   faddp    st(1)
005A:    D9 FA                   fsqrt    ; sqrt(ST(0))
005C:    DD 47 F0                fld      qword ptr [edi - 0x10]
005F:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0061:    D9 E1                   fabs    ; |ST(0)|
0063:    D9 F1                   fyl2x
0065:    D9 E8                   fld1    ; Load 1.0
0067:    D9 C1                   fld      st(1)
0069:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
006B:    D9 F0                   f2xm1
006D:    DE C1                   faddp    st(1)
006F:    D9 FD                   fscale
0071:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0073:    DD D8                   fstp     st(0)
0075:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
0078:    90                      nop
0079:    90                      nop
007A:    DD 45 E8                fld      qword ptr [ebp - 0x18]
007D:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0080:    DD 19                   fstp     qword ptr [ecx]
0082:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0085:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0088:    DD 1B                   fstp     qword ptr [ebx]
008A:    DD 45 F8                fld      qword ptr [ebp - 8]
008D:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
0090:    DD 1A                   fstp     qword ptr [edx]
0092:    DD 03                   fld      qword ptr [ebx]
0094:    D8 C8                   fmul     st(0)
0096:    DD 02                   fld      qword ptr [edx]
0098:    D8 C8                   fmul     st(0)
009A:    DD 01                   fld      qword ptr [ecx]
009C:    D8 C8                   fmul     st(0)
009E:    DE C1                   faddp    st(1)
00A0:    DE C1                   faddp    st(1)
00A2:    D9 FA                   fsqrt    ; sqrt(ST(0))
00A4:    DC 7D C8                fdivr    qword ptr [ebp - 0x38]
00A7:    90                      nop
00A8:    DD 03                   fld      qword ptr [ebx]
00AA:    D8 C9                   fmul     st(1)
00AC:    DC 46 18                fadd     qword ptr [esi + 0x18]
00AF:    DD 1B                   fstp     qword ptr [ebx]
00B1:    DD 02                   fld      qword ptr [edx]
00B3:    D8 C9                   fmul     st(1)
00B5:    DC 46 20                fadd     qword ptr [esi + 0x20]
00B8:    DD 1A                   fstp     qword ptr [edx]
00BA:    DD 01                   fld      qword ptr [ecx]
00BC:    DE C9                   fmulp    st(1)
00BE:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00C1:    DC 46 28                fadd     qword ptr [esi + 0x28]
00C4:    DD 19                   fstp     qword ptr [ecx]
00C6:    D9 D0                   fnop
00C8:    8B C3                   mov      eax, ebx
00CA:    5F                      pop      edi
00CB:    5E                      pop      esi
00CC:    5B                      pop      ebx
00CD:    89 EC                   mov      esp, ebp
00CF:    5D                      pop      ebp
00D0:    C2 08 00                ret      8
