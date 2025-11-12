# Running Komondor Examples

## Quick Start

### Basic Example (No Agents)

Run from the `Code/main` directory:

```bash
cd Code/main
./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 1 1 1 1 777
```

### Command Format

```bash
./komondor_main INPUT_FILE OUTPUT_FILE SIMULATION_CODE SAVE_NODE_LOGS PRINT_SYSTEM_LOGS PRINT_NODE_LOGS SIM_TIME SEED
```

### Parameters

1. **INPUT_FILE**: Path to the nodes configuration CSV file
   - Example: `../input/input_example/input_nodes.csv`
   - Must use semicolons (`;`) as separators

2. **OUTPUT_FILE**: Path to the output file for simulation results
   - Example: `../output/out.txt`
   - File will be created if it doesn't exist

3. **SIMULATION_CODE**: String identifier for the simulation
   - Example: `TEST`, `sim_1`, `experiment_1`
   - Used for logging and file naming

4. **SAVE_NODE_LOGS**: Save detailed node logs to separate files (0 or 1)
   - `1` = Save logs (slower, more detailed)
   - `0` = Don't save logs (faster)
   - Note: Enabling this increases execution time

5. **PRINT_SYSTEM_LOGS**: Print system logs to console (0 or 1)
   - `1` = Print system logs
   - `0` = Don't print system logs

6. **PRINT_NODE_LOGS**: Print node logs to console (0 or 1)
   - `1` = Print node logs
   - `0` = Don't print node logs

7. **SIM_TIME**: Simulation time duration in seconds
   - Example: `1` = 1 second, `100` = 100 seconds
   - **Note**: Very short simulations (1 second) may not produce meaningful results

8. **SEED**: Random seed for simulation reproducibility
   - Example: `777`, `12345`
   - Same seed = same random sequence = reproducible results

### Example Commands

#### Quick Test (1 second, minimal logging)
```bash
cd Code/main
./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 0 0 0 1 777
```

#### Full Logging (1 second, all logs enabled)
```bash
cd Code/main
./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 1 1 1 1 777
```

#### Longer Simulation (100 seconds, minimal logging)
```bash
cd Code/main
./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 0 0 0 100 777
```

#### Production Run (100 seconds, save logs, print system logs only)
```bash
cd Code/main
./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 1 1 0 100 777
```

### With Agents (Advanced)

If you have an agents configuration file:

```bash
./komondor_main INPUT_FILE_NODES INPUT_FILE_AGENTS OUTPUT_FILE SIMULATION_CODE SAVE_NODE_LOGS SAVE_AGENT_LOGS PRINT_SYSTEM_LOGS PRINT_NODE_LOGS PRINT_AGENT_LOGS SIM_TIME SEED
```

Example:
```bash
./komondor_main ../input/input_example/input_nodes.csv ../input/input_example/agents.csv ../output/out.txt TEST 1 1 1 1 1 100 777
```

### Input Files

Input files are located in `Code/input/`:
- **Basic example**: `Code/input/input_example/input_nodes.csv`
- **Spatial reuse example**: `Code/input/input_example/input_nodes_spatial_reuse.csv`
- **Agents example**: `Code/input/input_example/agents.csv`
- **Validation scenarios**: `Code/input/validation/`

### Output Files

Output files are written to `Code/output/`:
- Main output: Specified in the command (e.g., `out.txt`)
- Node logs (if enabled): `logs_output__N{node_id}_{node_code}.txt`
- Console logs: Redirected if using scripts

### Tips

1. **Simulation Time**: Start with short simulations (1-10 seconds) for testing, then increase for production runs
2. **Logging**: Disable detailed logging (`SAVE_NODE_LOGS=0`) for faster execution during development
3. **Reproducibility**: Use the same seed to reproduce results
4. **Performance**: Longer simulations with logging enabled can take significant time
5. **Path**: Always run from `Code/main/` directory, or adjust paths accordingly

### Troubleshooting

#### Binary Not Found
```bash
# Build Komondor first
cd Code
./build.sh
```

#### Input File Not Found
```bash
# Check if file exists
ls -la ../input/input_example/input_nodes.csv

# Verify you're in the correct directory
pwd  # Should be: .../Komondor/Code/main
```

#### Output Directory Not Found
The output directory will be created automatically if it doesn't exist.

#### Permission Denied
```bash
# Make sure binary is executable
chmod +x komondor_main
```

### Validation Scripts

To run validation scenarios:

```bash
cd Code/input
./script_regression_validation_scenarios.sh
```

This will run multiple validation scenarios and generate comprehensive results.

