#!/usr/bin/env bash
# multiple_inputs_script_agents.sh
#
# Runs komondor_main with an agents file on every input_nodes*.csv found
# in a given input folder.  Results are appended to a single output file.
#
# Run from Code/scripts_multiple_executions/ on Linux:
#   bash multiple_inputs_script_agents.sh
#
# Override paths via env vars:
#   NODES_DIR=../input/my_nodes AGENTS_FILE=../input/my_agents.csv \
#     bash multiple_inputs_script_agents.sh

set -euo pipefail

# --- parameters ---
SIM_TIME=100
SEED=1
NODES_DIR="${NODES_DIR:-../input/examples/mab_example}"
AGENTS_FILE="${AGENTS_FILE:-../input/examples/mab_example/agents_egreedy.csv}"
OUTPUT_FILE="../output/script_output_agents.txt"
LOG_FILE="../output/logs_console_agents.txt"

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
echo "Agents:   $AGENTS_FILE"
mapfile -t FILES < <(ls "$NODES_DIR"/input_nodes*.csv 2>/dev/null)

if [ ${#FILES[@]} -eq 0 ]; then
    echo "ERROR: No input_nodes*.csv files found in $NODES_DIR"
    exit 1
fi

echo "Detected ${#FILES[@]} node file(s):"
for f in "${FILES[@]}"; do
    echo "  - $(basename "$f")"
done
echo ""

# --- run simulations ---
echo "EXECUTING KOMONDOR SIMULATIONS (with agents)..."
total=${#FILES[@]}
ix=0
for nodes_file in "${FILES[@]}"; do
    ix=$((ix + 1))
    name="$(basename "$nodes_file" .csv)"
    echo "===================================================================================="
    echo "[$ix/$total] $name"
    "$BIN" \
        --nodes   "$nodes_file" \
        --agents  "$AGENTS_FILE" \
        --out     "$OUTPUT_FILE" \
        --code    "sim_${name}" \
        --time    "$SIM_TIME" \
        --seed    "$SEED" \
        --logs-sys 0 --logs-node 0 --save-node 0 --save-agent 0 \
        >> "$LOG_FILE" 2>&1
    echo "===================================================================================="
    echo ""
done

echo "DONE -- output: $OUTPUT_FILE"
