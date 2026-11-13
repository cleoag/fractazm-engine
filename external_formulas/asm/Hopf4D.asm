; Disassembly: Hopf4D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 163 bytes, decoded: 163 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    81 EC 30 00 00 00       sub      esp, 0x30
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
001E:    DE C1                   faddp    st(1)
0020:    90                      nop
0021:    DD 01                   fld      qword ptr [ecx]
0023:    D8 C8                   fmul     st(0)
0025:    DD 41 08                fld      qword ptr [ecx + 8]
0028:    D8 C8                   fmul     st(0)
002A:    DE C1                   faddp    st(1)
002C:    90                      nop
002D:    D9 C0                   fld      st(0)
002F:    D8 F2                   fdiv     st(2)
0031:    D9 E0                   fchs    ; -ST(0)
0033:    D9 E8                   fld1    ; Load 1.0
0035:    DE C1                   faddp    st(1)
0037:    DD 5D F8                fstp     qword ptr [ebp - 8]
003A:    90                      nop
003B:    DE F9                   fdivp    st(1)
003D:    D9 FA                   fsqrt    ; sqrt(ST(0))
003F:    D8 C0                   fadd     st(0)
0041:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0044:    90                      nop
0045:    DD 03                   fld      qword ptr [ebx]
0047:    D8 C8                   fmul     st(0)
0049:    DD 02                   fld      qword ptr [edx]
004B:    D8 C8                   fmul     st(0)
004D:    DE E9                   fsubp    st(1)
004F:    DC 4D F8                fmul     qword ptr [ebp - 8]
0052:    90                      nop
0053:    DC 46 18                fadd     qword ptr [esi + 0x18]
0056:    90                      nop
0057:    DD 03                   fld      qword ptr [ebx]
0059:    DC 0A                   fmul     qword ptr [edx]
005B:    DC 4D F8                fmul     qword ptr [ebp - 8]
005E:    D8 C0                   fadd     st(0)
0060:    90                      nop
0061:    DC 46 20                fadd     qword ptr [esi + 0x20]
0064:    90                      nop
0065:    DD 01                   fld      qword ptr [ecx]
0067:    D8 C8                   fmul     st(0)
0069:    DD 41 08                fld      qword ptr [ecx + 8]
006C:    D8 C8                   fmul     st(0)
006E:    DE E9                   fsubp    st(1)
0070:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0073:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0076:    DC 46 28                fadd     qword ptr [esi + 0x28]
0079:    90                      nop
007A:    DD 01                   fld      qword ptr [ecx]
007C:    DC 49 08                fmul     qword ptr [ecx + 8]
007F:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0082:    90                      nop
0083:    DC 46 C8                fadd     qword ptr [esi - 0x38]
0086:    DC 47 F0                fadd     qword ptr [edi - 0x10]
0089:    90                      nop
008A:    DD 59 08                fstp     qword ptr [ecx + 8]
008D:    DD 19                   fstp     qword ptr [ecx]
008F:    DD 1A                   fstp     qword ptr [edx]
0091:    DD 1B                   fstp     qword ptr [ebx]
0093:    90                      nop
0094:    90                      nop
0095:    90                      nop
0096:    D9 D0                   fnop
0098:    8B C3                   mov      eax, ebx
009A:    5F                      pop      edi
009B:    5E                      pop      esi
009C:    5B                      pop      ebx
009D:    89 EC                   mov      esp, ebp
009F:    5D                      pop      ebp
00A0:    C2 08 00                ret      8
