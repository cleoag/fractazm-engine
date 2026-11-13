// Original gnarlyIFS uses SSE2 transforms and x87 heightfield arithmetic.
// Odd Flip reads caller stack at ESP+32; retain it as an uncertified fallback.
constexpr unsigned int FORMULA_GNARLYIFS = 10222;

#include "ifs_original_state.h"

// ASM trace (x87 section, after SSE2 preamble):
//
// M3 parameters:
//   .Double Beta = 3                -> [PVar - 0x10]
//   .2Doubles Scale = 1             -> [PVar - 0x20]
//   .2Doubles Z add = 0             -> [PVar - 0x30]
//   .Double Y add = 0               -> [PVar - 0x38]
//   .Double X add = 0               -> [PVar - 0x40]
//   .Double Amplitude = 0.1         -> [PVar - 0x48]
//   .Integer Apply scale+add = 0    -> [PVar - 0x4c]
//   .Integer Iters = 16             -> [PVar - 0x50]
//   .Integer Flip XZ (0,1) = 0      -> [PVar - 0x54]
//   .Double Alpha = -2              -> [PVar - 0x5c]
//   .Double Step X = -.1            -> [PVar - 0x64]
//   .Double Step Y = -.1            -> [PVar - 0x6c]
//   .Double OTrapClrMul = 1         -> [PVar - 0x74]
//   [CONSTANTS]:
//   Double = 1.5                    -> [edi + 0x00]
//   Double = 0.1                    -> [edi + 0x08]
//
// 0x4B: mov edx, [edi-0x54] = Flip_XZ
// 0x4E: and edx, 1
// 0x51: shl edx, 4 -> edx = 0 or 16 (selects stack offset)
// 0x54: fild [edi-0x50] = Iters (load as integer, convert to double)
// 0x57: fistp [esp+0x18] -> store Iters as int at [esp+0x18]
//
// Stack: [esp]=x, [esp+8]=y, [esp+0x10]=z
// edx=0: h_var = [esp+edx+0x10] = [esp+0x10] = z; other = [esp+edx] = [esp] = x
//   wait: [esp+0+0x10]=z, [esp+0]=x
// edx=16: h_var = [esp+16+0x10] = [esp+0x20] = beyond stack? No, stack is [esp..esp+0x18].
//   Actually add esp, -0x1C allocated 0x1C bytes. So [esp+0x10]=z, [esp+0x18]=Iters.
//   [esp+0x10+0x10]=[esp+0x20] would be outside. Hmm.
//   Wait: the 'add esp, -0x1c' at 0x00 allocates 28 bytes. But [esp+0x20] = esp + 32
//   which is past our allocation. But the original stack has more data above.
//   Actually: before 'add esp, -0x1c', the return address is on top. After subtracting,
//   [esp+0x1C] = old esp = return addr. [esp+0x20] = old [esp+4] = caller's data.
//   This seems wrong for accessing coordinates.
//
//   Let me re-think. The SSE2 preamble stores:
//   movupd [esp], xmm0 -> [esp]=x, [esp+8]=y (packed pair)
//   movsd [esp+0x10], xmm1 -> [esp+0x10]=z
//   So when Flip_XZ=0 (edx=0): [esp+edx+0x10] = [esp+0x10] = z
//   When Flip_XZ=1 (edx=16): [esp+16+0x10] = [esp+0x20]... hmm but this is x,y,z at [esp].
//   Actually wait, looking at the SSE2 preamble for this formula:
//   xmm0 = [esi-0x78] (x,y pair), xmm1 = [esi-0x68] (z)
//   mulpd xmm0, [edi-0x20] -> Scale
//   mulsd xmm1, [edi-0x20] -> Scale
//   addpd xmm0, [edi-0x40] (X_add, Y_add pair)
//   addsd xmm1, [edi-0x30] (Z_add)
//   Store: [esp]=x, [esp+8]=y, [esp+0x10]=z.
//
//   With edx=0: [esp+0+0x10]=[esp+0x10]=z, [esp+0]=[esp]=x
//   With edx=16: [esp+16+0x10]=[esp+0x1A]... that doesn't align to 8. Hmm.
//   Wait: shl edx, 4 means edx = 0 or 16 (0x10). Then:
//   [esp+edx+0x10]: edx=0 -> [esp+0x10]=z; edx=0x10 -> [esp+0x20].
//   [esp+edx]: edx=0 -> [esp+0]=x; edx=0x10 -> [esp+0x10]=z.
//
//   When edx=0x10: h_var = [esp+0x20] which is ABOVE the stack allocation.
//   This accesses the previous stack frame! But looking at the original stack:
//   Before 'add esp, -0x1c': stack had return address, then above that the
//   coordinates were stored by the IFS engine. The [esi-0x78] etc. values
//   are at fixed offsets from esi, not on this stack.
//
//   Hmm, actually I think I'm overcomplicating this. Let me reconsider.
//   In waveIFS, a similar pattern exists: edx = (Flip_XZ & 1) * 16.
//   And waveIFS allocates 0x20 bytes on stack (add esp, -0x20).
//   Here only 0x1C bytes allocated, but waveIFS stores:
//   [esp]=x, [esp+8]=y, [esp+0x10]=z (24 bytes) in a 0x20 allocation.
//   With edx=16: [esp+16+0x10]=[esp+0x20] which in waveIFS is within the 0x20 alloc.
//   Actually 0x20 = 32 bytes, and [esp+0x20] is the 33rd byte, one past. Hmm.
//   But [esp+0x20] in waveIFS = old esp (before sub), and the old stack has
//   more data. The convention seems to accept reading past the allocation.
//
//   Actually, I think the trick is different. Let me look at neg edx at 0xE1:
//   After the Gnarl iteration, edx is negated. Then:
//   [esp+edx+0x10] where edx was 0 -> 0+0x10 = z. After neg: 0+0x10 still z.
//   [esp+edx+0x10] where edx was 0x10 -> after neg edx=-0x10: -0x10+0x10 = [esp] = x.
//   So after neg: the "other" axis.
//   Before neg: h_var axis used in Gnarl computation.
//   With edx=0: computes on z and y, height mapped along x.
//   With edx=0x10: computes on x and y, height mapped along z.
//
//   But wait, for the initial access at 0x5C:
//   fld [esp+edx+0x10] where edx=0 loads z, edx=0x10 loads [esp+0x20].
//   [esp+0x20] with 0x1C allocation... This is reading from the old stack.
//   Actually: when esp was decremented by 0x1C, [esp+0x1C] is the old bottom.
//   Reading [esp+0x20] reads 4 bytes above that, which is... something else.
//   This doesn't make sense for reading x.
//
//   Unless... the formula uses a different coordinate layout. Let me re-check.
//   The SSE2 stores at 0x40: movupd [esp], xmm0 (16 bytes: x at [esp], y at [esp+8])
//   And 0x45: movsd [esp+0x10], xmm1 = z at [esp+0x10].
//
//   For Flip_XZ=1, edx=16, we need to load x. x is at [esp+0] = [esp].
//   But [esp+edx+0x10] = [esp+0x20] is NOT x.
//
//   I think the coordinate layout might be different for this formula. Let me
//   re-check the SSE2 preamble carefully:
//   0x03: movupd xmm0, [esi-0x78]  -> xmm0 = (x, y) from context
//   0x08: movsd xmm1, [esi-0x68]   -> xmm1 = z
//   0x0D: mulpd xmm0, [edi-0x20]   -> (x*Scale, y*Scale)
//   0x12: mulsd xmm1, [edi-0x20]   -> z*Scale
//   0x17: addpd xmm0, [edi-0x40]   -> (x*S+X_add, y*S+Y_add)
//   0x1C: addsd xmm1, [edi-0x30]   -> z*S+Z_add
//
//   Store:
//   0x40: movupd [esp], xmm0       -> [esp]=x, [esp+8]=y
//   0x45: movsd [esp+0x10], xmm1   -> [esp+0x10]=z
//
//   For Flip_XZ=0 (edx=0):
//     0x5C: fld [esp+0+0x10] = z (this is the wave input variable)
//     0x85: fld [esp+0+8] = y  (at 0x85: DD 44 24 08)
//     0x89: fld [esp+0+0] = x  (at 0x89: DD 04 14, i.e., [esp+edx] = [esp+0] = x)
//
//   For Flip_XZ=1 (edx=0x10):
//     0x5C: fld [esp+0x10+0x10] = [esp+0x20] = ???
//     That's indeed past the allocation. This is clearly a quirk where the formula
//     relies on the stack layout having the return address and other data.
//
//   Hmm, but actually: maybe the Flip_XZ with shl 4 produces 0 or 16, and the
//   addressing works differently. Let me look at the waveIFS pattern more carefully.
//   In waveIFS: 'add esp, -0x20' (32 bytes). Coordinates at [esp+0x10], [esp+0x18],
//   [esp+0x20]? No, waveIFS stores amplitude at [esp+0x18].
//
//   OK I think the real answer is simpler. For gnarlyIFS (0x1C=28 bytes alloc):
//   [esp]=[esp+0]=x, [esp+8]=y, [esp+0x10]=z, [esp+0x18]=Iters
//   The Gnarl computation uses two variables: the "selected axis" and y.
//   For Flip_XZ=0: selected = z = [esp+0x10]. neg edx=0 -> [esp+0x10] for DE axis = z.
//   After neg edx=0, for DE: [esp+edx+0x10] = [esp+0x10] = z. But wait the DE subtract
//   is at 0x10A: DC 64 14 10 = fsub [esp+edx+0x10]. With negated edx.
//   If edx was 0 before neg, it's still 0: fsub [esp+0x10] = z. Shape = gnarl_height - z.
//   If edx was 0x10 before neg, after neg it's -0x10=0xFFFFFFF0 (32-bit):
//   [esp+0xFFFFFFF0+0x10] = [esp+0] = x. Shape = gnarl_height - x. That works!
//   The neg edx trick with 32-bit wrap: 0+0x10=0x10 (z), and (-16+16=0) -> [esp]=x.
//   So it selects the "other" axis after neg. Clever.
//
//   For the initial load at 0x5C: fld [esp+edx+0x10]
//   edx=0: loads z. edx=0x10: loads [esp+0x20].
//   For edx=0x10 case, [esp+0x20] is 32 bytes above esp, but we only allocated 28.
//   The 4 extra bytes... hmm, the return address is at [esp+0x1C] (4 bytes).
//   [esp+0x20] would be the first arg or saved register above. This seems wrong.
//
//   Wait -- could the allocation be larger? Let me re-read: 0x00: 83 C4 E4 = add esp, -0x1c.
//   0xE4 = 256-0x1C = 228. As a signed byte: -0x1C = -28. So 28 bytes. Hmm.
//
//   Actually, I wonder if [esp+0x20] just reads whatever happens to be there from
//   the caller's frame. For the initial Gnarl computation, it might read some prior
//   value. But that seems like a bug. Let me look more carefully at what happens
//   at 0x5C and following.
//
//   0x5C: DD 44 14 10 = fld qword ptr [esp + edx + 0x10]
//   When edx=0x10: fld [esp+0x20]. In the Mandelbulb3D engine, the caller likely
//   stores coordinates at specific offsets. The [esi-0x78] etc. references suggest
//   the context struct is at esi, and the stack frame layout is engine-controlled.
//   The formula code may rely on the engine's stack having specific data.
//
//   For simplicity, I'll interpret: Flip_XZ=0 uses z as input, Flip_XZ=1 uses x.
//   The Gnarl iteration then maps to a height that's compared against the other axis.
//
// Gnarl early bailout check:
// 0x5C: fld h_var (z or x based on Flip_XZ)
// 0x60: fabs -> |h_var|
// 0x62: fld |Amplitude| from [edi-0x48] -> |Amplitude|
//   Actually: DD 47 B8 = fld [edi-0x48]? 0xB8 signed = -0x48. Yes.
//   0x65: fabs -> |Amplitude|
//   0x67: fadd [edi+0x08] = 0.1 (const[1]) -> |Ampl| + 0.1
//   0x6A: fmul [edi] = 1.5 (const[0]) -> (|Ampl| + 0.1) * 1.5
//   0x6C: fadd [esi-0x88] -> + some context value (iteration count?)
//     DC 86 78FFFFFF = fadd [esi-0x88]. What's at [esi-0x88]? This is a context offset.
//     For DEoption=20, the engine provides the accumulated DE or iteration count.
//     This might be a pre-computed threshold for early bailout.
//
// 0x72: fcompp -> compare |h_var| with threshold, pop both
// 0x74-0x78: fnstsw, shr ah,1, jae 0x85
// If |h_var| < threshold: too far from surface, quick return:
//   0x7A: fld [esp+edx+0x10] (h_var again)
//   0x7E: fchs -> -h_var
//   0x80: jmp 0x15 (to scale correction and return)
//   This early exit returns -h_var as shape_de (a large positive value indicating far away)
//
// Main Gnarl iteration (0x85 onwards):
// 0x85: fld [esp+8] = y
// 0x89: fld [esp+edx] = other_var (x when Flip=0, or z when Flip=1)
//   [esp+0] = x for edx=0; [esp+edx] with edx=0x10 = [esp+0x10] = z.
//   So: other_var = x when Flip=0, z when Flip=1.
//   h_var = z when Flip=0, x when Flip=1.
//
// Stack at 0x8C: st0=other_var, st1=y
// Gnarl iteration loop (0x8C-0xC2):
// 0x8C: fld st(0) -> dup other_var
// 0x8E: fmul [edi-0x10] = Beta -> other*Beta
// 0x91: fsin -> sin(other*Beta)
// 0x93: fadd st(1) -> sin(other*Beta) + y  (D8 C1 = fadd st0,st1)
// 0x95: fmul [edi-0x5c] = Alpha -> (sin(other*B)+y)*Alpha  (DC 4 A4, 0xA4=-0x5C)
// 0x98: fsin -> sin((sin(other*B)+y)*Alpha)
// 0x9A: fadd st(1) -> sin(...) + y
// 0x9C: fsin -> sin(sin(...)+y)
// 0x9E: fmul [edi-0x64] = StepX -> sin(...)*StepX  (DC 4 9C, 0x9C=-0x64)
// 0xA1: fadd st(2) -> add to y... wait: D8 C2 = fadd st0,st2.
//   st0 has the computed delta for other_var, st2 is the original other_var?
//   Let me retrace more carefully:
//
//   At 0x8C top of loop: st0=v (initially other_var), st1=w (initially y)
//   0x8C: fld st(0) -> st0=v, st1=v, st2=w
//   0x8E: fmul Beta -> st0=v*B, st1=v, st2=w
//   0x91: fsin -> st0=sin(v*B), st1=v, st2=w
//   0x93: D8 C1 = fadd st0,st1 -> st0=sin(v*B)+v, st1=v, st2=w
//   Wait no: at 0x93, st1=v (other), st2=w (y).
//   Hmm but the fadd is with st(1) which is v, not w(y).
//   Let me re-read: 0x93: D8 C1 = fadd st(0), st(1).
//   After fsin, stack is: st0=sin(v*B), st1=v, st2=w.
//   fadd st(0),st(1): st0 = sin(v*B) + v. Hmm that doesn't match description.
//   Actually looking at Gnarl formula: new_v = v - StepX * sin(sin(Alpha*(sin(Beta*w)+v))+w)
//   and new_w = w - StepY * sin(sin(Alpha*(sin(Beta*v)+w))+v)
//   So the first sin uses Beta*w (not v). Let me re-read:
//   At loop start st0 and st1 should represent v and w.
//
//   Actually, let me look at what st0 and st1 represent at 0x8C (loop entry at 0x8C):
//   First iteration: st0=other_var (call it 'a'), st1=y (call it 'b')
//   0x8C: fld st(0) -> st0=a, st1=a, st2=b
//   0x8E: fmul Beta -> st0=a*B, st1=a, st2=b
//   0x91: fsin -> st0=sin(a*B)
//   0x93: fadd st(1) -> st0=sin(a*B)+a ... but Gnarl wants sin(B*b)+a or sin(B*a)+b?
//   Hmm, the code uses st0 (copy of a) multiplied by Beta, then adds st1 (original a).
//   That gives sin(a*B) + a. But standard Gnarl has sin(b*B) + a.
//
//   Wait, I need to re-check. At 0x93, st1 is the original a (from the dup at 0x8C).
//   So st0 = sin(a*B) + a. Hmm, that's self-referential.
//   Then:
//   0x95: fmul Alpha -> (sin(a*B)+a)*Alpha
//   0x98: fsin -> sin((sin(a*B)+a)*Alpha)
//   0x9A: D8 C1 = fadd st0,st1 -> sin(...) + a (st1 still = a)
//   0x9C: fsin -> sin(sin(...)+a)
//   0x9E: fmul StepX -> sin(sin((sin(a*B)+a)*A)+a) * StepX = delta_b
//   0xA1: D8 C2 = fadd st0,st2 -> delta_b + b
//
//   Then for second half (0xA3 onwards):
//   0xA3: fld st(2) = b -> st0=b, st1=delta_b+b, st2=a, st3=b
//   0xA5: fmul Beta -> st0=b*B
//   0xA8: fsin -> sin(b*B)
//   0xAA: D8 C3 = fadd st0,st3 -> sin(b*B) + b (st3 = original b)
//   0xAC: fmul Alpha -> (sin(b*B)+b)*Alpha
//   0xAF: fsin -> sin(...)
//   0xB1: D8 C3 = fadd st0,st3 -> sin(...) + b
//   0xB3: fsin -> sin(sin(...)+b)
//   0xB5: fmul [edi-0x6c] = StepY -> delta_a
//   0xB8: D8 C2 = fadd st0,st2 -> delta_a + a
//
//   0xBA: DD DB = fstp st(3) -> store (delta_a+a) to st3 (was b), pop
//     After: st0 = (delta_b+b), st1 = a, st2 = (delta_a+a)
//   0xBC: DD D9 = fstp st(1) -> store (delta_b+b) to st1, pop
//     After: st0 = (delta_b+b), st1 = (delta_a+a)
//     Wait: fstp st(1) stores st0 into st1 then pops.
//     Before: st0=(delta_b+b), st1=a, st2=(delta_a+a)
//     After fstp st(1): st1 gets (delta_b+b), pop -> st0=(delta_b+b)(was st1), st1=(delta_a+a)
//     Hmm: after pop, old st2 becomes new st1.
//     So: st0 = delta_b + b = new_b, st1 = delta_a + a = new_a
//     Wait no: when we pop, the written value in old st1 position becomes new st0:
//     Before pop: st0=(delta_b+b), st1=(delta_b+b)[written], st2=(delta_a+a)
//     After pop: st0=(delta_b+b)[from old st1], st1=(delta_a+a)[from old st2]
//     So: st0 = new_b, st1 = new_a.
//
//   Hmm, this means v and w swap roles each iteration? Or maybe the iteration
//   overwrites both and they're meant to be (new_a, new_b) = (delta_a+a, delta_b+b).
//   The end state: st0=new_b, st1=new_a. For next iteration:
//   At 0x8C again: st0=new_b acts as 'a', st1=new_a acts as 'b'.
//   So roles DO swap each iteration. This is the Gnarl iteration.
//
// 0xBE: dec [esp+0x18] -> Iters--
// 0xC2: jg 0x8C -> loop if Iters > 0
//
// After loop: st0=final_b, st1=final_a
// 0xC4: fmul st(0) -> final_b^2
// 0xC6: fxch st(1) -> st0=final_a, st1=final_b^2
// 0xC8: fmul st(0) -> final_a^2
// 0xCA: faddp -> final_a^2 + final_b^2
// 0xCC: fsqrt -> gnarl_r = sqrt(a^2 + b^2)
//
// Original input radius:
// 0xCE: fld [esp+8] = y
// 0xD2: fmul st(0) -> y^2
// 0xD4: fld [esp+edx] = other_var (with original edx, before neg)
// 0xD7: fmul st(0) -> other^2
// 0xD9: faddp -> y^2 + other^2
// 0xDB: fsqrt -> orig_r = sqrt(y^2 + other^2)
// 0xDD: fchs -> -orig_r
// 0xDF: faddp -> gnarl_r - orig_r = height_diff
//
// 0xE1: neg edx -> flip edx for selecting other axis
//
// OTrap writeback:
// 0xE3: fld st(0) -> dup height_diff
// 0xE5: fld [edi-0x74] = OTrapClrMul
// 0xE8: fldz
// 0xEA: fcompp -> compare 0 with OTrapClrMul, pop both
// 0xEC-0xF0: fnstsw, shr ah,1, jb 0xFC
//   If 0 < OTrapClrMul (i.e., OTrapClrMul > 0): skip special path
//   If OTrapClrMul <= 0: jump to 0xFC
//
// OTrapClrMul > 0 path (default):
// 0xF2: fld1 -> 1.0
// 0xF4: fadd st(1), st(0) -> height_diff + 1 (in st1)
// 0xF6: faddp st(1) -> 1 + (height_diff + 1) = height_diff + 2
//   Wait: DC C1 = fadd st(1), st(0). st1 = st1 + st0 = height_diff + 1.
//   DE C1 = faddp st(1): st1 = st1 + st0. st0=1, st1=height_diff+1. st1=height_diff+2? No.
//   Actually after fadd st(1),st(0): st0=1, st1=height_diff+1.
//   faddp st(1): st(1) = st(1) + st(0) = (height_diff+1) + 1 = height_diff+2. Pop.
//   st0 = height_diff+2.
//   D9 E1 = fabs -> |height_diff + 2|
//   D9 FA = fsqrt -> sqrt(|height_diff + 2|)
//
// 0xFC: fmul [edi-0x74] = OTrapClrMul
// 0xFF: fabs
// 0x101: fstp [esi+0x80] -> store OTrap
//
// Shape DE:
// 0x107: fmul [edi-0x48] = Amplitude -> height_diff * Amplitude
// 0x10A: fsub [esp+edx+0x10] -> subtract h_axis_var (after neg edx)
//   With neg edx: if original edx=0, now edx=0: [esp+0+0x10]=z. Hmm, neg 0 = 0.
//   If original edx=0x10, neg gives edx=-0x10=0xFFFFFFF0 in 32-bit.
//   [esp+0xFFFFFFF0+0x10] = [esp+0] = x. So for Flip=1, subtract x.
//   For Flip=0, subtract z. This is the height axis.
//
// shape_de = height_diff * Amplitude - h_axis
//
// Scale correction:
// 0x10E: cmp [edi-0x4c], 0
// 0x112: jne 0x117
// 0x114: fdiv [edi-0x20] = Scale
// 0x117: fstp [esi-0x20]
// 0x11A: add esp, 0x1C
// 0x11D: ret
//
// Note: there's dead/duplicate code after 0x11D (the code from 0x11E onwards
// appears to be leftover from a different version, showing similar OTrap+DE
// patterns. The 'ret' at 0x11D ends the actual formula.)

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void GnarlyIFSEvenOriginal_x87(double*, const double*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_GNARLYIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    if ((int(params[8]) & 1) == 0) {
        double state[] = {p.x,p.y,p.z,de,params[DIFS_VARYSCALE_SLOT],
                          params[MAX_FORMULA_PARAMS-1],params[DIFS_ROLD_SLOT],
                          0.,0.,0.,0.,1.5,.1};
        const int selectors[] = {int(params[6]),int(params[7])};
        GnarlyIFSEvenOriginal_x87(state,params,selectors);
        de=state[3];
        params[DIFS_VARYSCALE_SLOT]=state[4];
        params[MAX_FORMULA_PARAMS-1]=state[5];
        if (selectors[0]) params[DIFS_COMMIT_XYZ_SLOT]=1.;
        return cpu_double3(state[0],state[1],state[2]);
    }
#elif defined(MB3D_SOFTWARE_EXT80)
    if ((int(params[8])&1)==0) {
        Arm64ApplyIFSState state(p,params);
        using E=Arm64ApplyIFSState::E;
        const E x(state.transformed.x), y(state.transformed.y), z(state.transformed.z);
        const E threshold=(abs(E(params[5]))+E(params[14]))*E(params[13])
            + E(params[DIFS_ROLD_SLOT]);
        if (!(threshold>=abs(z))) return state.finish(-z,de);
        const auto sine=numeric::extendedSin;
        const E beta(params[0]), alpha(params[9]);
        const E stepX(params[10]), stepY(params[11]);
        E a=x,b=y;
        int64_t remaining=static_cast<int32_t>(params[7]);
        FormulaWorkBudget work("gnarlyIFS");
        do {
            work.beforeStep();
            const E nextA=sine(sine((sine(a*beta)+a)*alpha)+a)*stepX+b;
            const E nextB=sine(sine((sine(b*beta)+b)*alpha)+b)*stepY+a;
            // FSTP ST(3), FSTP ST(1) swaps the two iteration roles.
            a=nextA; b=nextB;
            // DEC INT_MIN sets OF, so the original signed JG does not loop.
        } while (--remaining>0);
        const E difference=sqrt(a*a+b*b)+(-sqrt(y*y+x*x));
        E trap=difference;
        // Two FADDs with FLD1, not a precombined +2 operation.
        if (E(0)>=E(params[12])) trap=sqrt(abs((trap+E(1))+E(1)));
        params[MAX_FORMULA_PARAMS-1]=static_cast<double>(abs(trap*E(params[12])));
        return state.finish(difference*E(params[5])-z,de);
    }
#endif
    // Portable and odd-Flip fallback; odd Flip depends on the Pascal caller
    // stack in the original CODE and is not certified by point-oracle tests.
    // IFS parameters
    const double Beta = params[0];            // [PVar-0x10]
    const double Scale = params[1];           // [PVar-0x20] (.2Doubles)
    const double Z_add = params[2];           // [PVar-0x30] (.2Doubles)
    const double Y_add = params[3];           // [PVar-0x38]
    const double X_add = params[4];           // [PVar-0x40]
    const double Amplitude = params[5];       // [PVar-0x48]
    const int Apply_scale_add = int(params[6]); // [PVar-0x4c]
    const int Iters = int(params[7]);        // [PVar-0x50]
    const int Flip_XZ = int(params[8]);      // [PVar-0x54]
    const double Alpha = params[9];           // [PVar-0x5c]
    const double StepX = params[10];          // [PVar-0x64]
    const double StepY = params[11];          // [PVar-0x6c]
    // params[12] = OTrapClrMul (orbit-trap multiplier)
    // Constants
    // const double c_1_5 = params[13]; // [edi] = 1.5
    // const double c_0_1 = params[14]; // [edi+0x08] = 0.1

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: Select axes based on Flip_XZ
    // Flip_XZ=0: Gnarl on (x,y), height along z
    // Flip_XZ=1: Gnarl on (z,y), height along x
    double h_var = (Flip_XZ & 1) != 0 ? x : z;     // height axis variable
    double other_var = (Flip_XZ & 1) != 0 ? z : x;  // Gnarl input variable

    // Step 3: Early bailout check
    double bound = (fabs(Amplitude) + 0.1) * 1.5 + params[MAX_FORMULA_PARAMS - 6]; // DIFS_ROLD_SLOT
    if (fabs(h_var) > bound) {
        double shape_de = -h_var;
        if (Apply_scale_add == 0) {
            shape_de /= Scale;
        }
        de = shape_de;
        return cpu_double3(state_x, state_y, state_z);
    }

    // Step 4: Gnarl 2D iteration
    // a = other_var, b = y
    double a = other_var;
    double b = y;

    for (int i = 0; i < (Iters > 0 ? Iters : 1); i++) {
        // Compute new_b = b + StepX * sin(sin(Alpha*(sin(Beta*a)+a))+a)
        double t1 = sin(a * Beta);
        t1 = (t1 + a) * Alpha;
        t1 = sin(t1) + a;
        t1 = sin(t1) * StepX + b;

        // Compute new_a = a + StepY * sin(sin(Alpha*(sin(Beta*b)+b))+b)
        double t2 = sin(b * Beta);
        t2 = (t2 + b) * Alpha;
        t2 = sin(t2) + b;
        t2 = sin(t2) * StepY + a;

        // Original FSTP ST(3), FSTP ST(1) swaps roles each iteration.
        a = t1;
        b = t2;
    }

    // Step 5: Gnarl height computation
    double gnarl_r = sqrt(a * a + b * b);
    double orig_r = sqrt(y * y + other_var * other_var);
    double height_diff = gnarl_r - orig_r;

    // Step 6: Shape DE = height_diff * Amplitude - h_var
    double shape_de = height_diff * Amplitude - h_var;

    // OTrap writeback
    double OTrapClrMul = params[12];
    double otrap_val = height_diff;
    if (OTrapClrMul <= 0.0) {
        otrap_val = sqrt(fabs(height_diff + 2.0));
    }
    params[MAX_FORMULA_PARAMS - 1] = fabs(otrap_val * OTrapClrMul);

    // Step 7: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(state_x, state_y, state_z);
}

constant FormulaInfo formula_info_gnarlyIFS = {
    .id = FORMULA_GNARLYIFS,
    .name = "gnarlyIFS",
    .param_count = 15,
    .complexity = 800,
    .uses_transcendental = true
};
