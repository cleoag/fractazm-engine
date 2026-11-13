; Disassembly: _TranslateRnd.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 184 bytes, decoded: 184 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    53                      push     ebx
0006:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0009:    89 C3                   mov      ebx, eax
000B:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000E:    83 C4 D0                add      esp, -0x30
0011:    FF 86 D0 00 00 00       inc      dword ptr [esi + 0xd0]
0017:    89 54 24 10             mov      dword ptr [esp + 0x10], edx
001B:    90                      nop
001C:    8B 86 D0 00 00 00       mov      eax, dword ptr [esi + 0xd0]
0022:    66 2D 07 3D             sub      ax, 0x3d07
0026:    F7 E0                   mul      eax
0028:    89 44 24 0C             mov      dword ptr [esp + 0xc], eax
002C:    8B 47 DC                mov      eax, dword ptr [edi - 0x24]
002F:    66 F7 64 24 0C          mul      word ptr [esp + 0xc]
0034:    25 FF 3F 00 00          and      eax, 0x3fff
0039:    89 04 24                mov      dword ptr [esp], eax
003C:    8B 47 D8                mov      eax, dword ptr [edi - 0x28]
003F:    66 F7 64 24 0C          mul      word ptr [esp + 0xc]
0044:    25 FF 3F 00 00          and      eax, 0x3fff
0049:    89 44 24 04             mov      dword ptr [esp + 4], eax
004D:    8B 47 D4                mov      eax, dword ptr [edi - 0x2c]
0050:    66 F7 64 24 0C          mul      word ptr [esp + 0xc]
0055:    25 FF 3F 00 00          and      eax, 0x3fff
005A:    89 44 24 08             mov      dword ptr [esp + 8], eax
005E:    90                      nop
005F:    B8 FF 3F 00 00          mov      eax, 0x3fff
0064:    89 44 24 0C             mov      dword ptr [esp + 0xc], eax
0068:    8B 54 24 10             mov      edx, dword ptr [esp + 0x10]
006C:    90                      nop
006D:    DD 01                   fld      qword ptr [ecx]
006F:    DD 02                   fld      qword ptr [edx]
0071:    DD 03                   fld      qword ptr [ebx]
0073:    DD 47 F0                fld      qword ptr [edi - 0x10]
0076:    DB 04 24                fild     dword ptr [esp]
0079:    DC 4F 08                fmul     qword ptr [edi + 8]
007C:    90                      nop
007D:    DC 27                   fsub     qword ptr [edi]
007F:    DE C9                   fmulp    st(1)
0081:    DE C1                   faddp    st(1)
0083:    DD 1B                   fstp     qword ptr [ebx]
0085:    DD 47 E8                fld      qword ptr [edi - 0x18]
0088:    DB 44 24 04             fild     dword ptr [esp + 4]
008C:    DC 4F 08                fmul     qword ptr [edi + 8]
008F:    90                      nop
0090:    DC 27                   fsub     qword ptr [edi]
0092:    DE C9                   fmulp    st(1)
0094:    DE C1                   faddp    st(1)
0096:    DD 1A                   fstp     qword ptr [edx]
0098:    DD 47 E0                fld      qword ptr [edi - 0x20]
009B:    DB 44 24 08             fild     dword ptr [esp + 8]
009F:    DC 4F 08                fmul     qword ptr [edi + 8]
00A2:    90                      nop
00A3:    DC 27                   fsub     qword ptr [edi]
00A5:    DE C9                   fmulp    st(1)
00A7:    DE C1                   faddp    st(1)
00A9:    DD 19                   fstp     qword ptr [ecx]
00AB:    90                      nop
00AC:    83 C4 30                add      esp, 0x30
00AF:    89 D8                   mov      eax, ebx
00B1:    5B                      pop      ebx
00B2:    5F                      pop      edi
00B3:    5E                      pop      esi
00B4:    5D                      pop      ebp
00B5:    C2 08 00                ret      8
