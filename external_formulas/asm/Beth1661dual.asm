; Disassembly: Beth1661dual.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 788 bytes, decoded: 788 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    83 C4 C0                add      esp, -0x40
0006:    53                      push     ebx
0007:    56                      push     esi
0008:    57                      push     edi
0009:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000C:    8B 5E 30                mov      ebx, dword ptr [esi + 0x30]
000F:    DD 00                   fld      qword ptr [eax]
0011:    DC 08                   fmul     qword ptr [eax]
0013:    DD 5D D8                fstp     qword ptr [ebp - 0x28]
0016:    9B                      wait
0017:    DD 45 D8                fld      qword ptr [ebp - 0x28]
001A:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
001D:    DD 5D F8                fstp     qword ptr [ebp - 8]
0020:    9B                      wait
0021:    DD 02                   fld      qword ptr [edx]
0023:    DC 0A                   fmul     qword ptr [edx]
0025:    DD 5D D0                fstp     qword ptr [ebp - 0x30]
0028:    9B                      wait
0029:    DD 45 D0                fld      qword ptr [ebp - 0x30]
002C:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
002F:    DD 5D F0                fstp     qword ptr [ebp - 0x10]
0032:    9B                      wait
0033:    DD 01                   fld      qword ptr [ecx]
0035:    DC 09                   fmul     qword ptr [ecx]
0037:    DD 5D C8                fstp     qword ptr [ebp - 0x38]
003A:    9B                      wait
003B:    DD 45 C8                fld      qword ptr [ebp - 0x38]
003E:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0041:    DD 5D E8                fstp     qword ptr [ebp - 0x18]
0044:    9B                      wait
0045:    8B 7D 0C                mov      edi, dword ptr [ebp + 0xc]
0048:    DD 07                   fld      qword ptr [edi]
004A:    8B 7D 0C                mov      edi, dword ptr [ebp + 0xc]
004D:    DC 0F                   fmul     qword ptr [edi]
004F:    DD 5D C0                fstp     qword ptr [ebp - 0x40]
0052:    9B                      wait
0053:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0056:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0059:    DD 5D E0                fstp     qword ptr [ebp - 0x20]
005C:    9B                      wait
005D:    DD 03                   fld      qword ptr [ebx]
005F:    DC 08                   fmul     qword ptr [eax]
0061:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0064:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
0067:    DD 45 E8                fld      qword ptr [ebp - 0x18]
006A:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
006D:    DE C1                   faddp    st(1)
006F:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0072:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0075:    DE C1                   faddp    st(1)
0077:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
007A:    DD 45 F0                fld      qword ptr [ebp - 0x10]
007D:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0080:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0083:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0086:    DE C1                   faddp    st(1)
0088:    DD 45 E0                fld      qword ptr [ebp - 0x20]
008B:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
008E:    DE C1                   faddp    st(1)
0090:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0093:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0096:    DE C1                   faddp    st(1)
0098:    DD 45 C8                fld      qword ptr [ebp - 0x38]
009B:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
009E:    DE C1                   faddp    st(1)
00A0:    DD 45 C0                fld      qword ptr [ebp - 0x40]
00A3:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
00A6:    DE C1                   faddp    st(1)
00A8:    DC 4B 20                fmul     qword ptr [ebx + 0x20]
00AB:    DE C1                   faddp    st(1)
00AD:    DD 43 10                fld      qword ptr [ebx + 0x10]
00B0:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00B3:    DD 45 F0                fld      qword ptr [ebp - 0x10]
00B6:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
00B9:    DC 45 E0                fadd     qword ptr [ebp - 0x20]
00BC:    DD 45 D0                fld      qword ptr [ebp - 0x30]
00BF:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
00C2:    DD 45 C8                fld      qword ptr [ebp - 0x38]
00C5:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
00C8:    DE C1                   faddp    st(1)
00CA:    DD 45 C0                fld      qword ptr [ebp - 0x40]
00CD:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
00D0:    DE C1                   faddp    st(1)
00D2:    DC 4B 28                fmul     qword ptr [ebx + 0x28]
00D5:    DE E9                   fsubp    st(1)
00D7:    DE C9                   fmulp    st(1)
00D9:    DE C1                   faddp    st(1)
00DB:    DD 43 08                fld      qword ptr [ebx + 8]
00DE:    DC 4D F8                fmul     qword ptr [ebp - 8]
00E1:    DD 45 D0                fld      qword ptr [ebp - 0x30]
00E4:    DC 45 C8                fadd     qword ptr [ebp - 0x38]
00E7:    DC 45 C0                fadd     qword ptr [ebp - 0x40]
00EA:    DE C9                   fmulp    st(1)
00EC:    DE C1                   faddp    st(1)
00EE:    DD 43 28                fld      qword ptr [ebx + 0x28]
00F1:    DC 4D F8                fmul     qword ptr [ebp - 8]
00F4:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
00F7:    DE C1                   faddp    st(1)
00F9:    DE C9                   fmulp    st(1)
00FB:    8B FE                   mov      edi, esi
00FD:    83 C7 18                add      edi, 0x18
0100:    DC 07                   fadd     qword ptr [edi]
0102:    DD 18                   fstp     qword ptr [eax]
0104:    9B                      wait
0105:    DD 03                   fld      qword ptr [ebx]
0107:    DC 0A                   fmul     qword ptr [edx]
0109:    DD 45 E8                fld      qword ptr [ebp - 0x18]
010C:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
010F:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0112:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0115:    DE C1                   faddp    st(1)
0117:    DD 45 F8                fld      qword ptr [ebp - 8]
011A:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
011D:    DE C1                   faddp    st(1)
011F:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
0122:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0125:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0128:    DD 45 E0                fld      qword ptr [ebp - 0x20]
012B:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
012E:    DE C1                   faddp    st(1)
0130:    DD 45 F8                fld      qword ptr [ebp - 8]
0133:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0136:    DE C1                   faddp    st(1)
0138:    DD 45 C8                fld      qword ptr [ebp - 0x38]
013B:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
013E:    DE C1                   faddp    st(1)
0140:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0143:    DC 4D F8                fmul     qword ptr [ebp - 8]
0146:    DE C1                   faddp    st(1)
0148:    DD 45 D8                fld      qword ptr [ebp - 0x28]
014B:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
014E:    DE C1                   faddp    st(1)
0150:    DC 4B 20                fmul     qword ptr [ebx + 0x20]
0153:    DE C1                   faddp    st(1)
0155:    DD 43 10                fld      qword ptr [ebx + 0x10]
0158:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
015B:    DD 45 E8                fld      qword ptr [ebp - 0x18]
015E:    DC 45 E0                fadd     qword ptr [ebp - 0x20]
0161:    DC 45 F8                fadd     qword ptr [ebp - 8]
0164:    DD 45 C8                fld      qword ptr [ebp - 0x38]
0167:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
016A:    DD 45 C0                fld      qword ptr [ebp - 0x40]
016D:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0170:    DE C1                   faddp    st(1)
0172:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0175:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0178:    DE C1                   faddp    st(1)
017A:    DC 4B 28                fmul     qword ptr [ebx + 0x28]
017D:    DE E9                   fsubp    st(1)
017F:    DE C9                   fmulp    st(1)
0181:    DE C1                   faddp    st(1)
0183:    DD 43 08                fld      qword ptr [ebx + 8]
0186:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0189:    DD 45 C8                fld      qword ptr [ebp - 0x38]
018C:    DC 45 C0                fadd     qword ptr [ebp - 0x40]
018F:    DC 45 D8                fadd     qword ptr [ebp - 0x28]
0192:    DE C9                   fmulp    st(1)
0194:    DE C1                   faddp    st(1)
0196:    DD 43 28                fld      qword ptr [ebx + 0x28]
0199:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
019C:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
019F:    DE C1                   faddp    st(1)
01A1:    DE C9                   fmulp    st(1)
01A3:    8B C6                   mov      eax, esi
01A5:    8B F8                   mov      edi, eax
01A7:    83 C7 20                add      edi, 0x20
01AA:    DC 07                   fadd     qword ptr [edi]
01AC:    DD 1A                   fstp     qword ptr [edx]
01AE:    9B                      wait
01AF:    DD 03                   fld      qword ptr [ebx]
01B1:    DC 09                   fmul     qword ptr [ecx]
01B3:    DD 45 E0                fld      qword ptr [ebp - 0x20]
01B6:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
01B9:    DD 45 F8                fld      qword ptr [ebp - 8]
01BC:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
01BF:    DE C1                   faddp    st(1)
01C1:    DD 45 F0                fld      qword ptr [ebp - 0x10]
01C4:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
01C7:    DE C1                   faddp    st(1)
01C9:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
01CC:    DD 45 E0                fld      qword ptr [ebp - 0x20]
01CF:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
01D2:    DD 45 F8                fld      qword ptr [ebp - 8]
01D5:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
01D8:    DE C1                   faddp    st(1)
01DA:    DD 45 F0                fld      qword ptr [ebp - 0x10]
01DD:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
01E0:    DE C1                   faddp    st(1)
01E2:    DD 45 C0                fld      qword ptr [ebp - 0x40]
01E5:    DC 4D F8                fmul     qword ptr [ebp - 8]
01E8:    DE C1                   faddp    st(1)
01EA:    DD 45 D8                fld      qword ptr [ebp - 0x28]
01ED:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
01F0:    DE C1                   faddp    st(1)
01F2:    DD 45 D0                fld      qword ptr [ebp - 0x30]
01F5:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
01F8:    DE C1                   faddp    st(1)
01FA:    DC 4B 20                fmul     qword ptr [ebx + 0x20]
01FD:    DE C1                   faddp    st(1)
01FF:    DD 43 10                fld      qword ptr [ebx + 0x10]
0202:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0205:    DD 45 E0                fld      qword ptr [ebp - 0x20]
0208:    DC 45 F8                fadd     qword ptr [ebp - 8]
020B:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
020E:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0211:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0214:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0217:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
021A:    DE C1                   faddp    st(1)
021C:    DD 45 D0                fld      qword ptr [ebp - 0x30]
021F:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
0222:    DE C1                   faddp    st(1)
0224:    DC 4B 28                fmul     qword ptr [ebx + 0x28]
0227:    DE E9                   fsubp    st(1)
0229:    DE C9                   fmulp    st(1)
022B:    DE C1                   faddp    st(1)
022D:    DD 43 08                fld      qword ptr [ebx + 8]
0230:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0233:    DD 45 C0                fld      qword ptr [ebp - 0x40]
0236:    DC 45 D8                fadd     qword ptr [ebp - 0x28]
0239:    DC 45 D0                fadd     qword ptr [ebp - 0x30]
023C:    DE C9                   fmulp    st(1)
023E:    DE C1                   faddp    st(1)
0240:    DD 43 28                fld      qword ptr [ebx + 0x28]
0243:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
0246:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0249:    DE C1                   faddp    st(1)
024B:    DE C9                   fmulp    st(1)
024D:    8B D0                   mov      edx, eax
024F:    83 C2 28                add      edx, 0x28
0252:    DC 02                   fadd     qword ptr [edx]
0254:    DD 19                   fstp     qword ptr [ecx]
0256:    9B                      wait
0257:    DD 03                   fld      qword ptr [ebx]
0259:    8B 55 0C                mov      edx, dword ptr [ebp + 0xc]
025C:    DC 0A                   fmul     qword ptr [edx]
025E:    DD 45 F8                fld      qword ptr [ebp - 8]
0261:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
0264:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0267:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
026A:    DE C1                   faddp    st(1)
026C:    DD 45 E8                fld      qword ptr [ebp - 0x18]
026F:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0272:    DE C1                   faddp    st(1)
0274:    DC 4B 18                fmul     qword ptr [ebx + 0x18]
0277:    DD 45 F8                fld      qword ptr [ebp - 8]
027A:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
027D:    DD 45 F0                fld      qword ptr [ebp - 0x10]
0280:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
0283:    DE C1                   faddp    st(1)
0285:    DD 45 E8                fld      qword ptr [ebp - 0x18]
0288:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
028B:    DE C1                   faddp    st(1)
028D:    DD 45 D8                fld      qword ptr [ebp - 0x28]
0290:    DC 4D F0                fmul     qword ptr [ebp - 0x10]
0293:    DE C1                   faddp    st(1)
0295:    DD 45 D0                fld      qword ptr [ebp - 0x30]
0298:    DC 4D E8                fmul     qword ptr [ebp - 0x18]
029B:    DE C1                   faddp    st(1)
029D:    DD 45 C8                fld      qword ptr [ebp - 0x38]
02A0:    DC 4D F8                fmul     qword ptr [ebp - 8]
02A3:    DE C1                   faddp    st(1)
02A5:    DC 4B 20                fmul     qword ptr [ebx + 0x20]
02A8:    DE C1                   faddp    st(1)
02AA:    DD 43 10                fld      qword ptr [ebx + 0x10]
02AD:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
02B0:    DD 45 F8                fld      qword ptr [ebp - 8]
02B3:    DC 45 F0                fadd     qword ptr [ebp - 0x10]
02B6:    DC 45 E8                fadd     qword ptr [ebp - 0x18]
02B9:    DD 45 D8                fld      qword ptr [ebp - 0x28]
02BC:    DC 4D D0                fmul     qword ptr [ebp - 0x30]
02BF:    DD 45 D0                fld      qword ptr [ebp - 0x30]
02C2:    DC 4D C8                fmul     qword ptr [ebp - 0x38]
02C5:    DE C1                   faddp    st(1)
02C7:    DD 45 C8                fld      qword ptr [ebp - 0x38]
02CA:    DC 4D D8                fmul     qword ptr [ebp - 0x28]
02CD:    DE C1                   faddp    st(1)
02CF:    DC 4B 28                fmul     qword ptr [ebx + 0x28]
02D2:    DE E9                   fsubp    st(1)
02D4:    DE C9                   fmulp    st(1)
02D6:    DE C1                   faddp    st(1)
02D8:    DD 43 08                fld      qword ptr [ebx + 8]
02DB:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
02DE:    DD 45 D8                fld      qword ptr [ebp - 0x28]
02E1:    DC 45 D0                fadd     qword ptr [ebp - 0x30]
02E4:    DC 45 C8                fadd     qword ptr [ebp - 0x38]
02E7:    DE C9                   fmulp    st(1)
02E9:    DE C1                   faddp    st(1)
02EB:    DD 43 28                fld      qword ptr [ebx + 0x28]
02EE:    DC 4D E0                fmul     qword ptr [ebp - 0x20]
02F1:    DC 4D C0                fmul     qword ptr [ebp - 0x40]
02F4:    DE C1                   faddp    st(1)
02F6:    DE C9                   fmulp    st(1)
02F8:    83 E8 38                sub      eax, 0x38
02FB:    DC 00                   fadd     qword ptr [eax]
02FD:    8B 46 30                mov      eax, dword ptr [esi + 0x30]
0300:    83 E8 10                sub      eax, 0x10
0303:    DC 00                   fadd     qword ptr [eax]
0305:    8B 45 0C                mov      eax, dword ptr [ebp + 0xc]
0308:    DD 18                   fstp     qword ptr [eax]
030A:    9B                      wait
030B:    5F                      pop      edi
030C:    5E                      pop      esi
030D:    5B                      pop      ebx
030E:    8B E5                   mov      esp, ebp
0310:    5D                      pop      ebp
0311:    C2 08 00                ret      8
