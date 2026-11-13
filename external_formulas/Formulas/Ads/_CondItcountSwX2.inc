// _CondItcountSwX2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _CondItcountSwX2
// Hand-ported from ASM: 277 bytes, DEoption=-1
// META-FORMULA: modifies iteration counts for TWO formula slots

constexpr unsigned int FORMULA__CONDITCOUNTSWX2 = 10355;

// ASM trace:
//   esi = context, edi = PVar
//   Same as _CondItcountSwitch but applied TWICE: once for slot A, once for slot B.
//   Execution counter [esi+0xD0] incremented ONCE, same value used for both.
//
//   000D: inc [esi+0xD0]             ; exec_counter++ (once)
//   0013: mov eax, [esi+0xD0]       ; edx = counter (saved)
//
//   --- Pass A (0x001B - 0x0081): uses params at standard PVar offsets ---
//   CVal_A = [edi-0x0C], Formula_A = [edi-0x10]
//   IC_Lo_A = [edi-0x14], IC_Eq_A = [edi-0x18], IC_Hi_A = [edi-0x1C]
//
//   --- Pass B (0x0092 - 0x010C): PVar shifted by -0x14 ---
//   0092: mov edi, [esi+0x30]        ; reload PVar
//   0095: add edi, -0x14             ; shift to B parameter set
//   CVal_B = [edi-0x0C] = [PVar-0x20], Formula_B = [edi-0x10] = [PVar-0x24]
//   IC_Lo_B = [edi-0x14] = [PVar-0x28], etc.

// Reuses cond_itcount_switch from _CondItcountSwitch.metal
// For standalone compilation, inline the logic:

static void cond_itcount_switch_inline(
    int* itcount,
    int exec_counter,
    int CVal, int formula_nr,
    int ic_lo, int ic_eq, int ic_hi
) {
    CVal = clamp(CVal, 1, 100);
    int val;
    if (exec_counter < CVal)       val = ic_lo;
    else if (exec_counter == CVal)  val = ic_eq;
    else                            val = ic_hi;
    val = clamp(val, 0, 100);
    formula_nr = clamp(formula_nr, 1, 6);
    if (itcount[formula_nr] < 0) val |= int(0x80000000);
    itcount[formula_nr] = val;
}

static void cond_itcount_swx2(
    int* itcount,       // itcount[0..6] array
    int exec_counter,          // execution counter (already incremented)
    // Slot A parameters:
    int CVal_A, int formula_nr_A,
    int ic_lo_A, int ic_eq_A, int ic_hi_A,
    // Slot B parameters:
    int CVal_B, int formula_nr_B,
    int ic_lo_B, int ic_eq_B, int ic_hi_B
) {
    cond_itcount_switch_inline(itcount, exec_counter,
        CVal_A, formula_nr_A, ic_lo_A, ic_eq_A, ic_hi_A);
    cond_itcount_switch_inline(itcount, exec_counter,
        CVal_B, formula_nr_B, ic_lo_B, ic_eq_B, ic_hi_B);
}

// Standard interface (passthrough)
template<>
cpu_double3 execute_formula<FORMULA__CONDITCOUNTSWX2>(cpu_double3 p, constant double* params, double &de) {
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__CondItcountSwX2 = {
    .id = FORMULA__CONDITCOUNTSWX2,
    .name = "_CondItcountSwX2",
    .param_count = 10,
    .complexity = 277,
    .uses_transcendental = false
};

/*
Original M3 Description:
Warning! Use at your risk, if misused this can freeze the program.

This is an extended version of _CondItcountSwitch that manipulates the
iteration counts of TWO formulas instead of one.

Parameters (Slot A):
  [0] ExecCnt value A (1...100): integer (default: 1)
  [1] Formula nr A (1...6): integer (default: 2)
  [2] ItCount If Lo A (0...100): integer (default: 2)
  [3] ItCount If Eq A (0...100): integer (default: 0)
  [4] ItCount If Hi A (0...100): integer (default: 1)

Parameters (Slot B):
  [5] ExecCnt value B (1...100): integer (default: 3)
  [6] Formula nr B (1...6): integer (default: 3)
  [7] ItCount If Lo B (0...100): integer (default: 0)
  [8] ItCount If Eq B (0...100): integer (default: 3)
  [9] ItCount If Hi B (0...100): integer (default: 1)

PVar layout:
  [PVar-0x0C] = ExecCnt value A (int32)
  [PVar-0x10] = Formula nr A (int32)
  [PVar-0x14] = ItCount If Lo A (int32)
  [PVar-0x18] = ItCount If Eq A (int32)
  [PVar-0x1C] = ItCount If Hi A (int32)
  [PVar-0x20] = ExecCnt value B (int32) — accessed via PVar shift -0x14
  [PVar-0x24] = Formula nr B (int32)
  [PVar-0x28] = ItCount If Lo B (int32)
  [PVar-0x2C] = ItCount If Eq B (int32)
  [PVar-0x30] = ItCount If Hi B (int32)

Context access:
  [esi+0xD0] = execution counter (incremented ONCE, used for both A and B)
  [esi + formula_nr*4 + 0x48] = ItCount[formula_nr]
*/
