#!/usr/bin/env bash
# validate_nav_reset.sh
#
# Validates IEEE 802.11 NAV early-reset (Section 10.3.2.4):
#   When a third-party node sets NAV from an overheard RTS, and the expected
#   CTS (or DATA after CTS) never arrives, the NAV is reset early instead of
#   blocking the node for the full exchange duration.
#
# Run from the Code/ directory on Linux:
#   cd Code && bash input/validation/nav_reset_scenarios/validate_nav_reset.sh
#
# Scenarios:
#   1) Three BSSs (A, B, C) on the same channel with RTS/CTS enabled.
#      BSS C is between A and B (A at 0m, C at 4m, B at 8m).
#      Frequent collisions between A and B cause CTS failures.
#      C overhears failed RTS → NAV reset expected.
#
#   2) Two BSSs (A, B) on the same channel, 6m apart with RTS/CTS.
#      Each overhears the other's failed RTS → NAV resets expected.
#
# Pass criteria:
#   - Build succeeds
#   - Simulator runs without crash
#   - Node logs contain "NAV RESET" messages (proving the mechanism fires)
#   - All BSSs achieve non-zero throughput (no starvation)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CODE_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"
MAIN_DIR="$CODE_DIR/main"
BIN="$MAIN_DIR/komondor_main"
OUTPUT_DIR="$CODE_DIR/output"

SIM_TIME=1
SEED=1

PASS=0
FAIL=0

check() {
    local label="$1"
    local condition="$2"
    if [ "$condition" = "1" ]; then
        echo "  [PASS] $label"
        PASS=$((PASS + 1))
    else
        echo "  [FAIL] $label"
        FAIL=$((FAIL + 1))
    fi
}

echo "============================================="
echo " NAV Reset Validation"
echo "============================================="
echo ""

# ---- Build ----
echo "[Step 1] Building..."
cd "$MAIN_DIR"
if make 2>&1 | tail -3; then
    echo "  [PASS] Build succeeded"
    PASS=$((PASS + 1))
else
    echo "  [FAIL] Build failed"
    exit 1
fi
echo ""

# ---- Prepare ----
mkdir -p "$OUTPUT_DIR"

# ============================================================
# Scenario 1: Three BSSs — AP_C in the middle
# ============================================================
echo "[Step 2] Scenario 1: Three BSSs (A at 0m, C at 4m, B at 8m)"
echo "  AP_C is between AP_A and AP_B; all on same channel with RTS/CTS."
echo "  A and B collide frequently → C overhears failed RTS → NAV reset expected."
echo ""

SIM_CODE="nav_reset_s1"
OUT1="$OUTPUT_DIR/out_nav_reset_1.txt"
rm -f "$OUT1"
# Remove old log files for this sim code
rm -f "$OUTPUT_DIR/logs_output_${SIM_CODE}"_*.txt

"$BIN" --nodes "$SCRIPT_DIR/input_nodes_nav_reset_1.csv" \
       --out "$OUT1" \
       --code "$SIM_CODE" \
       --logs-node 0 --save-node 1 \
       --time "$SIM_TIME" --seed "$SEED" \
       > "$OUTPUT_DIR/console_${SIM_CODE}.txt" 2>&1

echo "  Simulator completed. Checking results..."

# Check throughput: all BSSs should have > 0 Mbps
if [ -f "$OUT1" ]; then
    TPT_LINE=$(head -1 "$OUT1")
    TPT_A=$(echo "$TPT_LINE" | cut -d';' -f2 | cut -d'{' -f2 | cut -d'}' -f1 | cut -d',' -f1)
    TPT_B=$(echo "$TPT_LINE" | cut -d';' -f2 | cut -d'{' -f2 | cut -d'}' -f1 | cut -d',' -f2)
    TPT_C=$(echo "$TPT_LINE" | cut -d';' -f2 | cut -d'{' -f2 | cut -d'}' -f1 | cut -d',' -f3)

    A_OK=$(echo "$TPT_A > 0" | bc)
    B_OK=$(echo "$TPT_B > 0" | bc)
    C_OK=$(echo "$TPT_C > 0" | bc)

    check "BSS_A throughput > 0 (actual: ${TPT_A} Mbps)" "$A_OK"
    check "BSS_B throughput > 0 (actual: ${TPT_B} Mbps)" "$B_OK"
    check "BSS_C throughput > 0 (actual: ${TPT_C} Mbps)" "$C_OK"
else
    check "Output file exists" "0"
fi

# Check for NAV RESET log messages in any node log
NAV_RESET_COUNT=0
NAV_RESET_CTS_COUNT=0
NAV_RESET_DATA_COUNT=0
CTS_TIMER_COUNT=0
for logfile in "$OUTPUT_DIR/logs_output_${SIM_CODE}"_*.txt; do
    if [ -f "$logfile" ]; then
        COUNT=$(grep -c "NAV RESET" "$logfile" 2>/dev/null || true)
        NAV_RESET_COUNT=$((NAV_RESET_COUNT + COUNT))
        COUNT=$(grep -c "NAV RESET: CTS not received" "$logfile" 2>/dev/null || true)
        NAV_RESET_CTS_COUNT=$((NAV_RESET_CTS_COUNT + COUNT))
        COUNT=$(grep -c "NAV RESET: DATA not received" "$logfile" 2>/dev/null || true)
        NAV_RESET_DATA_COUNT=$((NAV_RESET_DATA_COUNT + COUNT))
        COUNT=$(grep -c "CTS-expected timer" "$logfile" 2>/dev/null || true)
        CTS_TIMER_COUNT=$((CTS_TIMER_COUNT + COUNT))
    fi
