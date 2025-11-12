# Valgrind Guide for Double-Free Detection

## Quick Start

### Option 1: Use the provided script (Recommended)
```bash
cd /home/sbarrachina/Komondor/Code/main
./run_valgrind.sh
```

### Option 2: Manual command
```bash
cd /home/sbarrachina/Komondor/Code/main
valgrind --tool=memcheck \
         --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind_output.txt \
         ./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 1 1 1 1 777
```

### Install Valgrind (if not installed)
```bash
# Ubuntu/Debian
sudo apt-get install valgrind

# RHEL/CentOS
sudo yum install valgrind

# Fedora
sudo dnf install valgrind
```

## Simplified Command (Faster, Less Verbose)

```bash
cd /home/sbarrachina/Komondor/Code/main
valgrind --leak-check=full --show-leak-kinds=all ./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 1 1 1 1 777 2>&1 | tee valgrind_output.txt
```

## What to Look For

### 1. Double-Free Error
Look for lines like:
```
==12345== Invalid free() / delete / delete[] / realloc()
==12345==    at 0x4C2EDEB: free (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==12345==    by 0x...: Node::~Node() (node.h:XXXX)
==12345==    by 0x...: ...
==12345==  Address 0x... is 0 bytes inside a block of size X free'd
==12345==    at 0x4C2EDEB: free (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==12345==    by 0x...: NodeChannelManager::~NodeChannelManager() (node_channel_manager.h:XXX)
```

### 2. Key Information
- **Address**: The memory address being double-freed
- **Stack trace**: Shows which destructor/function is freeing the memory
- **Block size**: Size of the array being freed
- **First free location**: Where it was first freed
- **Second free location**: Where it's being freed again (the error)

## Focused Command (Only Double-Free)

```bash
cd /home/sbarrachina/Komondor/Code/main
valgrind --tool=memcheck \
         --leak-check=no \
         ./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 1 1 1 1 777 2>&1 | grep -A 20 "Invalid free\|double free\|ERROR SUMMARY"
```

## Command Options Explained

- `--tool=memcheck`: Use the memory error detector (default)
- `--leak-check=full`: Show detailed leak information
- `--show-leak-kinds=all`: Show all types of leaks
- `--track-origins=yes`: Track origins of uninitialized values (slower but more detailed)
- `--verbose`: More detailed output
- `--log-file=filename`: Save output to file instead of stderr

## Quick Analysis Script

```bash
#!/bin/bash
cd /home/sbarrachina/Komondor/Code/main

echo "Running Valgrind analysis..."
valgrind --tool=memcheck \
         --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --log-file=valgrind_output.txt \
         ./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 1 1 1 1 777

echo ""
echo "=== DOUBLE-FREE ERRORS ==="
grep -A 10 "Invalid free\|double free" valgrind_output.txt

echo ""
echo "=== ERROR SUMMARY ==="
grep "ERROR SUMMARY" valgrind_output.txt

echo ""
echo "Full report saved to: valgrind_output.txt"
```

## Interpreting Results

### If you see double-free in NodeChannelManager:
- The channel arrays (`channels_free`, `channels_for_tx`) are being freed twice
- Check if Node is still trying to free them

### If you see double-free in NodeStatisticsManager:
- Statistics arrays are being freed twice
- Check if Node is still allocating/freeing them

### If you see double-free in other locations:
- There might be other arrays with the same issue
- Look for the array name in the stack trace

## Tips

1. **Run with timeout**: Valgrind is slow, use timeout to limit execution
   ```bash
   timeout 60 valgrind --leak-check=full ./komondor_main ...
   ```

2. **Suppress known issues**: If there are framework-related false positives, create a suppression file
   ```bash
   valgrind --suppressions=suppressions.supp ./komondor_main ...
   ```

3. **Compare before/after**: Run Valgrind before and after fixes to see improvements

4. **Focus on errors first**: Fix all "Invalid free" errors before worrying about leaks

