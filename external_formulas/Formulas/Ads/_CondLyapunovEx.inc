// _CondLyapunovEx.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _CondLyapunovEx
// Hand-ported from ASM: 341 bytes, DEoption=-1
// META-FORMULA: Extended Lyapunov switching with custom iteration counts

constexpr unsigned int FORMULA__CONDLYAPUNOVEX = 10357;

// ASM trace:
//   esi = context, edi = PVar
//   Same Lyapunov bit extraction as _CondLyapunov, but with configurable
//   IC_Low and IC_High values instead of hardcoded 0 and 1.
//
//   000E: inc [esi+0xD0]             ; exec_counter++
//   001C: and edx, 0xF               ; N2 = counter & 15
//   Bit extraction loop → bit = (sequential_code >> max(0, N2-1)) & 1
//
//   0030: jne 0xC5                   ; if bit == 1, jump
//
//   bit == 0:
//     Formula A gets IC_Low  ([edi-0x18], clamped [0,100])
//     Formula B gets IC_High ([edi-0x1C], clamped [1,100])
//
//   bit == 1:
//     Formula B gets IC_Low  ([edi-0x18], clamped [0,100])
//     Formula A gets IC_High ([edi-0x1C], clamped [1,100])
//
//   Write uses standard formula_nr clamping [1,6] and sign preservation.

static void cond_lyapunov_ex(
    int* itcount,       // itcount[0..6] array
    int exec_counter,          // execution counter (already incremented)
    int sequential_code,       // bit pattern (default: 33964)
    int formula_A,             // Formula A slot 1..6 (default: 2)
    int formula_B,             // Formula B slot 1..6 (default: 3)
    int ic_low,                // low iteration count (default: 0, range 0..100)
    int ic_high                // high iteration count (default: 2, range 1..100)
) {
    // Extract bit from sequential code
    int N2 = exec_counter & 0xF;
    int bit_pos = max(0, N2 - 1);
    int bit = (sequential_code >> bit_pos) & 1;

    // Clamp formula slots and IC values
    formula_A = clamp(formula_A, 1, 6);
    formula_B = clamp(formula_B, 1, 6);
    ic_low  = clamp(ic_low, 0, 100);
    ic_high = clamp(ic_high, 1, 100);

    int val_A, val_B;
    if (bit == 0) {
        val_A = ic_low;    // Formula A gets low count
        val_B = ic_high;   // Formula B gets high count
    } else {
        val_A = ic_high;   // Formula A gets high count
        val_B = ic_low;    // Formula B gets low count
    }

    // Write ItCount for Formula A with sign preservation
    if (itcount[formula_A] < 0) val_A |= int(0x80000000);
    itcount[formula_A] = val_A;

    // Write ItCount for Formula B with sign preservation
    if (itcount[formula_B] < 0) val_B |= int(0x80000000);
    itcount[formula_B] = val_B;
}

// Standard interface (passthrough)
template<>
cpu_double3 execute_formula<FORMULA__CONDLYAPUNOVEX>(cpu_double3 p, constant double* params, double &de) {
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__CondLyapunovEx = {
    .id = FORMULA__CONDLYAPUNOVEX,
    .name = "_CondLyapunovEx",
    .param_count = 5,
    .complexity = 341,
    .uses_transcendental = false
};

/*
Original M3 Description:
Warning! Use at your risk, if misused this can freeze the program.

This formula modifies the iteration counts of two formulas,
in a similar manner of Lyapunov's formula sequence style.
Use this in some hybrids to get a better control of the flow
than the normal CondLyapunov!

The itercount can be set on a custom value 0 to 100 instead of just 1 or 0.

Luca GN 2012

Algorithm:
  N2 = exec_counter & 15
  bit_pos = max(0, N2 - 1)
  bit = (sequential_code >> bit_pos) & 1

  if bit == 0: Formula A = IC_Low, Formula B = IC_High
  if bit == 1: Formula A = IC_High, Formula B = IC_Low

Parameters:
  [0] Sequential code: integer (default: 33964) — 15-bit pattern
  [1] Formula A (1...6): integer (default: 2)
  [2] Formula B (1...6): integer (default: 3)
  [3] I.C. Low (0...100): integer (default: 0)
  [4] I.C. High (1...100): integer (default: 2)

PVar layout:
  [PVar-0x0C] = Sequential code (int32)
  [PVar-0x10] = Formula A (int32)
  [PVar-0x14] = Formula B (int32)
  [PVar-0x18] = I.C. Low (int32)
  [PVar-0x1C] = I.C. High (int32)

Context access:
  [esi+0xD0] = execution counter (incremented each call)
  [esi + formula_nr*4 + 0x48] = ItCount[formula_nr]

NOTES:
  - IC_Low is clamped to [0, 100], IC_High is clamped to [1, 100]
    (high can never be 0, ensuring at least one formula always executes)
  - Same bit extraction as _CondLyapunov (sar loop, positions 0..14)
  - Extended version replaces the hardcoded 0/1 with configurable values,
    allowing finer control over hybrid formula mixing
*/
