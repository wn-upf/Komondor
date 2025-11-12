# Build Error Fix: CompC++ Binary Architecture Mismatch

## Problem

The `build_local` script fails with:
```
./build_local: line 1: .././COST/cxx: cannot execute binary file: Exec format error
cc1plus: fatal error: komondor_main.cxx: No such file or directory
```

**Root Cause:** The `COST/cxx` binary is compiled for macOS ARM64, but your system is Linux x86_64.

## Solution

Rebuild the CompC++ compiler from source for your platform.

### Step 1: Install Dependencies

```bash
sudo apt-get update
sudo apt-get install flex g++ make
```

### Step 2: Build CompC++ Compiler

```bash
cd Code/compcpp
make clean
make
```

This will create a `cxx` binary in the `compcpp` directory.

### Step 3: Copy Binary to COST Directory

```bash
cp compcpp/cxx COST/cxx
```

### Step 4: Build Komondor

```bash
cd Code/main
./build_local
```

Or use the Makefile:

```bash
cd Code/main
make
```

## Alternative: Use Makefile (Recommended)

The Makefile in `Code/main/` properly handles the build process with dependencies:

```bash
cd Code/main
make
```

This will:
1. Check if `komondor_main.cxx` exists
2. If not, run the CompC++ compiler to generate it
3. Compile the generated C++ code

## Verification

After building, verify the binary architecture:

```bash
file Code/COST/cxx
```

Should show: `ELF 64-bit LSB executable, x86-64` (or similar for Linux)

## Notes

- The `cxx` binary is platform-specific and must be rebuilt for each architecture
- The `build_local` script assumes the binary already exists and is executable
- Using `make` is more robust as it handles dependencies automatically

