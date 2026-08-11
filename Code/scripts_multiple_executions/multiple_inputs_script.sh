#!/usr/bin/env bash
# multiple_inputs_script.sh
#
# Runs komondor_main on every input_nodes*.csv found in a given input folder.
# Results are appended to a single output file.
#
# Run from Code/scripts_multiple_executions/ on Linux:
#   bash multiple_inputs_script.sh
#
# Override the input folder:
#   NODES_DIR=../input/my_scenarios bash multiple_inputs_script.sh

set -euo pipefail

# --- parameters ---
SIM_TIME=10
SEED=1
NODES_DIR="${NODES_DIR:-../input/examples/basic_example}"
OUTPUT_FILE="../output/script_output.txt"
LOG_FILE="../output/logs_console.txt"

BIN="../main/komondor_main"

# --- build ---
echo "Building Komondor..."
make -C ../main --no-print-directory
echo ""

# --- prepare output dir ---
mkdir -p ../output
> "$OUTPUT_FILE"
> "$LOG_FILE"

# --- collect input files (node files only) ---
echo "Scanning: $NODES_DIR"
mapfile -t FILES < <(ls "$NODES_DIR"/input_nodes*.csv 2>/dev/null)

if [ ${#FILES[@]} -eq 0 ]; then
    echo "ERROR: No input_nodes*.csv files found in $NODES_DIR"
    exit 1
fi

echo "Detected ${#FILES[@]} input file(s):"
for f in "${FILES[@]}"; do
    echo "  - $(basename "$f")"
done
echo ""

# --- run simulations ---
echo "EXECUTING KOMONDOR SIMULATIONS..."
total=${#FILES[@]}
ix=0
for nodes_file in "${FILES[@]}"; do
    ix=$((ix + 1))
    name="$(basename "$nodes_file" .csv)"
    echo "===================================================================================="
    echo "[$ix/$total] $name"
    "$BIN" \
        --nodes  "$nodes_file" \
        --out    "$OUTPUT_FILE" \
        --code   "sim_${name}" \
        --time   "$SIM_TIME" \
        --seed   "$SEED" \
        --logs-sys 0 --logs-node 0 --save-node 0 \
        >> "$LOG_FILE" 2>&1
    echo "===================================================================================="
    echo ""
done

echo "DONE -- output: $OUTPUT_FILE"
