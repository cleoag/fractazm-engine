; Disassembly: _recFold.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 154 bytes, decoded: 154 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    DD 03                   fld      qword ptr [ebx]
0010:    90                      nop
0011:    D9 E1                   fabs    ; |ST(0)|
0013:    DD 01                   fld      qword ptr [ecx]
0015:    90                      nop
0016:    90                      nop
0017:    D9 E1                   fabs    ; |ST(0)|
0019:    D9 C0                   fld      st(0)
001B:    D8 C2                   fadd     st(2)
001D:    D9 C1                   fld      st(1)
001F:    D8 E3                   fsub     st(3)
0021:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0024:    DD 1B                   fstp     qword ptr [ebx]
0026:    90                      nop
0027:    DC 67 E0                fsub     qword ptr [edi - 0x20]
002A:    DD 19                   fstp     qword ptr [ecx]
002C:    90                      nop
002D:    90                      nop
002E:    DD D8                   fstp     st(0)
0030:    DD D8                   fstp     st(0)
0032:    DD 03                   fld      qword ptr [ebx]
0034:    90                      nop
0035:    D9 E1                   fabs    ; |ST(0)|
0037:    D9 E0                   fchs    ; -ST(0)
0039:    DD 01                   fld      qword ptr [ecx]
003B:    90                      nop
003C:    90                      nop
003D:    D9 C0                   fld      st(0)
003F:    D8 EA                   fsubr    st(2)
0041:    D9 C1                   fld      st(1)
0043:    D8 C3                   fadd     st(3)
0045:    D9 E0                   fchs    ; -ST(0)
0047:    DC 67 D8                fsub     qword ptr [edi - 0x28]
004A:    DC 0F                   fmul     qword ptr [edi]
004C:    DD 1B                   fstp     qword ptr [ebx]
004E:    90                      nop
004F:    DC 67 D8                fsub     qword ptr [edi - 0x28]
0052:    DC 0F                   fmul     qword ptr [edi]
0054:    DD 19                   fstp     qword ptr [ecx]
0056:    90                      nop
0057:    90                      nop
0058:    DD D8                   fstp     st(0)
005A:    DD D8                   fstp     st(0)
005C:    DD 47 F0                fld      qword ptr [edi - 0x10]
005F:    DD 03                   fld      qword ptr [ebx]
0061:    90                      nop
0062:    D9 E4                   ftst
0064:    9B                      wait
0065:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0067:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0069:    76 19                   jbe      0x84    ; Conditional jump
006B:    8B 47 D4                mov      eax, dword ptr [edi - 0x2c]
006E:    48                      dec      eax
006F:    7E 13                   jle      0x84    ; Conditional jump
0071:    3D E7 03 00 00          cmp      eax, 0x3e7
0076:    90                      nop
0077:    7F 0B                   jg       0x84    ; Conditional jump
0079:    D8 E9                   fsubr    st(1)
007B:    D9 E1                   fabs    ; |ST(0)|
007D:    D8 E9                   fsubr    st(1)
007F:    D9 E1                   fabs    ; |ST(0)|
0081:    48                      dec      eax
0082:    7F F5                   jg       0x79    ; Conditional jump
0084:    DD D9                   fstp     st(1)
0086:    DD 1B                   fstp     qword ptr [ebx]
0088:    90                      nop
0089:    90                      nop
008A:    90                      nop
008B:    90                      nop
008C:    90                      nop
008D:    90                      nop
008E:    90                      nop
008F:    90                      nop
0090:    90                      nop
0091:    89 D8                   mov      eax, ebx
0093:    5B                      pop      ebx
0094:    5F                      pop      edi
0095:    5E                      pop      esi
0096:    5D                      pop      ebp
0097:    C2 08 00                ret      8
