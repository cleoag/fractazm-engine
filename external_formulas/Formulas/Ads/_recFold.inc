// _recFold.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _recFold
// Hand-ported from ASM: 154 bytes, DEoption=-1
// Rectangular fold: two 2D folding phases in XZ plane + recursive tent fold on X.
// Y is unchanged throughout.

constexpr unsigned int FORMULA__RECFOLD = 3676;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void RecFoldOriginal_x87(double*, const double*);
#endif

// ASM trace (154 bytes):
//
// Phase 1: Rhombic fold in XZ (using original coordinates)
//   ax = |x|, az = |z|
//   x = (az - ax) - Asymm
//   z = (az + ax) - P
//
//   NOTE: medium_confidence template incorrectly uses multiplication (* Asymm, * P).
//   The ASM uses fsub (subtraction): offsets, not scales.
//
// Phase 2: Second fold in XZ (using Phase 1 output)
//   ax2 = |x|
//   x = ( ax2 - z - Q) * 0.5
//   z = (-ax2 - z - Q) * 0.5
//
//   The 0.5 constant is at [edi+0x00] (anonymous param, hardcoded here).
//
// Phase 3: Recursive tent fold on X only (if x > 0)
//   N = #Iterations - 1 (capped at 999)
//   repeat N times: x = |FoldX - |FoldX - x||
//
// Output: (x, y_unchanged, z)
//
// C handling: DEoption=-1 (transform, no C addition)

template<>
cpu_double3 execute_formula<FORMULA__RECFOLD>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x, p.y, p.z};
    RecFoldOriginal_x87(state, params);
    return cpu_double3(state[0], state[1], state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const E ax=abs(E(p.x)),az=abs(E(p.z));
    E x=double((az-ax)-params[1]);
    E z=double((az+ax)-params[2]);
    const E folded=-abs(x),difference=folded-z,sum=z+folded;
    x=double((-sum-params[3])*params[5]);
    z=double((difference-params[3])*params[5]);
    const int iterations=int(params[4]);
    // FTST/SHR/JBE rejects negative and unordered X. TOP remains nonzero
    // with FoldX and X loaded, so ZF is clear even for an exact zero X.
    if(x>=E(0) && iterations>1 && iterations<=1000) {
        const E fold=params[0];
        for(int i=1;i<iterations;++i) x=abs(fold-abs(fold-x));
    }
    return cpu_double3(double(x),p.y,double(z));
#else
    const double FoldX      = params[0];  // [edi-0x10] (default: 1)
    const double Asymm      = params[1];  // [edi-0x18] (default: -3)
    const double P          = params[2];  // [edi-0x20] (default: 10)
    const double Q          = params[3];  // [edi-0x28] (default: 1)
    const int   Iterations = int(params[4]);  // [edi-0x2c] (default: 4, int)
    // [edi+0x00] = 0.5 (hardcoded)

    double x = p.x, y = p.y, z = p.z;

    // Phase 1: Rhombic fold — subtracts offsets (NOT multiplication)
    double ax = fabs(x);
    double az = fabs(z);
    x = (az - ax) - Asymm;
    z = (az + ax) - P;

    // Phase 2: Second fold with 0.5 scaling
    double ax2 = fabs(x);
    double new_x = ( ax2 - z - Q) * 0.5;
    double new_z = (-ax2 - z - Q) * 0.5;
    x = new_x;
    z = new_z;

    // Phase 3: Recursive tent fold on x (only if x > 0)
    if (x > 0.0) {
        // Original DEC/CMP skips every count outside 2..1000. Test the
        // input first so INT_MIN does not trigger signed C++ overflow.
        if (Iterations > 1 && Iterations <= 1000) {
            const int N = Iterations - 1;
            for (int i = 0; i < N; i++) {
                x = fabs(FoldX - fabs(FoldX - x));
            }
        }
    }

    return cpu_double3(x, y, z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info__recFold = {
    .id = FORMULA__RECFOLD,
    .name = "_recFold",
    .param_count = 5,
    .complexity = 154,
    .uses_transcendental = false
};

/*
Original M3 Description:
Creates many copies of your fractal, arranged in a rectangle shape.
A 2D folding transform in the XZ plane (y unchanged).

Phase 1: Rhombic decomposition with absolute values
  ax = |x|, az = |z|
  x = (az - ax) - Asymm
  z = (az + ax) - P

Phase 2: Second folding with half-scaling
  ax2 = |x|
  x = (ax2 - z - Q) * 0.5
  z = (-ax2 - z - Q) * 0.5

Phase 3: Recursive tent fold (bounded loop)
  if x > 0: repeat (Iterations-1) times:
    x = |FoldX - |FoldX - x||

Parameters:
  [0] FoldX: double (default: 1) - tent fold amplitude
  [1] Asymm: double (default: -3) - rhombic offset (asymmetry)
  [2] P: double (default: 10) - sum offset
  [3] Q: double (default: 1) - second fold offset
  [4] #Iterations: int (default: 4, max 1000) - recursive fold count

Constants (hardcoded):
  [PVar+0x00] = 0.5
*/
