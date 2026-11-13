#!/bin/bash
PARAMDIR="M3Parameter"
OUTDIR="headless_test_output/native"
TIMEOUT=300
REPORT="$OUTDIR/native_test_report.txt"

mkdir -p "$OUTDIR"

echo "MB3D Native Resolution Test Report" > "$REPORT"
echo "===================================" >> "$REPORT"
echo "Timeout: ${TIMEOUT}s per scene" >> "$REPORT"
echo "Date: $(date)" >> "$REPORT"
echo "" >> "$REPORT"

TOTAL=0; PASS=0; FAIL=0; TIMEOUT_COUNT=0; IDX=0

for M3P in "$PARAMDIR"/*.m3p; do
  IDX=$((IDX + 1))
  FNAME=$(basename "$M3P")
  SAFENAME=$(echo "$FNAME" | sed 's/[^a-zA-Z0-9._-]/_/g' | sed 's/.m3p$//')
  OUTFILE="$OUTDIR/${SAFENAME}.png"
  TOTAL=$((TOTAL + 1))

  echo -n "[$IDX] $FNAME ... "

  START_TIME=$(date +%s)
  OUTPUT=$(timeout $TIMEOUT ./Mandelbulb3D.exe --render "$M3P" --output "$OUTFILE" 2>&1)
  EXIT_CODE=$?
  END_TIME=$(date +%s)
  ELAPSED=$((END_TIME - START_TIME))

  # Extract resolution from output
  RES=$(echo "$OUTPUT" | grep -oP 'Rendering \K[0-9]+x[0-9]+')

  if [ $EXIT_CODE -eq 124 ]; then
    echo "TIMEOUT ${RES} (${ELAPSED}s)"
    echo "  [$IDX] TIMEOUT: $FNAME ${RES} (${ELAPSED}s)" >> "$REPORT"
    echo "  Last output: $(echo "$OUTPUT" | tail -1)" >> "$REPORT"
    TIMEOUT_COUNT=$((TIMEOUT_COUNT + 1))
    taskkill //F //IM Mandelbulb3D.exe > /dev/null 2>&1
    sleep 1
  elif [ $EXIT_CODE -ne 0 ]; then
    echo "FAIL exit=$EXIT_CODE ${RES} (${ELAPSED}s)"
    echo "  [$IDX] FAIL: $FNAME (exit=$EXIT_CODE, ${RES}, ${ELAPSED}s)" >> "$REPORT"
    echo "  Last output: $(echo "$OUTPUT" | tail -1)" >> "$REPORT"
    FAIL=$((FAIL + 1))
  elif [ -f "$OUTFILE" ]; then
    FSIZE=$(stat -c%s "$OUTFILE" 2>/dev/null || echo "0")
    echo "OK ${RES} ${FSIZE}b (${ELAPSED}s)"
    echo "  [$IDX] OK: $FNAME (${RES}, ${FSIZE}b, ${ELAPSED}s)" >> "$REPORT"
    PASS=$((PASS + 1))
  else
    echo "FAIL no output ${RES} (${ELAPSED}s)"
    echo "  [$IDX] FAIL: $FNAME (no output, ${RES}, ${ELAPSED}s)" >> "$REPORT"
    echo "  Last output: $(echo "$OUTPUT" | tail -1)" >> "$REPORT"
    FAIL=$((FAIL + 1))
  fi
done

echo "" >> "$REPORT"
echo "===================================" >> "$REPORT"
echo "SUMMARY: $TOTAL total, $PASS passed, $FAIL failed, $TIMEOUT_COUNT timed out" >> "$REPORT"
echo ""
echo "==================================="
echo "SUMMARY: $TOTAL total, $PASS passed, $FAIL failed, $TIMEOUT_COUNT timed out"
cat "$REPORT"
