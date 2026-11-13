; Disassembly: FernTransform.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 223 bytes, decoded: 223 bytes (100.0%)
; DEoption: 21

0000:    DD 46 98                fld      qword ptr [esi - 0x68]
0003:    DD 46 90                fld      qword ptr [esi - 0x70]
0006:    DD 46 88                fld      qword ptr [esi - 0x78]
0009:    D9 C1                   fld      st(1)
000B:    D9 E1                   fabs    ; |ST(0)|
000D:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0010:    D8 C1                   fadd     st(1)
0012:    D9 E0                   fchs    ; -ST(0)
0014:    D8 5F F4                fcomp    dword ptr [edi - 0xc]
0017:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0019:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
001B:    72 46                   jb       0x63    ; Conditional jump
001D:    D8 47 F4                fadd     dword ptr [edi - 0xc]
0020:    DD 46 70                fld      qword ptr [esi + 0x70]
0023:    D9 47 F0                fld      dword ptr [edi - 0x10]
0026:    DC CC                   fmul     st(4), st(0)
0028:    DC CB                   fmul     st(3), st(0)
002A:    DC CA                   fmul     st(2), st(0)
002C:    DE C9                   fmulp    st(1)
002E:    DD 5E 70                fstp     qword ptr [esi + 0x70]
0031:    D9 C1                   fld      st(1)
0033:    D8 4F E8                fmul     dword ptr [edi - 0x18]
0036:    D9 C1                   fld      st(1)
0038:    D8 4F EC                fmul     dword ptr [edi - 0x14]
003B:    DE E9                   fsubp    st(1)
003D:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003F:    D8 4F EC                fmul     dword ptr [edi - 0x14]
0042:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0044:    D8 4F E8                fmul     dword ptr [edi - 0x18]
0047:    DE C1                   faddp    st(1)
0049:    D9 C2                   fld      st(2)
004B:    D8 4F E0                fmul     dword ptr [edi - 0x20]
004E:    D9 C1                   fld      st(1)
0050:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
0053:    DE E9                   fsubp    st(1)
0055:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0057:    D8 4F E4                fmul     dword ptr [edi - 0x1c]
005A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
005C:    D8 4F E0                fmul     dword ptr [edi - 0x20]
005F:    DE C1                   faddp    st(1)
0061:    EB 72                   jmp      0xd5
0063:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0065:    D9 E4                   ftst
0067:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0069:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006B:    72 30                   jb       0x9d    ; Conditional jump
006D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006F:    D8 47 C0                fadd     dword ptr [edi - 0x40]
0072:    D9 C1                   fld      st(1)
0074:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0077:    D9 C1                   fld      st(1)
0079:    D8 4F CC                fmul     dword ptr [edi - 0x34]
007C:    DE E9                   fsubp    st(1)
007E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0080:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0083:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0085:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0088:    DE C1                   faddp    st(1)
008A:    DD 46 70                fld      qword ptr [esi + 0x70]
008D:    D9 47 D0                fld      dword ptr [edi - 0x30]
0090:    DC CC                   fmul     st(4), st(0)
0092:    DC CB                   fmul     st(3), st(0)
0094:    DC CA                   fmul     st(2), st(0)
0096:    DE C9                   fmulp    st(1)
0098:    DD 5E 70                fstp     qword ptr [esi + 0x70]
009B:    EB 38                   jmp      0xd5
009D:    D9 E0                   fchs    ; -ST(0)
009F:    D9 E4                   ftst
00A1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00A3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A5:    72 2E                   jb       0xd5    ; Conditional jump
00A7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00A9:    D8 47 C4                fadd     dword ptr [edi - 0x3c]
00AC:    D9 C1                   fld      st(1)
00AE:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00B1:    D9 C1                   fld      st(1)
00B3:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00B6:    DE E9                   fsubp    st(1)
00B8:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00BA:    D8 4F D8                fmul     dword ptr [edi - 0x28]
00BD:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00BF:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
00C2:    DE C1                   faddp    st(1)
00C4:    DD 46 70                fld      qword ptr [esi + 0x70]
00C7:    D9 47 DC                fld      dword ptr [edi - 0x24]
00CA:    DC CC                   fmul     st(4), st(0)
00CC:    DC CB                   fmul     st(3), st(0)
00CE:    DC CA                   fmul     st(2), st(0)
00D0:    DE C9                   fmulp    st(1)
00D2:    DD 5E 70                fstp     qword ptr [esi + 0x70]
00D5:    DD 5E 88                fstp     qword ptr [esi - 0x78]
00D8:    DD 5E 90                fstp     qword ptr [esi - 0x70]
00DB:    DD 5E 98                fstp     qword ptr [esi - 0x68]
00DE:    C3                      ret
