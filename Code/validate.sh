#!/usr/bin/env bash
# validate.sh
#
# Run from the Code/ directory on Linux:
#   cd Code && bash validate.sh
#
# Covers:
#   Part 1: Basic scenarios (1a/1b, aggregation on/off)
#   Part 2: Complex scenarios (2a–2d, aggregation on/off)
#   Part 3: Channel-bonding scenarios (20/40/80/160 MHz)
#   Part 4: Spatial-reuse scenarios (SR on/off)
#
# Expected values come from the pre-refactor baseline captured in
# input/script_regression_validation_scenarios.sh.

set -euo pipefail

# ============================================================
# Paths
# ============================================================
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MAIN_DIR="$SCRIPT_DIR/main"
INPUT_DIR="$SCRIPT_DIR/input/validation"
OUTPUT_DIR="$SCRIPT_DIR/output"
BIN="$MAIN_DIR/komondor_main"

# ============================================================
# Simulation parameters
# ============================================================
SIM_TIME=100
SEED=1

# Tolerances
ALLOWED_ERROR_TPT=1       # Mbps — throughput pass/fail band
ALLOWED_ERROR_RSSI=0.1    # dBm  — RSSI pass/fail band

# ============================================================
# Expected values (pre-refactor baseline)
# ============================================================
# Basic scenarios: WLAN-A throughput (Mbps)
# Indexes: 0=1a_no_agg  1=1a_agg  2=1b_no_agg  3=1b_agg
values_basic=(22.80 88.25 22.80 88.25)
labels_basic=("1a_no_agg" "1a_agg" "1b_no_agg" "1b_agg")

# Complex scenarios: WLAN-A/B/C throughput (Mbps), 8 rows × 3 cols
# Row order: 2a_no_agg  2a_agg  2b_no_agg  2b_agg  2c_no_agg  2c_agg  2d_no_agg  2d_agg
declare -A values_complex
values_complex[0,0]=10.06;  values_complex[0,1]=10.05;  values_complex[0,2]=10.04
values_complex[1,0]=49.95;  values_complex[1,1]=49.45;  values_complex[1,2]=49.64
values_complex[2,0]=21.69;  values_complex[2,1]=3.50;   values_complex[2,2]=21.69
values_complex[3,0]=131.04; values_complex[3,1]=1.19;   values_complex[3,2]=131.05
values_complex[4,0]=24.31;  values_complex[4,1]=18.46;  values_complex[4,2]=24.30
values_complex[5,0]=132.08; values_complex[5,1]=65.02;  values_complex[5,2]=132.10
values_complex[6,0]=24.31;  values_complex[6,1]=24.30;  values_complex[6,2]=24.31
values_complex[7,0]=132.09; values_complex[7,1]=132.09; values_complex[7,2]=132.09
labels_complex=("2a_no_agg" "2a_agg" "2b_no_agg" "2b_agg" "2c_no_agg" "2c_agg" "2d_no_agg" "2d_agg")

# Channel-bonding scenarios: WLAN-A RSSI (dBm)
# Indexes: 0=20MHz  1=40MHz  2=80MHz  3=160MHz
values_cb=(-108.23 -111.24 -114.25 -117.26)
labels_cb=("20MHz" "40MHz" "80MHz" "160MHz")

# Spatial-reuse scenarios: WLAN-A/B throughput (Mbps), 2 rows
declare -A values_sr
values_sr[0,0]=30.65; values_sr[0,1]=30.62
values_sr[1,0]=21.131; values_sr[1,1]=21.101
labels_sr=("4a_sr" "4b_no_sr")

# ============================================================
# Helpers
# ============================================================
PASS=0
FAIL=0

check_tpt() {
    local label="$1"
    local actual="$2"
    local expected="$3"
    local lo hi
    lo=$(echo "$expected - $ALLOWED_ERROR_TPT" | bc)
    hi=$(echo "$expected + $ALLOWED_ERROR_TPT" | bc)
    local ok_lo ok_hi
    ok_lo=$(echo "$actual >= $lo" | bc)
    ok_hi=$(echo "$actual <= $hi" | bc)
    if [ "$ok_lo" = "1" ] && [ "$ok_hi" = "1" ]; then
        echo "  [PASS] $label  actual=$actual  expected=$expected±${ALLOWED_ERROR_TPT}"
        PASS=$((PASS + 1))
    else
        echo "  [FAIL] $label  actual=$actual  expected=$expected±${ALLOWED_ERROR_TPT}"
        FAIL=$((FAIL + 1))
    fi
}

check_rssi() {
    local label="$1"
    local actual="$2"
    local expected="$3"
    local lo hi
    lo=$(echo "$expected - $ALLOWED_ERROR_RSSI" | bc)
    hi=$(echo "$expected + $ALLOWED_ERROR_RSSI" | bc)
    local ok_lo ok_hi
    ok_lo=$(echo "$actual >= $lo" | bc)
    ok_hi=$(echo "$actual <= $hi" | bc)
    if [ "$ok_lo" = "1" ] && [ "$ok_hi" = "1" ]; then
        echo "  [PASS] $label  actual=$actual  expected=$expected±${ALLOWED_ERROR_RSSI}"
        PASS=$((PASS + 1))
    else
        echo "  [FAIL] $label  actual=$actual  expected=$expected±${ALLOWED_ERROR_RSSI}"
        FAIL=$((FAIL + 1))
    fi
}

run_sim() {
    local nodes_file="$1"
    local out_file="$2"
    local extra="${3:-}"
    "$BIN" --nodes "$nodes_file" \
           --out "$out_file" \
           --code "sim_$(date +%s%N).csv" \
           --logs-sys 1 --logs-node 1 --save-node 0 \
           --time "$SIM_TIME" --seed "$SEED" \
           $extra >> "$OUTPUT_DIR/logs_console.txt" 2>&1
}

