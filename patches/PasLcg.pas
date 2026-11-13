{
  PasLcg — deterministic 32-bit Linear Congruential Generator for the
  Mandelbulb3D render path. Replaces uses of System.Random in render
  threads so Pascal and C++ produce byte-identical pseudo-random streams.

  See docs/PARITY_TRACE.md in the mb3d_cpp_port repo for the
  canonical contract (formula, seeding, advance-then-sample ordering,
  24-bit mantissa, cross-platform guarantees).

  Spec version: Phase 2, Task 3 of 16 (LCG formula is the classical
  Microsoft-C LCG: state := state * 214013 + 2531011).
}
unit PasLcg;
{$MODE Delphi}
interface
type
  TLcgState = Cardinal;

function PasLcgInitFromThread(threadId: Integer): TLcgState; inline;
function PasLcgNext(var state: TLcgState): TLcgState; inline;
function PasLcgU01(var state: TLcgState): Double; inline;

implementation

function PasLcgInitFromThread(threadId: Integer): TLcgState;
begin
  Result := Cardinal(threadId + 1) * Cardinal($324594A1) + Cardinal($24563487);
end;

function PasLcgNext(var state: TLcgState): TLcgState;
begin
  state := state * Cardinal(214013) + Cardinal(2531011);
  Result := state;
end;

function PasLcgU01(var state: TLcgState): Double;
var
  top24: Cardinal;
begin
  PasLcgNext(state);
  top24 := state shr 8;
  // FPC Delphi-mode rejects Double(LongWord) as a numeric cast (it would
  // reinterpret bits). Use Int64 as the bridge — it widens the 24-bit
  // payload into a signed integer the compiler will implicitly convert to
  // Double, giving us the exact /16777216.0 division we specified.
  Result := Int64(top24) / 16777216.0;
end;

end.
