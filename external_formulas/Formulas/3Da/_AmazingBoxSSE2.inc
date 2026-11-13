// _AmazingBoxSSE2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _AmazingBoxSSE2
// Hand-ported from ASM: 147 bytes, DEoption=11
// SSE2 formula: standard AmazingBox (box fold + sphere fold) without C addition

constexpr unsigned int FORMULA__AMAZINGBOXSSE2 = 10351;

// ASM trace (SSE2 packed double operations):
//   esi = PVar (context overwritten at 0x07: mov esi,[esi+0x30])
//   eax = &{x,y}, ecx = &z, [ecx+8] = DE
//
//   Parameter layout (.Folding16 + .Boxscale + CONSTANTS):
//     [esi-0x40] = xmmword: {-Fold, -Fold}    (.Folding16 lower bound)
//     [esi-0x30] = xmmword: {+Fold, +Fold}    (.Folding16 upper bound)
//     [esi-0x20] = MinR²                       (.Boxscale precomputed)
//     [esi-0x18] = Scale/MinR²                 (.Boxscale precomputed)
//     [esi-0x10] = Scale                       (.Boxscale / .Double)
//     [esi+0x00] = FixedR² = 1.0              (CONSTANTS[0])
//
//   Step 1 — Box fold (x,y packed, z scalar):
//     000A: movupd xmm2, [eax]              ; xmm2 = {x, y}
//     000E: movsd  xmm4, [ecx]              ; xmm4 = z
//     0012: movapd xmm0, xmm2               ; xmm0 = {x, y}
//     0016: maxpd  xmm0, [esi-0x40]          ; clamp lower: max({x,y}, {-F,-F})
//     001B: maxsd  xmm4, [esi-0x40]          ; clamp lower: max(z, -F)
//     0020: minpd  xmm0, [esi-0x30]          ; clamp upper: min(_, {+F,+F})
//     0025: minsd  xmm4, [esi-0x30]          ; clamp upper: min(_, +F)
//     002A: addpd  xmm0, xmm0               ; xmm0 = 2*clamped
//     002E: addsd  xmm4, xmm4               ; xmm4 = 2*clamped_z
//     0032: subpd  xmm0, xmm2               ; xmm0 = 2*clamp-{x,y} → box-folded {bx,by}
//     0036: subsd  xmm4, [ecx]              ; xmm4 = 2*clamp-z → bz
//
//   Step 2 — Compute r²:
//     003A: movapd xmm1, xmm0               ; xmm1 = {bx, by}
//     003E: movsd  xmm5, xmm4               ; xmm5 = bz
//     0042: mulpd  xmm1, xmm1               ; xmm1 = {bx², by²}
//     0046: mulsd  xmm5, xmm5               ; xmm5 = bz²
//     004A: pshufd xmm2, xmm1, 0x4E         ; xmm2 = {by², bx²}
//     004: addsd  xmm1, xmm5               ; xmm1.lo = bx² + bz²
//     0053: addsd  xmm1, xmm2               ; xmm1.lo = bx² + by² + bz² = r²
//
//   Step 3 — Sphere fold (scale selection):
//     0057: ucomisd xmm1, [esi-0x20]         ; r² vs MinR²
//     005C: jae 0x65                          ; if r² >= MinR² → check FixedR²
//     005E: movsd  xmm3, [esi-0x18]          ; scale = Scale/MinR²
//     0063: jmp 0x74
//     0065: ucomisd xmm1, [esi]              ; r² vs FixedR² (=1.0)
//     0069: movsd  xmm3, [esi-0x10]          ; scale = Scale (tentative)
//     006E: jae 0x74                          ; if r² >= FixedR² → use Scale
//     0070: divsd  xmm3, xmm1               ; scale = Scale / r²
//
//   Step 4 — Apply scale to xyz and DE:
//     0074: movhpd xmm4, [ecx+8]            ; xmm4 = {bz, DE}
//     0079: shufpd xmm3, xmm3, 0            ; xmm3 = {scale, scale}
//     007E: mulpd  xmm0, xmm3               ; {bx,by} *= scale
//     0082: mulpd  xmm4, xmm3               ; {bz,DE} *= scale
//     0086: movupd [eax], xmm0              ; store x, y
//     008A: movupd [ecx], xmm4              ; store z, DE
//
// NOTE: No C addition — description says "without adding the C component".
// NOTE: esi overwritten to PVar — no context access, no C values used.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void AmazingBoxSSE2Original(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__AMAZINGBOXSSE2>(cpu_double3 p, double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x, p.y, p.z, de};
    alignas(16) const double originalParams[] = {
        -params[2], -params[2], params[2], params[2],
        params[1], params[0], params[3], 1.0
    };
    AmazingBoxSSE2Original(state, originalParams);
    de = state[3];
    return cpu_double3(state[0], state[1], state[2]);
