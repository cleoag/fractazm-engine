# Plan: Restore JIT Formula Compilation Under FPC

## Context

MB3D's formula editor lets users write fractal formulas in Pascal source, compile to native x86 at runtime, and save as `.m3f` files. This was powered by **PAX Compiler** (commercial, Delphi-only), disabled during FPC migration. Goal: restore source→native compilation under FPC.

## Current State

### What works without PAX
- **457/460 .m3f files** have pre-compiled x86 in `[CODE]` sections — load via `VirtualAlloc`, no compiler needed
- **10 built-in formulas** — hardcoded in `formulas.pas`
- **All .m3p parameter files** — reference formulas by name/ID, loading is compiler-independent

### What's broken
- **3 .m3f files** with `[SOURCE]` only (no `[CODE]`): `JITBenesiPineTree.m3f`, `JITTrigExample.m3f`, `JIT_IQ_Bulb.m3f`
- **Formula editor** — Compile (F9) calls `TFormulaCompiler.CompileFormula` → returns `nil` → crash
- **Creating new JIT formulas** — impossible

### Why PascalScript won't work
PascalScript uses a **bytecode interpreter**, not native code generation. Formulas are called millions of times per render — bytecode would be **100-500x slower**. The formula procedure type `ThybridIteration = procedure(var x,y,z,w: Double; PIteration3D: TPIteration3D)` must be a **native function pointer**.

## Approach: FPC-as-JIT via DLL Compilation

Use `ppc386.exe` to compile formula source into a **temporary DLL**, then load via `LoadLibrary` + `GetProcAddress`.

**Advantages:**
- Native x86 code (same quality as built-in formulas)
- Same compiler as main app — no calling convention mismatches
- No new dependencies (FPC already installed)
- Full Pascal syntax support (PAX only supported a subset)
- DLL uses `TypeDefinitions` directly — field offsets always in sync (PAX registered them manually)

**Pipeline:**
```
Source code → PreprocessCode (inject const/param vars) → Generate temp .pas (library wrapper)
→ ppc386.exe -Mdelphi → temp .dll → LoadLibrary → GetProcAddress → ThybridIteration pointer
```

## Backward Compatibility: ALL existing files work

### .m3p parameter files — NO IMPACT
`.m3p` stores formula references as `iFnr` (integer ID) + `CustomFname` (string). Loading via `LoadCustomFormula` → finds `.m3f` → loads `[CODE]` or compiles `[SOURCE]`. The compiler backend change is invisible to `.m3p` loading.

### .m3f files with [CODE] — NO IMPACT
Pre-compiled x86 hex bytes loaded via `VirtualAlloc`. This path has zero PAX dependency. The 457 `[CODE]`-based formulas are completely unaffected.

### .m3f files with [SOURCE] — RESTORED
The 3 `[SOURCE]`-only formulas will now compile via FPC instead of PAX. Output is functionally identical (native x86 procedure pointer). FPC in `{$mode delphi}` generates compatible calling conventions.

### Calling convention safety
`ThybridIteration` has 4 `var Double` params + 1 pointer = 5 params → too many for register convention → **all params go on stack**. Stack-based passing is identical between Delphi and FPC. Already verified: 457 pre-compiled .m3f formulas work under FPC.

## Files to Create/Modify

### 1. CREATE: `formula/FPCFormulaCompiler.pas` (~200 lines)

```pascal
TFPCCompiledFormula = class(TCompiledArtifact)
  FDLLHandle: THandle;
  FDLLPath: String;
  destructor Destroy; override;  // FreeLibrary + delete temp DLL
end;

TFPCFormulaCompiler = class
  function CompileFormula(const Formula: TJITFormula): TFPCCompiledFormula;
  function PreprocessCode(const Code: String; const Formula: TJITFormula): String;
end;
```

