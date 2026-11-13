program LcgDumpTool;
{
  Phase 2 LCG cross-validation harness.

  Prints the PasLcg state and u01 stream for threadIds 0..3, N=10 samples each,
  in a fixed format suitable for bytewise diff against
  comparison/fixtures/phase2_lcg_pascal.txt in the mb3d_cpp_port repo.

  See docs/BUILD.md for build/run instructions.
}
{$MODE Delphi}
{$APPTYPE CONSOLE}

uses SysUtils, PasLcg;

function DoubleToHex(d: Double): string;
var q: QWord;
begin
  Move(d, q, SizeOf(q));
  Result := LowerCase(IntToHex(q, 16));
end;

function StateHex(s: TLcgState): string;
begin
  Result := LowerCase(IntToHex(s, 8));
end;

procedure DumpThread(threadId: Integer);
var
  state: TLcgState;
  i: Integer;
  u: Double;
begin
  state := PasLcgInitFromThread(threadId);
  Writeln(Format('thread=%d i=-1 state=%s u01_bits=- u01=-', [threadId, StateHex(state)]));
  for i := 0 to 9 do
  begin
    u := PasLcgU01(state);
    Writeln(Format('thread=%d i=%d state=%s u01_bits=%s u01=%.17g',
      [threadId, i, StateHex(state), DoubleToHex(u), u]));
  end;
end;

var t: Integer;
begin
  Writeln('# phase2 LCG cross-validation fixture');
  Writeln('# format: thread=<d> i=<d> state=<8-hex> u01_bits=<16-hex> u01=<17-digit-double>');
  for t := 0 to 3 do
    DumpThread(t);
end.
