; Disassembly: Makin4D p2b.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 182 bytes, decoded: 182 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    57                      push     edi
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000C:    8B D8                   mov      ebx, eax
000E:    DD 03                   fld      qword ptr [ebx]
0010:    D8 C8                   fmul     st(0)
0012:    DD 02                   fld      qword ptr [edx]
0014:    D8 C8                   fmul     st(0)
0016:    DE C1                   faddp    st(1)
0018:    90                      nop
0019:    90                      nop
001A:    DD 01                   fld      qword ptr [ecx]
001C:    D8 C8                   fmul     st(0)
001E:    DD 41 08                fld      qword ptr [ecx + 8]
0021:    D8 C8                   fmul     st(0)
0023:    DE C1                   faddp    st(1)
0025:    90                      nop
0026:    90                      nop
0027:    DE C1                   faddp    st(1)
0029:    90                      nop
002A:    DD 03                   fld      qword ptr [ebx]
002C:    DC 09                   fmul     qword ptr [ecx]
002E:    DD 02                   fld      qword ptr [edx]
0030:    DC 49 08                fmul     qword ptr [ecx + 8]
0033:    DE E9                   fsubp    st(1)
0035:    D8 C0                   fadd     st(0)
0037:    90                      nop
0038:    DD 03                   fld      qword ptr [ebx]
003A:    DC 49 08                fmul     qword ptr [ecx + 8]
003D:    DD 02                   fld      qword ptr [edx]
003F:    DC 09                   fmul     qword ptr [ecx]
0041:    DE C1                   faddp    st(1)
0043:    D8 C0                   fadd     st(0)
0045:    90                      nop
0046:    DD 03                   fld      qword ptr [ebx]
0048:    D8 C8                   fmul     st(0)
004A:    DD 02                   fld      qword ptr [edx]
004C:    D8 C8                   fmul     st(0)
004E:    DE E9                   fsubp    st(1)
0050:    D8 C0                   fadd     st(0)
0052:    90                      nop
0053:    DD 03                   fld      qword ptr [ebx]
0055:    DC 0A                   fmul     qword ptr [edx]
0057:    D8 C0                   fadd     st(0)
0059:    D8 C0                   fadd     st(0)
005B:    DD 1A                   fstp     qword ptr [edx]
005D:    DD 1B                   fstp     qword ptr [ebx]
005F:    DD 03                   fld      qword ptr [ebx]
0061:    D8 C8                   fmul     st(0)
0063:    DD 02                   fld      qword ptr [edx]
0065:    D8 C8                   fmul     st(0)
0067:    DE C1                   faddp    st(1)
0069:    D9 FA                   fsqrt    ; sqrt(ST(0))
006B:    D8 FB                   fdivr    st(3)
006D:    D9 E0                   fchs    ; -ST(0)
006F:    D9 E8                   fld1    ; Load 1.0
0071:    DE C1                   faddp    st(1)
0073:    DD 03                   fld      qword ptr [ebx]
0075:    D8 C9                   fmul     st(1)
0077:    DD 1B                   fstp     qword ptr [ebx]
0079:    DD 02                   fld      qword ptr [edx]
007B:    DE C9                   fmulp    st(1)
007D:    DD 1A                   fstp     qword ptr [edx]
007F:    DD 59 08                fstp     qword ptr [ecx + 8]
0082:    DD 19                   fstp     qword ptr [ecx]
0084:    DD D8                   fstp     st(0)
0086:    D9 D0                   fnop
0088:    DD 46 18                fld      qword ptr [esi + 0x18]
008B:    DC 03                   fadd     qword ptr [ebx]
008D:    DD 1B                   fstp     qword ptr [ebx]
008F:    DD 46 20                fld      qword ptr [esi + 0x20]
0092:    DC 02                   fadd     qword ptr [edx]
0094:    DD 1A                   fstp     qword ptr [edx]
0096:    DD 46 28                fld      qword ptr [esi + 0x28]
0099:    DC 01                   fadd     qword ptr [ecx]
009B:    DD 19                   fstp     qword ptr [ecx]
009D:    DD 46 C8                fld      qword ptr [esi - 0x38]
00A0:    DC 47 F0                fadd     qword ptr [edi - 0x10]
00A3:    DC 41 08                fadd     qword ptr [ecx + 8]
00A6:    DD 59 08                fstp     qword ptr [ecx + 8]
00A9:    D9 D0                   fnop
00AB:    8B C3                   mov      eax, ebx
00AD:    5F                      pop      edi
00AE:    5E                      pop      esi
00AF:    5B                      pop      ebx
00B0:    89 EC                   mov      esp, ebp
00B2:    5D                      pop      ebp
00B3:    C2 08 00                ret      8
