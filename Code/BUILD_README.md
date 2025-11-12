# Komondor Build Guide

## Quick Build

Use the comprehensive build script that handles everything automatically:

```bash
cd Code
./build.sh
```

This script will:
1. ✅ Check if the CompC++ binary exists and is for the correct architecture
2. ✅ Rebuild CompC++ automatically if needed (checks for dependencies)
3. ✅ Build Komondor simulator
4. ✅ Provide clear error messages and instructions if anything fails

## Manual Build

If you prefer to build manually or the automated script doesn't work:

### Step 1: Build CompC++ Compiler

```bash
cd Code/compcpp
make clean
make
cp cxx ../COST/cxx
```

**Requirements:**
- `flex` - Lexical analyzer generator
- `g++` - C++ compiler
- `make` - Build tool

Install dependencies:
```bash
sudo apt-get install flex g++ make
```

### Step 2: Build Komondor

```bash
cd Code/main
./build_local
```

Or use the Makefile:
```bash
cd Code/main
make
```

## Architecture Issues

If you see errors like:
```
cannot execute binary file: Exec format error
```

This means the CompC++ binary is for a different architecture (e.g., macOS binary on Linux). The `build.sh` script will automatically detect this and rebuild CompC++ for your platform.

## Troubleshooting

### Missing Dependencies

If the build fails due to missing dependencies:
```bash
sudo apt-get update
sudo apt-get install flex g++ make
```

### Build Script Not Executable

```bash
chmod +x Code/build.sh
```

### Clean Build

To start fresh:
```bash
cd Code/compcpp
make clean

cd ../main
make clean  # or remove komondor_main and komondor_main.cxx manually
```

Then run `./build.sh` again.

## Output

After successful build:
- CompC++ binary: `Code/COST/cxx`
- Komondor binary: `Code/main/komondor_main`

## Running Komondor

```bash
cd Code/main
./komondor_main INPUT_FILE OUTPUT_FILE FLAGS... SIM_TIME SEED
```

See the main README.md for detailed usage instructions.

