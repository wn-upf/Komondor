#!/bin/bash
# Valgrind analysis script for Komondor double-free detection

echo "=========================================="
echo "Valgrind Double-Free Detection"
echo "=========================================="
echo ""

# Check if valgrind is installed
if ! command -v valgrind &> /dev/null; then
    echo "ERROR: Valgrind is not installed."
    echo ""
    echo "Install it with:"
    echo "  sudo apt-get install valgrind    # Ubuntu/Debian"
    echo "  sudo yum install valgrind        # RHEL/CentOS"
    echo "  sudo dnf install valgrind        # Fedora"
    exit 1
fi

cd "$(dirname "$0")"

echo "Running Valgrind (this will be slow)..."
echo "Output will be saved to: valgrind_output.txt"
echo ""

# Run with timeout to prevent hanging
timeout 120 valgrind \
    --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file=valgrind_output.txt \
    ./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 1 1 1 1 777

VALGRIND_EXIT=$?

echo ""
echo "=========================================="
echo "Analysis Complete"
echo "=========================================="
echo ""

if [ -f valgrind_output.txt ]; then
    echo "=== DOUBLE-FREE / INVALID FREE ERRORS ==="
    grep -A 15 "Invalid free\|double free" valgrind_output.txt || echo "No double-free errors found in output"
    
    echo ""
    echo "=== ERROR SUMMARY ==="
    grep "ERROR SUMMARY" valgrind_output.txt || echo "No error summary found"
    
    echo ""
    echo "=== LEAK SUMMARY ==="
    grep "LEAK SUMMARY" valgrind_output.txt || echo "No leak summary found"
    
    echo ""
    echo "Full report: valgrind_output.txt"
    echo ""
    echo "To see full details, run:"
    echo "  less valgrind_output.txt"
    echo "  or"
    echo "  grep -A 20 'Invalid free' valgrind_output.txt"
else
    echo "ERROR: Valgrind output file not created"
    echo "Exit code: $VALGRIND_EXIT"
fi
