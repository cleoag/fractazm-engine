; Disassembly: _reciprocalY3b.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 70 bytes, decoded: 70 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    52                      push     edx
0005:    DD 02                   fld      qword ptr [edx]
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8A 4A 07                mov      cl, byte ptr [edx + 7]
000D:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
0010:    DC 4E D0                fmul     qword ptr [esi - 0x30]
0013:    D9 E1                   fabs    ; |ST(0)|
0015:    DC 46 F0                fadd     qword ptr [esi - 0x10]
0018:    D9 E8                   fld1    ; Load 1.0
001A:    DE F1                   fdivrp   st(1)
001C:    D9 E0                   fchs    ; -ST(0)
001E:    DC 46 E8                fadd     qword ptr [esi - 0x18]
0021:    DD 02                   fld      qword ptr [edx]
0023:    D8 C8                   fmul     st(0)
0025:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0028:    D9 E1                   fabs    ; |ST(0)|
002A:    DC 46 E0                fadd     qword ptr [esi - 0x20]
002D:    80 E1 80                and      cl, 0x80
0030:    D9 E8                   fld1    ; Load 1.0
0032:    DE F1                   fdivrp   st(1)
0034:    D9 E0                   fchs    ; -ST(0)
0036:    DC 46 D8                fadd     qword ptr [esi - 0x28]
0039:    DE C1                   faddp    st(1)
003B:    DD 1A                   fstp     qword ptr [edx]
003D:    30 4A 07                xor      byte ptr [edx + 7], cl
0040:    5A                      pop      edx
0041:    5E                      pop      esi
0042:    5D                      pop      ebp
0043:    C2 08 00                ret      8
