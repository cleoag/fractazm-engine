; Disassembly: Quadrat3D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 166 bytes, decoded: 166 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    50                      push     eax
0005:    56                      push     esi
0006:    57                      push     edi
0007:    90                      nop
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B D8                   mov      ebx, eax
000D:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0010:    D9 D0                   fnop
0012:    D9 D0                   fnop
0014:    DD 03                   fld      qword ptr [ebx]
0016:    D8 C8                   fmul     st(0)
0018:    DD 02                   fld      qword ptr [edx]
001A:    D8 C8                   fmul     st(0)
001C:    DD 47 D8                fld      qword ptr [edi - 0x28]
001F:    DE C1                   faddp    st(1)
0021:    DE C1                   faddp    st(1)
0023:    D9 EE                   fldz    ; Load 0.0
0025:    D8 D9                   fcomp    st(1)
0027:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0029:    9E                      sahf
002A:    77 66                   ja       0x92    ; Conditional jump
002C:    74 6C                   je       0x9a    ; Conditional jump
002E:    90                      nop
002F:    90                      nop
0030:    DD 03                   fld      qword ptr [ebx]
0032:    D8 C8                   fmul     st(0)
0034:    DD 02                   fld      qword ptr [edx]
0036:    D8 C8                   fmul     st(0)
0038:    D9 E0                   fchs    ; -ST(0)
003A:    DE C1                   faddp    st(1)
003C:    DC 4F F0                fmul     qword ptr [edi - 0x10]
003F:    DD 47 D0                fld      qword ptr [edi - 0x30]
0042:    DD 01                   fld      qword ptr [ecx]
0044:    D8 C8                   fmul     st(0)
0046:    D8 F3                   fdiv     st(3)
0048:    D9 E0                   fchs    ; -ST(0)
004A:    DE C1                   faddp    st(1)
004C:    DE C9                   fmulp    st(1)
004E:    DC 46 18                fadd     qword ptr [esi + 0x18]
0051:    D9 D0                   fnop
0053:    DD 03                   fld      qword ptr [ebx]
0055:    DC 0A                   fmul     qword ptr [edx]
0057:    DC 4F E8                fmul     qword ptr [edi - 0x18]
005A:    DD 47 D0                fld      qword ptr [edi - 0x30]
005D:    DD 01                   fld      qword ptr [ecx]
005F:    D8 C8                   fmul     st(0)
0061:    D8 F4                   fdiv     st(4)
0063:    D9 E0                   fchs    ; -ST(0)
0065:    DE C1                   faddp    st(1)
0067:    DE C9                   fmulp    st(1)
0069:    DC 46 20                fadd     qword ptr [esi + 0x20]
006C:    D9 D0                   fnop
006E:    DD 01                   fld      qword ptr [ecx]
0070:    D9 C3                   fld      st(3)
0072:    D9 FA                   fsqrt    ; sqrt(ST(0))
0074:    DE C9                   fmulp    st(1)
0076:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0079:    DC 46 28                fadd     qword ptr [esi + 0x28]
007C:    D9 D0                   fnop
007E:    DD 19                   fstp     qword ptr [ecx]
0080:    DD 1A                   fstp     qword ptr [edx]
0082:    DD 1B                   fstp     qword ptr [ebx]
0084:    DD D8                   fstp     st(0)
0086:    D9 D0                   fnop
0088:    8B C3                   mov      eax, ebx
008A:    5F                      pop      edi
008B:    5E                      pop      esi
008C:    58                      pop      eax
008D:    5B                      pop      ebx
008E:    5D                      pop      ebp
008F:    C2 08 00                ret      8
0092:    D9 EE                   fldz    ; Load 0.0
0094:    D9 EE                   fldz    ; Load 0.0
0096:    D9 EE                   fldz    ; Load 0.0
0098:    EB E2                   jmp      0x7c
009A:    DD 46 18                fld      qword ptr [esi + 0x18]
009D:    DD 46 20                fld      qword ptr [esi + 0x20]
00A0:    DD 46 28                fld      qword ptr [esi + 0x28]
00A3:    EB D7                   jmp      0x7c
00A5:    90                      nop
