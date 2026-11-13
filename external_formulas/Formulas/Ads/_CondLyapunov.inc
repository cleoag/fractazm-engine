// _CondLyapunov.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _CondLyapunov
// Hand-ported from ASM: 245 bytes, DEoption=-1
// META-FORMULA: Lyapunov-style binary sequence switching between two formulas

constexpr unsigned int FORMULA__CONDLYAPUNOV = 10356;

// ASM trace:
//   esi = context, edi = PVar
//   Increments execution counter, extracts bit from sequential code,
//   then sets ItCount for Formula A and Formula B to 0/1 or 1/0.
//
//   000E: inc [esi+0xD0]             ; exec_counter++
//   0014: mov eax, [esi+0xD0]       ; counter
//   001C: and edx, 0xF               ; N2 = counter & 15
//
//   Bit extraction loop (0x0020-0x002D):
//     0020: mov eax, [edi-0x0C]      ; eax = sequential_code
//     0023: dec edx                   ; N2--
//     0024: cmp edx, 0
//     0027: jle 0x2D                  ; if N2 ≤ 0, done
//     0029: sar eax, 1               ; arithmetic shift right
//     002B: jmp 0x23                  ; loop
//     002D: and eax, 1               ; bit = eax & 1
//
//   bit_position = max(0, (counter & 0xF) - 1)
//   bit = (sequential_code >> bit_position) & 1
//
//   0030: jne 0x92                   ; if bit == 1, jump
//
//   bit == 0: Formula A = 0 (skip), Formula B = 1 (execute)
//   bit == 1: Formula A = 1 (execute), Formula B = 0 (skip)
//
//   Write uses standard formula_nr clamping [1,6] and sign preservation.

static void cond_lyapunov(
    int* itcount,       // itcount[0..6] array
    int exec_counter,          // execution counter (already incremented)
    int sequential_code,       // bit pattern (default: 33964 = 0x849C)
    int formula_A,             // Formula A slot 1..6 (default: 2)
    int formula_B              // Formula B slot 1..6 (default: 3)
) {
    // Extract bit from sequential code using counter position
    // bit_position = max(0, (counter & 0xF) - 1), gives positions 0..14
    int N2 = exec_counter & 0xF;
    int bit_pos = max(0, N2 - 1);
    int bit = (sequential_code >> bit_pos) & 1;

    // Clamp formula slots to [1, 6]
    formula_A = clamp(formula_A, 1, 6);
    formula_B = clamp(formula_B, 1, 6);

    int val_A, val_B;
    if (bit == 0) {
        val_A = 0;   // skip Formula A
        val_B = 1;   // execute Formula B
    } else {
        val_A = 1;   // execute Formula A
        val_B = 0;   // skip Formula B
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
cpu_double3 execute_formula<FORMULA__CONDLYAPUNOV>(cpu_double3 p, constant double* params, double &de) {
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__CondLyapunov = {
    .id = FORMULA__CONDLYAPUNOV,
    .name = "_CondLyapunov",
    .param_count = 5,
    .complexity = 245,
    .uses_transcendental = false
};

/*
Original M3 Description:
Warning! Use at your risk, if misused this can freeze the program.

This formula modifies the iteration counts of two formulas,
in a similar manner of Lyapunov's formula sequence style.

Algorithm:
  N2 = exec_counter & 15       (bit position, 0-15 cycle)
  bit_pos = max(0, N2 - 1)     (positions 0..14)
  bit = (sequential_code >> bit_pos) & 1

  if bit == 0: Formula A skipped (IC=0), Formula B executed (IC=1)
  if bit == 1: Formula A executed (IC=1), Formula B skipped (IC=0)

Default sequential_code = 33964 = 0x849C = binary 1000010010011100
This gives a 15-bit repeating pattern that alternates between the two
formulas in a pseudo-chaotic sequence.

Luca GN 2012 (suggested by Aexion)

Parameters:
  [0] Sequential code: integer (default: 33964) — 15-bit pattern
  [1] Formula A (1...6): integer (default: 2) — first formula slot
  [2] Formula B (1...6): integer (default: 3) — second formula slot
  [3] Freq Hi: integer (default: 2) — UNUSED in ASM code
  [4] Freq Lo: integer (default: 0) — UNUSED in ASM code

PVar layout:
  [PVar-0x0C] = Sequential code (int32)
  [PVar-0x10] = Formula A (int32)
  [PVar-0x14] = Formula B (int32)
  [PVar-0x18] = Freq Hi (int32, declared but not accessed by ASM)
  [PVar-0x1C] = Freq Lo (int32, declared but not accessed by ASM)

Context access:
  [esi+0xD0] = execution counter (incremented each call)
  [esi + formula_nr*4 + 0x48] = ItCount[formula_nr]

NOTES:
  - The bit extraction uses arithmetic shift right (sar), so the sign bit
    of sequential_code propagates. For typical positive values this doesn't
    matter, but negative codes will have bit 14 always = 1.
  - Freq Hi / Freq Lo parameters are declared in the M3 but never read
    by the ASM code. _CondLyapunovEx was created to add custom IC values.
*/
