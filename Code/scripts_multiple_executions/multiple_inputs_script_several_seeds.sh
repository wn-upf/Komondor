#!/usr/bin/env bash
# multiple_inputs_script_several_seeds.sh
#
# Runs komondor_main on every input_nodes*.csv found in a given folder,
# each with N_SEEDS different random seeds.
# Results are appended to a single output file.
#
# Run from Code/scripts_multiple_executions/ on Linux:
#   bash multiple_inputs_script_several_seeds.sh
#
# Override parameters via env vars:
#   NODES_DIR=../input/my_scenarios N_SEEDS=5 bash multiple_inputs_script_several_seeds.sh

set -euo pipefail

# --- parameters ---
SIM_TIME=10
N_SEEDS=5
NODES_DIR="${NODES_DIR:-../input/examples/basic_example}"
OUTPUT_FILE="../output/script_output_seeds.txt"
LOG_FILE="../output/logs_console_seeds.txt"

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

echo "Detected ${#FILES[@]} node file(s), $N_SEEDS seed(s) each:"
for f in "${FILES[@]}"; do
    echo "  - $(basename "$f")"
done
echo ""

# --- run simulations ---
echo "EXECUTING KOMONDOR SIMULATIONS (multiple seeds)..."
total=$(( ${#FILES[@]} * N_SEEDS ))
run=0
for nodes_file in "${FILES[@]}"; do
    name="$(basename "$nodes_file" .csv)"
    for seed_ix in $(seq 1 "$N_SEEDS"); do
        SEED=$RANDOM
        run=$((run + 1))
        echo "===================================================================================="
        echo "[$run/$total] $name  seed=$SEED"
        "$BIN" \
            --nodes  "$nodes_file" \
            --out    "$OUTPUT_FILE" \
            --code   "sim_${name}_s${SEED}" \
            --time   "$SIM_TIME" \
            --seed   "$SEED" \
            --logs-sys 0 --logs-node 0 --save-node 0 \
            >> "$LOG_FILE" 2>&1
        echo "===================================================================================="
        echo ""
    done
done

echo "DONE -- output: $OUTPUT_FILE"
