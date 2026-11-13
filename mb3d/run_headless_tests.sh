#!/bin/bash
# Headless render test for all .m3p files
OUTDIR="headless_test_output"
TIMEOUT=90
WIDTH=200
HEIGHT=150
REPORT="$OUTDIR/test_report.txt"
CSV="$OUTDIR/test_report.csv"

echo "MB3D Headless Render Test Report" > "$REPORT"
echo "================================" >> "$REPORT"
echo "Resolution: ${WIDTH}x${HEIGHT}, Timeout: ${TIMEOUT}s" >> "$REPORT"
echo "Date: $(date)" >> "$REPORT"
echo "" >> "$REPORT"

echo "file,status,time_sec,output_size,output_dims" > "$CSV"

TOTAL=0
PASS=0
FAIL=0
TIMEOUT_COUNT=0
IDX=0

for M3P in M3Parameter/*.m3p; do
  IDX=$((IDX + 1))
  FNAME=$(basename "$M3P")
  SAFENAME=$(echo "$FNAME" | sed 's/[^a-zA-Z0-9._-]/_/g' | sed 's/.m3p$//')
  OUTFILE="$OUTDIR/${SAFENAME}.png"
  TOTAL=$((TOTAL + 1))
  
  echo -n "[$IDX/80] $FNAME ... "
  
  START_TIME=$(date +%s)
  
  # Run with timeout
  timeout $TIMEOUT ./Mandelbulb3D.exe --render "$M3P" --output "$OUTFILE" --width $WIDTH --height $HEIGHT 2>&1 | tail -2
  EXIT_CODE=$?
  
  END_TIME=$(date +%s)
  ELAPSED=$((END_TIME - START_TIME))
  
  if [ $EXIT_CODE -eq 124 ]; then
    echo "TIMEOUT (${ELAPSED}s)"
    echo "  [$IDX] TIMEOUT: $FNAME (${ELAPSED}s)" >> "$REPORT"
    echo "\"$FNAME\",TIMEOUT,$ELAPSED,0,n/a" >> "$CSV"
    TIMEOUT_COUNT=$((TIMEOUT_COUNT + 1))
    # Kill any leftover processes
    taskkill //F //IM Mandelbulb3D.exe > /dev/null 2>&1
    sleep 1
  elif [ $EXIT_CODE -ne 0 ]; then
    echo "FAIL exit=$EXIT_CODE (${ELAPSED}s)"
    echo "  [$IDX] FAIL: $FNAME (exit=$EXIT_CODE, ${ELAPSED}s)" >> "$REPORT"
    echo "\"$FNAME\",FAIL,$ELAPSED,0,n/a" >> "$CSV"
    FAIL=$((FAIL + 1))
  elif [ -f "$OUTFILE" ]; then
    FSIZE=$(stat -c%s "$OUTFILE" 2>/dev/null || echo "0")
    DIMS=$(file "$OUTFILE" | grep -oP '\d+ x \d+' | head -1)
    echo "OK ${DIMS} ${FSIZE}b (${ELAPSED}s)"
    echo "  [$IDX] OK: $FNAME (${DIMS}, ${FSIZE}b, ${ELAPSED}s)" >> "$REPORT"
    echo "\"$FNAME\",OK,$ELAPSED,$FSIZE,\"$DIMS\"" >> "$CSV"
    PASS=$((PASS + 1))
  else
    echo "FAIL no output (${ELAPSED}s)"
    echo "  [$IDX] FAIL: $FNAME (no output file, ${ELAPSED}s)" >> "$REPORT"
    echo "\"$FNAME\",FAIL_NO_OUTPUT,$ELAPSED,0,n/a" >> "$CSV"
    FAIL=$((FAIL + 1))
  fi
done

echo "" >> "$REPORT"
echo "================================" >> "$REPORT"
echo "SUMMARY: $TOTAL total, $PASS passed, $FAIL failed, $TIMEOUT_COUNT timed out" >> "$REPORT"

echo ""
echo "================================"
echo "SUMMARY: $TOTAL total, $PASS passed, $FAIL failed, $TIMEOUT_COUNT timed out"
echo "Report: $REPORT"
echo "CSV: $CSV"
