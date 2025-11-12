#!/bin/bash
# Comprehensive build script for Komondor simulator
# This script automatically:
# 1. Checks CompC++ binary architecture
# 2. Rebuilds CompC++ if needed
# 3. Builds Komondor

# Note: We don't use 'set -e' because we want to handle errors gracefully

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CODE_DIR="$SCRIPT_DIR"
COST_BINARY="$CODE_DIR/COST/cxx"
COMPCPP_DIR="$CODE_DIR/compcpp"
MAIN_DIR="$CODE_DIR/main"

# Function to print colored messages
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check if CompC++ binary is valid for this platform
check_compcpp_binary() {
    if [ ! -f "$COST_BINARY" ]; then
        return 1  # Binary doesn't exist
    fi
    
    # Check if binary is executable
    if [ ! -x "$COST_BINARY" ]; then
        return 1  # Not executable
    fi
    
    # Check architecture using file command (most reliable method)
    if command_exists file; then
        FILE_TYPE=$(file "$COST_BINARY" 2>/dev/null || echo "unknown")
        
        # Check if it's a Mach-O binary (macOS) - definitely wrong for Linux
        if echo "$FILE_TYPE" | grep -qi "Mach-O"; then
            return 1  # Wrong architecture
        fi
        
        # If it's an ELF binary (Linux), assume it's correct
        # We'll catch execution errors during actual use if there's still an issue
        if echo "$FILE_TYPE" | grep -qi "ELF"; then
            return 0  # Likely valid for Linux
        fi
    fi
    
    # If we can't determine file type, assume it might be valid
    # We'll catch errors during actual compilation if it's not
    return 0
}

# Function to check build dependencies
check_dependencies() {
    local missing_deps=()
    
    if ! command_exists flex; then
        missing_deps+=("flex")
    fi
    
    if ! command_exists g++; then
        missing_deps+=("g++")
    fi
    
    if ! command_exists make; then
        missing_deps+=("make")
    fi
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        echo "Install them with: sudo apt-get install ${missing_deps[*]}"
        return 1
    fi
    
    return 0
}

# Function to build CompC++
build_compcpp() {
    print_info "Building CompC++ compiler..."
    
    # Check dependencies
    if ! check_dependencies; then
        print_error "Cannot build CompC++: missing dependencies"
        return 1
    fi
    
    # Navigate to compcpp directory
    cd "$COMPCPP_DIR" || {
        print_error "Cannot access compcpp directory: $COMPCPP_DIR"
        return 1
    }
    
    # Clean previous build
    print_info "Cleaning previous CompC++ build..."
    make clean >/dev/null 2>&1 || true
    
    # Build CompC++
    print_info "Compiling CompC++..."
    if ! make; then
        print_error "Failed to build CompC++"
        return 1
    fi
    
    # Check if binary was created
    if [ ! -f "./cxx" ]; then
        print_error "CompC++ binary was not created"
        return 1
    fi
    
    # Copy binary to COST directory
    print_info "Installing CompC++ binary to COST directory..."
    mkdir -p "$(dirname "$COST_BINARY")"
    cp ./cxx "$COST_BINARY"
    chmod +x "$COST_BINARY"
    
    print_info "CompC++ compiler built successfully"
    return 0
}

# Function to build Komondor
build_komondor() {
    print_info "Building Komondor simulator..."
    
    # Navigate to main directory
    cd "$MAIN_DIR" || {
        print_error "Cannot access main directory: $MAIN_DIR"
        return 1
    }
    
    # Define paths
    COST_WRAPPER=".././COST/cxx"
    SRC_CC="komondor_main.cc"
    SRC_CXX="komondor_main.cxx"
    BIN="komondor_main"
    
    # Verify CompC++ binary exists and is executable
    if [ ! -f "$COST_WRAPPER" ] || [ ! -x "$COST_WRAPPER" ]; then
        print_error "CompC++ compiler not found or not executable: $COST_WRAPPER"
        return 1
    fi
    
    # Generate C++ code from CompC++ source
    print_info "Generating C++ code from $SRC_CC..."
    if ! "$COST_WRAPPER" "$SRC_CC"; then
        print_error "Failed to generate $SRC_CXX"
        return 1
    fi
    
    # Check if generation was successful
    if [ ! -f "$SRC_CXX" ]; then
        print_error "Generated file $SRC_CXX not found"
        return 1
    fi
    
    # Compile the generated C++ code
    print_info "Compiling generated C++ code..."
    if ! g++ -Wall -Werror -g -o "$BIN" "$SRC_CXX"; then
        print_error "Failed to compile Komondor"
        return 1
    fi
    
    # Verify binary was created
    if [ ! -f "$BIN" ] || [ ! -x "$BIN" ]; then
        print_error "Komondor binary was not created or is not executable"
        return 1
    fi
    
    print_info "Komondor built successfully: $MAIN_DIR/$BIN"
    return 0
}

# Main build process
main() {
    print_info "Starting Komondor build process..."
    print_info "Code directory: $CODE_DIR"
    
    # Step 1: Check if CompC++ binary is valid
    print_info "Checking CompC++ binary..."
    if check_compcpp_binary; then
        print_info "CompC++ binary is valid for this platform"
    else
        print_warn "CompC++ binary is missing or invalid, rebuilding..."
        if ! build_compcpp; then
            print_error "Failed to build CompC++ compiler"
            exit 1
        fi
    fi
    
    # Step 2: Build Komondor
    if ! build_komondor; then
        print_error "Failed to build Komondor"
        exit 1
    fi
    
    print_info "Build process completed successfully!"
    print_info "Komondor binary: $MAIN_DIR/komondor_main"
}

# Run main function
main "$@"