#else

    const double ScaleOverMinR2 = params[0];
    const double minR2 = params[1];
    const double Scale = params[3];   // Original [esi-0x10], including Min R = 0.
    const double Fold    = params[2];   // default: 1.0 (.Folding16)
    // [CONSTANTS] embedded as compile-time values:
    const double FixedR2 = 1.0;       // [esi+0x00]

    // Step 1: Box fold  n' = 2*clamp(n, -Fold, +Fold) - n
    double bx = p.x > -Fold ? p.x : -Fold;
    bx = 2.0 * (bx < Fold ? bx : Fold) - p.x;
    double by = p.y > -Fold ? p.y : -Fold;
    by = 2.0 * (by < Fold ? by : Fold) - p.y;
    double bz = p.z > -Fold ? p.z : -Fold;
    bz = 2.0 * (bz < Fold ? bz : Fold) - p.z;

    // Step 2: r²
    double rr = (bx * bx + bz * bz) + by * by;

    // Step 3: Sphere fold — determine scale
    double scale;
    if (!(rr >= minR2)) {
        scale = ScaleOverMinR2;     // from .Boxscale: [esi-0x18]
    } else if (!(rr >= FixedR2)) {
        scale = Scale / rr;         // interpolated
    } else {
        scale = Scale;              // from [esi-0x10]
    }

    // Step 4: Apply scale (xyz and DE)
    de *= scale;

    return cpu_double3(bx * scale, by * scale, bz * scale);
#endif
}

// Formula metadata
constant FormulaInfo formula_info__AmazingBoxSSE2 = {
    .id = FORMULA__AMAZINGBOXSSE2,
    .name = "_AmazingBoxSSE2",
    .param_count = 4,
    .complexity = 147,
    .uses_transcendental = false
};

/*
Original M3 Description:
The amazing box formula without adding the C component.
Use with other formulas in combination.

Mathematical Formula:
  Box fold (all 3 axes):
    n' = 2 * clamp(n, -Fold, +Fold) - n

  Sphere fold:
    r² = bx² + by² + bz²
    if r² < MinR²:     scale = Scale / MinR²
    elif r² < FixedR²: scale = Scale / r²
    else:               scale = Scale

  Apply:
    x' = bx * scale
    y' = by * scale
    z' = bz * scale
    de' = de * scale

  No C addition — designed for use in combination with other formulas.

Parameters:
  [0] Scale: double (default: 2.0) — overall scaling factor
  [1] Min R: double (default: 0.5) — minimum radius for sphere fold
  [2] Fold: double (default: 1.0) — box fold limit
  [3] FixedR²: double (default: 1.0) — fixed radius squared (from CONSTANTS)

M3 Options:
  .Version = 3
  .DEscale = 0.2
  .DEoption = 11
  .RStop = 1024
  .SIpower = 2
  .SSE2
  .Double Scale = 2
  .Boxscale Min R = 0.5
  .Folding16 Fold = 1

PVar layout:
  [PVar-0x40] = xmmword {-Fold, -Fold}   (.Folding16 lower bound, 16-byte aligned)
  [PVar-0x30] = xmmword {+Fold, +Fold}   (.Folding16 upper bound, 16-byte aligned)
  [PVar-0x20] = MinR² = 0.25             (.Boxscale precomputed)
  [PVar-0x18] = Scale/MinR² = 8.0        (.Boxscale precomputed)
  [PVar-0x10] = Scale = 2.0              (.Boxscale / .Double)
  [PVar+0x00] = FixedR² = 1.0            (CONSTANTS[0])

NOTES:
  - SSE2 implementation uses packed double operations for parallel x,y processing
  - .Folding16 stores fold limits as 16-byte aligned packed doubles for SIMD
  - .Boxscale precomputes MinR², Scale/MinR², and Scale for branch-free sphere fold
  - No C addition: esi is overwritten to PVar, losing context struct access
  - DE tracking: [ecx+8] (de) is multiplied by the same scale as xyz
  - This is functionally identical to AmazingBox but without the C component,
    making it suitable for combination with other formulas that handle C
*/