done

echo ""
echo "  NAV reset statistics:"
echo "    CTS-expected timers armed:     $CTS_TIMER_COUNT"
echo "    NAV RESET (CTS not received):  $NAV_RESET_CTS_COUNT"
echo "    NAV RESET (DATA not received): $NAV_RESET_DATA_COUNT"
echo "    Total NAV RESET events:        $NAV_RESET_COUNT"
echo ""

check "NAV RESET events fired (count: $NAV_RESET_COUNT)" "$([ "$NAV_RESET_COUNT" -gt 0 ] && echo 1 || echo 0)"
echo ""

# ============================================================
# Scenario 2: Two BSSs — mutual interference
# ============================================================
echo "[Step 3] Scenario 2: Two BSSs (A at 0m, B at 6m)"
echo "  Both on same channel with RTS/CTS; close enough to collide."
echo "  Each overhears the other's failed RTS → NAV reset expected."
echo ""

SIM_CODE="nav_reset_s2"
OUT2="$OUTPUT_DIR/out_nav_reset_2.txt"
rm -f "$OUT2"
rm -f "$OUTPUT_DIR/logs_output_${SIM_CODE}"_*.txt

"$BIN" --nodes "$SCRIPT_DIR/input_nodes_nav_reset_2.csv" \
       --out "$OUT2" \
       --code "$SIM_CODE" \
       --logs-node 0 --save-node 1 \
       --time "$SIM_TIME" --seed "$SEED" \
       > "$OUTPUT_DIR/console_${SIM_CODE}.txt" 2>&1

echo "  Simulator completed. Checking results..."

if [ -f "$OUT2" ]; then
    TPT_LINE=$(head -1 "$OUT2")
    TPT_A=$(echo "$TPT_LINE" | cut -d';' -f2 | cut -d'{' -f2 | cut -d'}' -f1 | cut -d',' -f1)
    TPT_B=$(echo "$TPT_LINE" | cut -d';' -f2 | cut -d'{' -f2 | cut -d'}' -f1 | cut -d',' -f2)

    A_OK=$(echo "$TPT_A > 0" | bc)
    B_OK=$(echo "$TPT_B > 0" | bc)

    check "BSS_A throughput > 0 (actual: ${TPT_A} Mbps)" "$A_OK"
    check "BSS_B throughput > 0 (actual: ${TPT_B} Mbps)" "$B_OK"
else
    check "Output file exists" "0"
fi

NAV_RESET_COUNT=0
NAV_RESET_CTS_COUNT=0
NAV_RESET_DATA_COUNT=0
CTS_TIMER_COUNT=0
for logfile in "$OUTPUT_DIR/logs_output_${SIM_CODE}"_*.txt; do
    if [ -f "$logfile" ]; then
        COUNT=$(grep -c "NAV RESET" "$logfile" 2>/dev/null || true)
        NAV_RESET_COUNT=$((NAV_RESET_COUNT + COUNT))
        COUNT=$(grep -c "NAV RESET: CTS not received" "$logfile" 2>/dev/null || true)
        NAV_RESET_CTS_COUNT=$((NAV_RESET_CTS_COUNT + COUNT))
        COUNT=$(grep -c "NAV RESET: DATA not received" "$logfile" 2>/dev/null || true)
        NAV_RESET_DATA_COUNT=$((NAV_RESET_DATA_COUNT + COUNT))
        COUNT=$(grep -c "CTS-expected timer" "$logfile" 2>/dev/null || true)
        CTS_TIMER_COUNT=$((CTS_TIMER_COUNT + COUNT))
    fi
done

echo ""
echo "  NAV reset statistics:"
echo "    CTS-expected timers armed:     $CTS_TIMER_COUNT"
echo "    NAV RESET (CTS not received):  $NAV_RESET_CTS_COUNT"
echo "    NAV RESET (DATA not received): $NAV_RESET_DATA_COUNT"
echo "    Total NAV RESET events:        $NAV_RESET_COUNT"
echo ""

check "NAV RESET events fired (count: $NAV_RESET_COUNT)" "$([ "$NAV_RESET_COUNT" -gt 0 ] && echo 1 || echo 0)"
echo ""

# ============================================================
# Summary
# ============================================================
TOTAL=$((PASS + FAIL))
echo "============================================="
echo " Summary: $PASS/$TOTAL passed, $FAIL failed"
echo "============================================="
if [ "$FAIL" -eq 0 ]; then
    echo " ALL TESTS PASSED"
    echo ""
    echo " To inspect NAV RESET events in detail:"
    echo "   grep 'NAV RESET' $OUTPUT_DIR/logs_output_nav_reset_s1_*.txt"
    echo "   grep 'NAV RESET' $OUTPUT_DIR/logs_output_nav_reset_s2_*.txt"
    exit 0
else
    echo " SOME TESTS FAILED — see details above"
    exit 1
fi
