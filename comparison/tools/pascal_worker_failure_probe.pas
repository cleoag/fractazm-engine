program pascal_worker_failure_probe;
{$mode delphi}
uses Windows, Classes, SysUtils, FormulaExecutionGuard;

type
  TFailureWorker = class(TThread)
  protected
    procedure Execute; override;
  end;

var
  Gate: THandle;
  Stop: LongBool;
  Finalized, Checks: LongInt;
  Workers: array[0..3] of TFailureWorker;
  I, Run: Integer;
  Diagnostic: string;
  Guard: TFormulaIterationGuard;

procedure Check(Value: Boolean; const Description: string);
begin
  if not Value then raise Exception.Create(Description);
  Inc(Checks);
end;

procedure TFailureWorker.Execute;
begin
  try
    WaitForSingleObject(Gate, INFINITE);
    try
      raise EFormulaExecutionError.Create('simultaneous worker rejection');
    except
      on E: Exception do ReportFormulaWorkerFailure('four-worker probe', E, @Stop);
    end;
  finally
    InterlockedIncrement(Finalized);
  end;
end;

begin
  Gate := CreateEvent(nil, True, False, nil);
  Check(Gate <> 0, 'create worker start barrier');
  try
    for Run := 1 to 2 do
    begin
      ClearFormulaExecutionFailure;
      Stop := False;
      Finalized := 0;
      ResetEvent(Gate);
      if Run = 2 then
      begin
        try
          raise EFormulaExecutionError.Create('original first failure');
        except
          on E: Exception do ReportFormulaWorkerFailure('first', E, @Stop);
        end;
      end;
      for I := Low(Workers) to High(Workers) do
        Workers[I] := TFailureWorker.Create(True);
      for I := Low(Workers) to High(Workers) do Workers[I].Start;
      SetEvent(Gate);
      for I := Low(Workers) to High(Workers) do Workers[I].WaitFor;
      for I := Low(Workers) to High(Workers) do
      begin
        Check(Workers[I].FatalException = nil, 'worker error was caught');
        Workers[I].Free;
      end;
      Check(Finalized = Length(Workers), 'all worker finally blocks completed');
      Check(Stop, 'shared cancellation published');
      Diagnostic := FormulaExecutionFailureText;
      if Run = 1 then
        Check(Diagnostic = 'four-worker probe: EFormulaExecutionError: simultaneous worker rejection',
          'one complete main-thread diagnostic')
      else
        Check(Diagnostic = 'first: EFormulaExecutionError: original first failure',
          'concurrent secondary failures preserve first cause');
      ClearFormulaExecutionFailure;
      Check(FormulaExecutionFailureText = '', 'failure cleared after workers joined');
      FormulaGuardInit(Guard, 0, 0, 0, 0);
      for I := 1 to 2048 do FormulaGuardBefore(Guard, I, 0);
      Check(Guard.CallsWithoutProgress = 1, 'healthy job after failed workers');
    end;
  finally
    CloseHandle(Gate);
  end;
  WriteLn('PASS ', Checks);
end.