parse_tpt_list() {
    # Extract {A,B,C,...} throughput list from an output file line
    local line="$1"
    echo "$line" | cut -d';' -f2 | cut -d'{' -f2 | cut -d'}' -f1
}

parse_rssi_list() {
    # Extract RSSI field (column 3) from an output file line
    local line="$1"
    echo "$line" | cut -d';' -f3 | cut -d'{' -f2 | cut -d'}' -f1
}

# ============================================================
# STEP 1: Build
# ============================================================
echo "============================================="
echo " Build & Regression Validation"
echo "============================================="
echo ""
echo "[Step 1] Building with: make ($(cd "$MAIN_DIR" && make --version | head -1))"
echo ""

cd "$MAIN_DIR"
BUILD_LOG=$(mktemp)
if make 2>&1 | tee "$BUILD_LOG"; then
    echo ""
    WARN_COUNT=$(grep -c "warning:" "$BUILD_LOG" || true)
    if [ "$WARN_COUNT" -eq 0 ]; then
        echo "  [PASS] Build: zero warnings"
        PASS=$((PASS + 1))
    else
        echo "  [WARN] Build: $WARN_COUNT warning(s) detected"
        grep "warning:" "$BUILD_LOG" | head -20
        FAIL=$((FAIL + 1))
    fi
else
    echo "  [FAIL] Build failed — aborting"
    exit 1
fi
rm -f "$BUILD_LOG"
echo ""

# ============================================================
# STEP 2: Prepare output directory
# ============================================================
mkdir -p "$OUTPUT_DIR"
> "$OUTPUT_DIR/logs_console.txt"    # reset console log

# ============================================================
# STEP 3: Run scenarios
# ============================================================

# --- Part 1: Basic scenarios ---
echo "[Step 2] Running scenarios..."
echo ""
OUT_BASIC="$OUTPUT_DIR/out_basic.txt"
rm -f "$OUT_BASIC"
for f in "$INPUT_DIR/basic_scenarios/"*.csv; do
    run_sim "$f" "$OUT_BASIC"
done

# --- Part 2: Complex scenarios ---
OUT_COMPLEX="$OUTPUT_DIR/out_complex.txt"
rm -f "$OUT_COMPLEX"
for f in "$INPUT_DIR/complex_scenarios/"*.csv; do
    run_sim "$f" "$OUT_COMPLEX"
done

# --- Part 3: Channel-bonding scenarios ---
OUT_CB="$OUTPUT_DIR/out_channel_bonding.txt"
rm -f "$OUT_CB"
for f in "$INPUT_DIR/channel_bonding_scenarios/"*.csv; do
    run_sim "$f" "$OUT_CB"
done

# --- Part 4: Spatial-reuse scenarios ---
OUT_SR="$OUTPUT_DIR/out_spatial_reuse.txt"
rm -f "$OUT_SR"
for f in "$INPUT_DIR/spatial_reuse_scenarios/"*.csv; do
    run_sim "$f" "$OUT_SR"
done

# ============================================================
# STEP 4: Validate
# ============================================================
echo "[Step 3] Validation results"
echo ""

# --- Part 1: Basic ---
echo "--- Part 1: Basic scenarios ---"
ix=0
while IFS= read -r line; do
    tpt=$(parse_tpt_list "$line")
    check_tpt "${labels_basic[$ix]}" "$tpt" "${values_basic[$ix]}"
    ix=$((ix + 1))
done < "$OUT_BASIC"
echo ""

# --- Part 2: Complex ---
echo "--- Part 2: Complex scenarios ---"
ix=0
while IFS= read -r line; do
    tpt_list=$(parse_tpt_list "$line")
    tpt_a=$(echo "$tpt_list" | cut -d',' -f1)
    tpt_b=$(echo "$tpt_list" | cut -d',' -f2)
    tpt_c=$(echo "$tpt_list" | cut -d',' -f3)
    check_tpt "${labels_complex[$ix]}_A" "$tpt_a" "${values_complex[$ix,0]}"
    check_tpt "${labels_complex[$ix]}_B" "$tpt_b" "${values_complex[$ix,1]}"
    check_tpt "${labels_complex[$ix]}_C" "$tpt_c" "${values_complex[$ix,2]}"
    ix=$((ix + 1))
done < "$OUT_COMPLEX"
echo ""

# --- Part 3: Channel-bonding ---
echo "--- Part 3: Channel-bonding scenarios ---"
ix=0
while IFS= read -r line; do
    rssi=$(parse_rssi_list "$line" | cut -d',' -f1)
    check_rssi "${labels_cb[$ix]}" "$rssi" "${values_cb[$ix]}"
    ix=$((ix + 1))
done < "$OUT_CB"
echo ""

# --- Part 4: Spatial-reuse ---
echo "--- Part 4: Spatial-reuse scenarios ---"
ix=0
while IFS= read -r line; do
    tpt_list=$(parse_tpt_list "$line")
    tpt_a=$(echo "$tpt_list" | cut -d',' -f1)
    tpt_b=$(echo "$tpt_list" | cut -d',' -f2)
    check_tpt "${labels_sr[$ix]}_A" "$tpt_a" "${values_sr[$ix,0]}"
    check_tpt "${labels_sr[$ix]}_B" "$tpt_b" "${values_sr[$ix,1]}"
    ix=$((ix + 1))
done < "$OUT_SR"
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
    exit 0
else
    echo " SOME TESTS FAILED — see details above"
    exit 1
fi
