program pascal_formula_iteration_guard_probe;
{$mode delphi}
{$asmmode intel}
uses SysUtils, FormulaExecutionGuard;
var G: TFormulaIterationGuard; I, Checks: Integer; Failed, Stop: LongBool;
procedure Check(Value: Boolean; const Message: string);
begin
  if not Value then raise Exception.Create(Message);
  Inc(Checks);
end;
function AsmGuardReturn(Value: Integer): Double;
var LocalGuard: TFormulaIterationGuard;
asm
  pushad
  lea eax, LocalGuard
  xor edx, edx
  xor ecx, ecx
  push 0
  push 0
  call FormulaGuardInit
  popad
  pushad
  mov ecx, eax
  xor edx, edx
  lea eax, LocalGuard
  call FormulaGuardBefore
  popad
  fld1
end;
begin
  ClearFormulaExecutionFailure;
  Check(AsmGuardReturn(0) = 1, 'ASM local frame / register ABI / FP return');
  FormulaGuardInit(G, 0, 0, 0, 5);
  for I := 0 to 5 do FormulaGuardVisitInactive(G, 0, I);
  Failed := False;
  try FormulaGuardVisitInactive(G, 0, 0);
  except on E: EFormulaExecutionError do Failed := Pos('reason=empty_cycle', E.Message) > 0; end;
  Check(Failed, 'empty cycle must fail');
  FormulaGuardInit(G, 0, 0, 0, 5);
  for I := 1 to FormulaCallsWithoutProgressLimit do FormulaGuardBefore(G, 0, 0);
  Failed := False;
  try FormulaGuardBefore(G, -1, 0);
  except on E: EFormulaExecutionError do Failed := Pos('reason=no_progress_budget', E.Message) > 0; end;
  Check(Failed, 'backward counter must not reset work budget');
  FormulaGuardBefore(G, 1, 0);
  Check(G.CallsWithoutProgress = 1, 'strict progress resets work budget');
  for I := 1 to 100003 do FormulaGuardBefore(G, 1, 0);
  Check(G.CallsWithoutProgress = 100004, 'finite repeats beyond 65535 allowed');
  Failed := False;
  try FormulaGuardInit(G, 0, 0, 6, 5);
  except on E: EFormulaExecutionError do Failed := Pos('reason=invalid_slot_range', E.Message) > 0; end;
  Check(Failed, 'invalid range before indexing');
  Stop := False;
  try raise EFormulaExecutionError.Create('first');
  except on E: Exception do ReportFormulaWorkerFailure('test', E, @Stop); end;
  Check(Stop and (Pos('first', FormulaExecutionFailureText) > 0), 'failure sets stop and diagnostic');
  try raise EFormulaExecutionError.Create('second');
  except on E: Exception do ReportFormulaWorkerFailure('test', E, @Stop); end;
  Check(Pos('first', FormulaExecutionFailureText) > 0, 'first failure retained');
  ClearFormulaExecutionFailure;
  Check(FormulaExecutionFailureText = '', 'fresh job clears previous failure');
  FormulaGuardInit(G, 0, 0, 0, 0);
  FormulaGuardBefore(G, 0, 0);
  Check(G.CallsWithoutProgress = 1, 'healthy recovery');
  WriteLn('PASS ', Checks);
end.
