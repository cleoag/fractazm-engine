// _CondItcountR.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _CondItcountR
// Hand-ported from ASM: 118 bytes, DEoption=-1
// META-FORMULA: modifies iteration counts, does NOT transform coordinates

constexpr unsigned int FORMULA__CONDITCOUNTR = 10353;

// ASM trace:
//   esi = context, edi = PVar
//   Reads r² from [esi+0x38] (pre-computed by engine)
//   Compares r² against two thresholds (R1², R2²) → three zones
//   Writes selected ItCount to ItCount[formula_nr] in context struct
//
//   000C: fld [esi+0x38]           ; r²
//   000: fcom [edi-0x10]          ; r² vs R1²
//          if r² ≤ R1² → eax = [edi-0x20] (ItCount_R<R1)
//   001E: fcom [edi-0x18]          ; r² vs R2²
//          if r² ≤ R2² → eax = [edi-0x24] (ItCount_R1<R<R2)
//          else        → eax = [edi-0x28] (ItCount_R>R2)
//   0030: fstp st(0)               ; pop r²
//
//   Clamp eax to [0, 99999]:
//   0032: test eax,eax; jge → clamp min 0
//   003A: cmp eax, 0x1869; jle → clamp max 99999
//
//   Load formula_nr = [edi-0x1C], clamp to [1,6]:
//   005: lea edi, [esi + edi*4 + 0x48]   ; addr of ItCount[formula_nr]
//   0063: cmp [edi], 0
//   0066: jge → skip sign preservation
//   0068: or eax, 0x80000000               ; preserve sign bit
//   006D: mov [edi], eax                   ; store ItCount

// Meta-formula: sets ItCount for a target formula slot based on r² thresholds
// itcount array: itcount[1..6], indexed by formula_nr (1-based)
static void cond_itcount_r(
    int* itcount,     // itcount[0..6] array (index 0 unused)
    double r_squared,         // pre-computed x²+y²+z² from [esi+0x38]
    double R1_sq,             // threshold R1² (.DSQUARE, default: 1.0)
    double R2_sq,             // threshold R2² (.DSQUARE, default: 4.0)
    int formula_nr,          // target slot 1..6 (default: 2)
    int ic_lo,               // ItCount when r² ≤ R1² (default: 0)
    int ic_mid,              // ItCount when R1² < r² ≤ R2² (default: 1)
    int ic_hi                // ItCount when r² > R2² (default: 2)
) {
    // Select ItCount based on r² zone
    int val;
    if (r_squared <= R1_sq) {
        val = ic_lo;
    } else if (r_squared <= R2_sq) {
        val = ic_mid;
    } else {
        val = ic_hi;
    }

    // Clamp to [0, 99999]
    val = clamp(val, 0, 99999);

    // Clamp formula_nr to [1, 6]
    formula_nr = clamp(formula_nr, 1, 6);

    // Preserve sign bit (inversion flag) from current ItCount
    if (itcount[formula_nr] < 0) {
        val |= int(0x80000000);
    }

    itcount[formula_nr] = val;
}

// Standard interface (passthrough — coordinates unchanged)
template<>
cpu_double3 execute_formula<FORMULA__CONDITCOUNTR>(cpu_double3 p, constant double* params, double &de) {
    // NOTE: This meta-formula modifies IterationState.itcount[], not coordinates.
    // The Metal engine must call cond_itcount_r() directly with the itcount array.
    // The standard execute_formula interface cannot express this side-effect.
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__CondItcountR = {
    .id = FORMULA__CONDITCOUNTR,
    .name = "_CondItcountR",
    .param_count = 6,
    .complexity = 118,
    .uses_transcendental = false
};

/*
Original M3 Description:
This formula modifies the iteration count on another formula by a condition
on the current 3d vector length (R):

if R < R1 { ItCount[Formula[Formula nr]] = ItCount_R<R1 }
else if R < R2 { ItCount[Formula[Formula nr]] = ItCount_R1<R<R2 }
else { ItCount[Formula[Formula nr]] = ItCount_R>R2 }

Use a high value like 1e30 for R2 to disable the condition R>R2.
A zero value for R1 disables the condition R<R1.

Because the formula iteration counts are not resetted on iteration start,
you should only modify formulas that are following this one.

Parameters:
  [0] R1²: double (.DSQUARE R1 = 1, engine pre-squares → stored as 1.0)
  [1] R2²: double (.DSQUARE R2 = 2, engine pre-squares → stored as 4.0)
  [2] Formula nr: integer (default: 2, range 1..6)
  [3] ItCount R<R1: integer (default: 0)
  [4] ItCount R1<R<R2: integer (default: 1)
  [5] ItCount R>R2: integer (default: 2)

PVar layout:
  [PVar-0x10] = R1² (double, 8 bytes, .DSQUARE pre-squared)
  [PVar-0x18] = R2² (double, 8 bytes, .DSQUARE pre-squared)
  [PVar-0x1C] = Formula nr (int32, 4 bytes)
  [PVar-0x20] = ItCount R<R1 (int32, 4 bytes)
  [PVar-0x24] = ItCount R1<R<R2 (int32, 4 bytes)
  [PVar-0x28] = ItCount R>R2 (int32, 4 bytes)

Context access:
  [esi+0x38] = r² (pre-computed x²+y²+z² by engine)
  [esi + formula_nr*4 + 0x48] = ItCount[formula_nr] (int32)

Sign bit convention:
  Bit 31 of ItCount is a "sign/inversion" flag preserved across writes.
  If the current ItCount value is negative, the formula ORs 0x80000000
  onto the new value to preserve this flag.
*/
