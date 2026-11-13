; Disassembly: _SinApprox.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 126 bytes, decoded: 126 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    50                      push     eax
0006:    52                      push     edx
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000D:    8B 77 F4                mov      esi, dword ptr [edi - 0xc]
0010:    83 E6 03                and      esi, 3
0013:    DD 04 F0                fld      qword ptr [eax + esi*8]
0016:    31 D2                   xor      edx, edx
0018:    D8 67 F0                fsub     dword ptr [edi - 0x10]
001B:    D8 4F EC                fmul     dword ptr [edi - 0x14]
001E:    DC 0F                   fmul     qword ptr [edi]
0020:    D9 C0                   fld      st(0)
0022:    DC 67 F8                fsub     qword ptr [edi - 8]
0025:    D9 FC                   frndint    ; round(ST(0))
0027:    DE E9                   fsubp    st(1)
0029:    D8 C0                   fadd     st(0)
002B:    D9 E8                   fld1    ; Load 1.0
002D:    D8 D9                   fcomp    st(1)
002F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0031:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0033:    73 0C                   jae      0x41    ; Conditional jump
0035:    D9 E8                   fld1    ; Load 1.0
0037:    D8 C0                   fadd     st(0)
0039:    DE E1                   fsubrp   st(1)
003B:    81 F2 00 00 00 80       xor      edx, 0x80000000
0041:    D9 EB                   fldpi    ; Load pi
0043:    DE C9                   fmulp    st(1)
0045:    DC 57 08                fcom     qword ptr [edi + 8]
0048:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004A:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
004C:    72 0A                   jb       0x58    ; Conditional jump
004E:    D9 EB                   fldpi    ; Load pi
0050:    DE E9                   fsubp    st(1)
0052:    81 F2 00 00 00 80       xor      edx, 0x80000000
0058:    D9 C0                   fld      st(0)
005A:    D8 C8                   fmul     st(0)
005C:    D8 C9                   fmul     st(1)
005E:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0061:    DE E9                   fsubp    st(1)
0063:    F7 C2 00 00 00 80       test     edx, 0x80000000
0069:    79 02                   jns      0x6d    ; Conditional jump
006B:    D9 E0                   fchs    ; -ST(0)
006D:    5A                      pop      edx
006E:    58                      pop      eax
006F:    D8 4F E8                fmul     dword ptr [edi - 0x18]
0072:    D8 47 E4                fadd     dword ptr [edi - 0x1c]
0075:    DD 1C F0                fstp     qword ptr [eax + esi*8]
0078:    5F                      pop      edi
0079:    5E                      pop      esi
007A:    5D                      pop      ebp
007B:    C2 08 00                ret      8
