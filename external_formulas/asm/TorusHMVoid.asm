; Disassembly: TorusHMVoid.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 266 bytes, decoded: 266 bytes (100.0%)
; DEoption: 20

0000:    51                      push     ecx
0001:    83 C4 D0                add      esp, -0x30
0004:    DD 46 98                fld      qword ptr [esi - 0x68]
0007:    D8 67 E4                fsub     dword ptr [edi - 0x1c]
000A:    DD 46 90                fld      qword ptr [esi - 0x70]
000D:    D8 67 E8                fsub     dword ptr [edi - 0x18]
0010:    DD 46 88                fld      qword ptr [esi - 0x78]
0013:    D8 67 EC                fsub     dword ptr [edi - 0x14]
0016:    D9 C0                   fld      st(0)
0018:    D8 4F BC                fmul     dword ptr [edi - 0x44]
001B:    D9 C2                   fld      st(2)
001D:    D8 4F B8                fmul     dword ptr [edi - 0x48]
0020:    DE C1                   faddp    st(1)
0022:    D9 C3                   fld      st(3)
0024:    D8 4F B4                fmul     dword ptr [edi - 0x4c]
0027:    DE C1                   faddp    st(1)
0029:    D9 C1                   fld      st(1)
002B:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
002E:    D9 C3                   fld      st(3)
0030:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0033:    DE C1                   faddp    st(1)
0035:    D9 C4                   fld      st(4)
0037:    D8 4F CC                fmul     dword ptr [edi - 0x34]
003A:    DE C1                   faddp    st(1)
003C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003E:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0041:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0043:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0046:    DE C3                   faddp    st(3)
0048:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004A:    D8 4F C0                fmul     dword ptr [edi - 0x40]
004D:    DE C2                   faddp    st(2)
004F:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0053:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0057:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
005B:    DD 44 24 08             fld      qword ptr [esp + 8]
005F:    D8 C8                   fmul     st(0)
0061:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0065:    D8 C8                   fmul     st(0)
0067:    DE C1                   faddp    st(1)
0069:    D9 FA                   fsqrt    ; sqrt(ST(0))
006B:    D8 67 E0                fsub     dword ptr [edi - 0x20]
006E:    D9 C0                   fld      st(0)
0070:    D8 C8                   fmul     st(0)
0072:    DD 44 24 18             fld      qword ptr [esp + 0x18]
0076:    D8 C8                   fmul     st(0)
0078:    DE C1                   faddp    st(1)
007A:    D9 FA                   fsqrt    ; sqrt(ST(0))
007C:    D8 67 DC                fsub     dword ptr [edi - 0x24]
007F:    90                      nop
0080:    D9 C0                   fld      st(0)
0082:    D9 E1                   fabs    ; |ST(0)|
0084:    D9 47 D8                fld      dword ptr [edi - 0x28]
0087:    D9 E1                   fabs    ; |ST(0)|
0089:    DC 47 08                fadd     qword ptr [edi + 8]
008C:    DC 47 08                fadd     qword ptr [edi + 8]
008F:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0091:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0093:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0095:    73 0D                   jae      0xa4    ; Conditional jump
0097:    D9 E1                   fabs    ; |ST(0)|
0099:    DD 5E E0                fstp     qword ptr [esi - 0x20]
009C:    DD D8                   fstp     st(0)
009E:    DD D8                   fstp     st(0)
00A0:    DD D8                   fstp     st(0)
00A2:    EB 61                   jmp      0x105
00A4:    D9 C1                   fld      st(1)
00A6:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00AA:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00AC:    DC 0F                   fmul     qword ptr [edi]
00AE:    DD 44 24 08             fld      qword ptr [esp + 8]
00B2:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00B6:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00B8:    DC 0F                   fmul     qword ptr [edi]
00BA:    90                      nop
00BB:    DD 1C 24                fstp     qword ptr [esp]
00BE:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00C2:    D9 5C 24 18             fstp     dword ptr [esp + 0x18]
00C6:    DD D8                   fstp     st(0)
00C8:    89 E0                   mov      eax, esp
00CA:    8B 57 F4                mov      edx, dword ptr [edi - 0xc]
00CD:    89 E1                   mov      ecx, esp
00CF:    FF 96 0C 01 00 00       call     dword ptr [esi + 0x10c]
00D5:    8B 4F F0                mov      ecx, dword ptr [edi - 0x10]
00D8:    83 E1 03                and      ecx, 3
00DB:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00DE:    DC 67 F8                fsub     qword ptr [edi - 8]
00E1:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00E4:    D8 6C 24 18             fsubr    dword ptr [esp + 0x18]
00E8:    D9 E1                   fabs    ; |ST(0)|
00EA:    DC 67 08                fsub     qword ptr [edi + 8]
00ED:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00F0:    8B 4F B0                mov      ecx, dword ptr [edi - 0x50]
00F3:    83 E1 03                and      ecx, 3
00F6:    DD 04 CC                fld      qword ptr [esp + ecx*8]
00F9:    D8 47 AC                fadd     dword ptr [edi - 0x54]
00FC:    D8 4F A8                fmul     dword ptr [edi - 0x58]
00FF:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
0105:    83 C4 30                add      esp, 0x30
0108:    59                      pop      ecx
0109:    C3                      ret
