#!/bin/bash
OUTDIR="headless_test_output"
TIMEOUT=120
WIDTH=200
HEIGHT=150
REPORT="$OUTDIR/test_report2.txt"

echo "MB3D Headless Render Test Report v2" > "$REPORT"
echo "================================" >> "$REPORT"
echo "Resolution: ${WIDTH}x${HEIGHT}, Timeout: ${TIMEOUT}s" >> "$REPORT"
echo "Date: $(date)" >> "$REPORT"
echo "" >> "$REPORT"

TOTAL=0; PASS=0; FAIL=0; TIMEOUT_COUNT=0; IDX=0

for M3P in M3Parameter/*.m3p; do
  IDX=$((IDX + 1))
  FNAME=$(basename "$M3P")
  SAFENAME=$(echo "$FNAME" | sed 's/[^a-zA-Z0-9._-]/_/g' | sed 's/.m3p$//')
  OUTFILE="$OUTDIR/${SAFENAME}.png"
  TOTAL=$((TOTAL + 1))
  
  echo -n "[$IDX] $FNAME ... "
  
  START_TIME=$(date +%s)
  OUTPUT=$(timeout $TIMEOUT ./Mandelbulb3D.exe --render "$M3P" --output "$OUTFILE" --width $WIDTH --height $HEIGHT 2>&1)
  EXIT_CODE=$?
  END_TIME=$(date +%s)
  ELAPSED=$((END_TIME - START_TIME))
  
  if [ $EXIT_CODE -eq 124 ]; then
    echo "TIMEOUT (${ELAPSED}s)"
    echo "  [$IDX] TIMEOUT: $FNAME (${ELAPSED}s)" >> "$REPORT"
    echo "  Last output: $(echo "$OUTPUT" | tail -1)" >> "$REPORT"
    TIMEOUT_COUNT=$((TIMEOUT_COUNT + 1))
    taskkill //F //IM Mandelbulb3D.exe > /dev/null 2>&1
    sleep 1
  elif [ $EXIT_CODE -ne 0 ]; then
    echo "FAIL exit=$EXIT_CODE (${ELAPSED}s)"
    echo "  [$IDX] FAIL: $FNAME (exit=$EXIT_CODE, ${ELAPSED}s)" >> "$REPORT"
    FAIL=$((FAIL + 1))
  elif [ -f "$OUTFILE" ]; then
    FSIZE=$(stat -c%s "$OUTFILE" 2>/dev/null || echo "0")
    echo "OK ${FSIZE}b (${ELAPSED}s)"
    echo "  [$IDX] OK: $FNAME (${FSIZE}b, ${ELAPSED}s)" >> "$REPORT"
    PASS=$((PASS + 1))
  else
    echo "FAIL no output (${ELAPSED}s)"
    echo "  [$IDX] FAIL: $FNAME (no output, ${ELAPSED}s)" >> "$REPORT"
    echo "  Last output: $(echo "$OUTPUT" | tail -1)" >> "$REPORT"
    FAIL=$((FAIL + 1))
  fi
done

echo "" >> "$REPORT"
echo "================================" >> "$REPORT"
echo "SUMMARY: $TOTAL total, $PASS passed, $FAIL failed, $TIMEOUT_COUNT timed out" >> "$REPORT"
echo ""
echo "================================"
echo "SUMMARY: $TOTAL total, $PASS passed, $FAIL failed, $TIMEOUT_COUNT timed out"
