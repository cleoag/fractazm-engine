; Disassembly: _boxtiling.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 159 bytes, decoded: 159 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    50                      push     eax
0005:    56                      push     esi
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    8B D8                   mov      ebx, eax
000B:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000E:    DD 46 D8                fld      qword ptr [esi - 0x28]
0011:    D9 E4                   ftst
0013:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0015:    D9 D0                   fnop
0017:    9E                      sahf
0018:    DD C0                   ffree    st(0)
001A:    D9 F7                   fincstp
001C:    74 12                   je       0x30    ; Conditional jump
001E:    DD 41 08                fld      qword ptr [ecx + 8]
0021:    D9 C0                   fld      st(0)
0023:    DC 76 D8                fdiv     qword ptr [esi - 0x28]
0026:    D9 FC                   frndint    ; round(ST(0))
0028:    DC 4E D8                fmul     qword ptr [esi - 0x28]
002B:    DE E9                   fsubp    st(1)
002D:    DD 59 08                fstp     qword ptr [ecx + 8]
0030:    DD 46 E0                fld      qword ptr [esi - 0x20]
0033:    D9 E4                   ftst
0035:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0037:    D9 D0                   fnop
0039:    9E                      sahf
003A:    DD C0                   ffree    st(0)
003C:    D9 F7                   fincstp
003E:    74 12                   je       0x52    ; Conditional jump
0040:    DD 01                   fld      qword ptr [ecx]
0042:    90                      nop
0043:    D9 C0                   fld      st(0)
0045:    DC 76 E0                fdiv     qword ptr [esi - 0x20]
0048:    D9 FC                   frndint    ; round(ST(0))
004A:    DC 4E E0                fmul     qword ptr [esi - 0x20]
004D:    DE E9                   fsubp    st(1)
004F:    DD 19                   fstp     qword ptr [ecx]
0051:    90                      nop
0052:    DD 46 E8                fld      qword ptr [esi - 0x18]
0055:    D9 E4                   ftst
0057:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0059:    D9 D0                   fnop
005B:    9E                      sahf
005C:    DD C0                   ffree    st(0)
005E:    D9 F7                   fincstp
0060:    74 12                   je       0x74    ; Conditional jump
0062:    DD 02                   fld      qword ptr [edx]
0064:    90                      nop
0065:    D9 C0                   fld      st(0)
0067:    DC 76 E8                fdiv     qword ptr [esi - 0x18]
006A:    D9 FC                   frndint    ; round(ST(0))
006C:    DC 4E E8                fmul     qword ptr [esi - 0x18]
006F:    DE E9                   fsubp    st(1)
0071:    DD 1A                   fstp     qword ptr [edx]
0073:    90                      nop
0074:    DD 46 F0                fld      qword ptr [esi - 0x10]
0077:    D9 E4                   ftst
0079:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
007B:    D9 D0                   fnop
007D:    9E                      sahf
007E:    DD C0                   ffree    st(0)
0080:    D9 F7                   fincstp
0082:    74 12                   je       0x96    ; Conditional jump
0084:    DD 03                   fld      qword ptr [ebx]
0086:    90                      nop
0087:    D9 C0                   fld      st(0)
0089:    DC 76 F0                fdiv     qword ptr [esi - 0x10]
008C:    D9 FC                   frndint    ; round(ST(0))
008E:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0091:    DE E9                   fsubp    st(1)
0093:    DD 1B                   fstp     qword ptr [ebx]
0095:    90                      nop
0096:    8B C3                   mov      eax, ebx
0098:    5E                      pop      esi
0099:    58                      pop      eax
009A:    5B                      pop      ebx
009B:    5D                      pop      ebp
009C:    C2 08 00                ret      8