**CompileFormula implementation:**
1. Create temp dir `%TEMP%\mb3d_jit\`
2. Generate `mb3d_jit_XXXX.pas` — library wrapping the formula:
   ```pascal
   library mb3d_jit_XXXX;
   {$mode delphi}
   uses Math, TypeDefinitions;

   [preprocessed formula source inserted here]

   exports UserProcName;
   begin end.
   ```
3. Run `ppc386.exe -Mdelphi -O2 -Fu<project_path> mb3d_jit_XXXX.pas`
4. On success: `LoadLibrary('mb3d_jit_XXXX.dll')`, `GetProcAddress(Handle, 'UserProcName')`
5. Set `Result.CodePointer := procAddr`, `Result.CodeSize`
6. On failure: parse FPC error output → `Result.AddErrorMessage()`

**Destructor**: `FreeLibrary(FDLLHandle)` then `DeleteFile(FDLLPath)`

### 2. MODIFY: `formula/FormulaCompiler.pas`

In `TFormulaCompiler`:
- Add `{$IFDEF FPC} FDelegate: TFPCFormulaCompiler; {$ENDIF}` alongside existing PAX delegate
- `CompileFormula`: delegate to FPC compiler under `{$IFDEF FPC}`
- `PreprocessCode`: reuse existing string manipulation (already PAX-independent in logic, just wrapped in ifdefs)
- `TFormulaCompilerRegistry.GetCompilerInstance`: return FPC-backed compiler

### 3. MODIFY: `CustomFormulas.pas` (lines 950-981)

The `[SOURCE]` path already calls `TFormulaCompilerRegistry.GetCompilerInstance(langDELPHI).CompileFormula(ParseResult)`. Need to:
- Handle DLL lifetime: `TFPCCompiledFormula` DLL handle must stay loaded while formula is active
- Store DLL handle in `TCustomFormula` (add field) or keep `TFPCCompiledFormula` alive
- Set `bCPmemReserved := False` for DLL-backed formulas (memory owned by DLL, freed via `FreeLibrary`)

### 4. MODIFY: `formula/JITFormulaEditGUI.pas`

Minimal changes:
- The `Compile` function (line 520) already goes through `TFormulaCompilerRegistry` — will automatically use FPC backend
- Guard the test execution at line 638 with proper error handling for the new backend

### 5. MODIFY: `Mandelbulb3D.lpi`
- Add `FPCFormulaCompiler` unit to project

### 6. NOT MODIFIED (no changes needed):
- `formulas.pas`, `JITFormulas.pas`, `TypeDefinitions.pas`, `Calc.pas`, `CalcThread.pas`
- Any `.m3p` or `.m3f` files

## FPC Compiler Path

Discovery strategy:
1. Check `PATH` for `ppc386.exe`
2. Known location: `C:\FPC\3.2.2\bin\i386-Win32\ppc386.exe`
3. User override via INI (future)

Initial implementation: PATH + hardcoded fallback.

## Verification

### Step 1: Build
```bash
/c/lazarus/lazbuild.exe --cpu=i386 --os=win32 ... Mandelbulb3D.lpi
# Must compile cleanly
```

### Step 2: Test [SOURCE] formula compilation
Create a test .m3p referencing `JIT_IQ_Bulb` formula:
```bash
Mandelbulb3D.exe --render test_jit.m3p --output test_jit.png --width 480 --height 360
# Must: find JIT_IQ_Bulb.m3f → compile [SOURCE] via FPC → render fractal
```

### Step 3: Backward compatibility
```bash
bash run_headless_tests.sh        # 79/80 existing scenes still pass
bash tools/run_formula_tests.sh   # 21/21 formula tests still pass
```

### Step 4: Formula editor (manual)
1. Launch MB3D → open formula editor → load `JIT_IQ_Bulb.m3f`
2. Press F9 → compilation succeeds, info panel shows success
3. Press F5 → save as `JITtest.m3f` → file has `[SOURCE]` section

## Risks

| Risk | Mitigation |
|------|-----------|
| FPC not on user machine | Graceful error: "FPC compiler not found"; [CODE] formulas still work |
| Compile time ~1-2s | Acceptable for editor; cache DLLs by content hash |
| Temp DLL locked by OS | Unique filenames per compile; cleanup on exit |
| TypeDefinitions changes | DLL uses same source → always in sync (better than PAX) |
