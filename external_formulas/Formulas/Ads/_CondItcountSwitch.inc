// _CondItcountSwitch.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _CondItcountSwitch
// Hand-ported from ASM: 152 bytes, DEoption=-1
// META-FORMULA: modifies iteration counts, does NOT transform coordinates

constexpr unsigned int FORMULA__CONDITCOUNTSWITCH = 10354;

// ASM trace:
//   esi = context, edi = PVar
//   Increments execution counter [esi+0xD0], compares against CVal
//   Three branches: counter < CVal, counter == CVal, counter > CVal
//   Each branch selects a different ItCount value for the target formula
//
//   000D: inc [esi+0xD0]            ; exec_counter++
//   0013: mov eax, [esi+0xD0]      ; eax = counter
//   001B: mov eax, [edi-0x0C]      ; CVal, clamped to [1, 100]
//   0034: cmp edx, eax              ; counter vs CVal
//   0036: jl 0x3                   ; counter < CVal → ItCount_Lo
//   0038: je 0x84                   ; counter == CVal → ItCount_Eq
//   003A: jmp 0x8A                  ; counter > CVal → ItCount_Hi
//
//   ItCount values clamped to [0, 100]
//   Formula_nr from [edi-0x10], clamped to [1, 6]
//   Write to [esi + formula_nr*4 + 0x48] with sign preservation

static void cond_itcount_switch(
    int* itcount,      // itcount[0..6] array (index 0 unused)
    int exec_counter,         // execution counter (incremented before use)
    int CVal,                 // comparison value (default: 1, range 1..100)
    int formula_nr,           // target slot 1..6 (default: 2)
    int ic_lo,                // ItCount when counter < CVal (default: 5)
    int ic_eq,                // ItCount when counter == CVal (default: 0)
    int ic_hi                 // ItCount when counter > CVal (default: 0)
) {
    // Clamp CVal to [1, 100]
    CVal = clamp(CVal, 1, 100);

    // Select ItCount based on counter vs CVal
    int val;
    if (exec_counter < CVal) {
        val = ic_lo;
    } else if (exec_counter == CVal) {
        val = ic_eq;
    } else {
        val = ic_hi;
    }

    // Clamp ItCount to [0, 100]
    val = clamp(val, 0, 100);

    // Clamp formula_nr to [1, 6]
    formula_nr = clamp(formula_nr, 1, 6);

    // Preserve sign bit from current ItCount
    if (itcount[formula_nr] < 0) {
        val |= int(0x80000000);
    }

    itcount[formula_nr] = val;
}

// Standard interface (passthrough — coordinates unchanged)
template<>
cpu_double3 execute_formula<FORMULA__CONDITCOUNTSWITCH>(cpu_double3 p, constant double* params, double &de) {
    // NOTE: This meta-formula modifies IterationState.itcount[] and uses
    // an execution counter. The Metal engine must call cond_itcount_switch()
    // directly, incrementing the counter before each call.
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__CondItcountSwitch = {
    .id = FORMULA__CONDITCOUNTSWITCH,
    .name = "_CondItcountSwitch",
    .param_count = 5,
    .complexity = 152,
    .uses_transcendental = false
};

/*
Original M3 Description:
Warning! Use at your risk, if misused this can freeze the program.

Put other two formulas in the alternation BEFORE pressing calculate.
Best used with IFS formulas, less useful with bulbs but still interesting.

This formula modifies the iteration count (depending on how many
times it's executed) of another formula:

if ExecCount < CVal { ItCount[Formula[Formula nr]] = ICILO }
else if ExecCount == CVal { ItCount[Formula[Formula nr]] = ICIEQ }
else { ItCount[Formula[Formula nr]] = ICIHI }

Because the formula iteration counts are not resetted on iteration start,
you should only modify formulas that are following this one.

Parameters:
  [0] ExecCnt value (1...100): integer (default: 1) — comparison threshold
  [1] Formula nr (1...6): integer (default: 2) — target formula slot
  [2] ItCount If Lo (0...100): integer (default: 5) — count when counter < CVal
  [3] ItCount If Eq (0...100): integer (default: 0) — count when counter == CVal
  [4] ItCount If Hi (0...100): integer (default: 0) — count when counter > CVal

PVar layout:
  [PVar-0x0C] = ExecCnt value (int32, 4 bytes)
  [PVar-0x10] = Formula nr (int32, 4 bytes)
  [PVar-0x14] = ItCount If Lo (int32, 4 bytes)
  [PVar-0x18] = ItCount If Eq (int32, 4 bytes)
  [PVar-0x1C] = ItCount If Hi (int32, 4 bytes)

Context access:
  [esi+0xD0] = execution counter (incremented each call by this formula)
  [esi + formula_nr*4 + 0x48] = ItCount[formula_nr] (int32)
*/
