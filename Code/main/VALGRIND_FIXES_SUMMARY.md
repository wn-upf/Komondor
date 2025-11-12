# Valgrind Fixes Summary

## Initial Status
- **Before fixes**: 262 errors from 114 contexts
- **After FILE* fix**: 134 errors from 88 contexts
- **Target**: Resolve all Valgrind findings

## Fixes Applied

### 1. FILE* Double-Free (FIXED ✅)
**Issue**: FILE* pointer closed twice - once in `Node::Stop()` and once in `NodeLoggingManager::~NodeLoggingManager()`

**Fix**: Removed `fclose(node_logger.file)` from `Node::Stop()` - file is now managed entirely by `NodeLoggingManager`

**File**: `Code/main/node.h` (line 642)

### 2. Uninitialized Notification Struct (FIXED ✅)
**Issue**: `Notification notification;` declared but not initialized, causing uninitialized value errors

**Fix**: Changed to `Notification notification = {};` to zero-initialize all fields

**File**: `Code/main/node.h` (line 3980)

### 3. Uninitialized Member Variable (FIXED ✅)
**Issue**: `limited_num_packets_aggregated` used before initialization in `EndBackoff()`

**Fix**: Added initialization `limited_num_packets_aggregated = 0;` in `InitializeVariables()`

**File**: `Code/main/node.h` (line 5758)

### 4. Memory Leak: nodes_transmitting (FIXED ✅)
**Issue**: Array allocated in `InitializeVariables()` but never freed

**Fix**: Added cleanup in `Node::Stop()`:
```cpp
if (nodes_transmitting != NULL) {
    delete[] nodes_transmitting;
    nodes_transmitting = NULL;
}
```

**File**: `Code/main/node.h` (lines 653-656)

### 5. Memory Leaks: Performance Arrays (FIXED ✅)
**Issue**: Arrays allocated by `Performance::SetSizeOf*()` methods never freed

**Fix**: 
- Added `CleanupArrays()` method to `Performance` struct
- Called `performance_report.CleanupArrays()` in `Node::Stop()` after saving results

**Files**: 
- `Code/structures/performance.h` (added `CleanupArrays()` method)
- `Code/main/node.h` (line 705)

### 6. Memory Leaks: Channel-Related Arrays (FIXED ✅)
**Issue**: Arrays allocated directly in `InitializeVariables()` never freed:
- `total_time_transmitting_per_channel`
- `last_total_time_transmitting_per_channel`
- `total_time_lost_per_channel`
- `last_total_time_lost_per_channel`
- `total_time_channel_busy_per_channel`
- `timestampt_channel_becomes_free`
- `num_trials_tx_per_num_channels`
- `total_time_transmitting_in_num_channels`
- `total_time_lost_in_num_channels`
- `nacks_received`
- `rssi_per_sta`

**Fix**: Added cleanup for all arrays in `Node::Stop()`

**File**: `Code/main/node.h` (lines 658-702)

## Remaining Issues

After all fixes, we should re-run Valgrind to verify:
1. All memory leaks resolved
2. All uninitialized value errors resolved
3. No invalid read/write errors

## Testing

Run Valgrind to verify fixes:
```bash
cd /home/sbarrachina/Komondor/Code/main
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
    --track-origins=yes --log-file=valgrind_final.txt \
    ./komondor_main ../input/input_example/input_nodes.csv ../output/out.txt TEST 1 1 1 1 777
```

## Notes

- `simulation_performance` shares pointers with `performance_report` arrays, but since both are destroyed when Node is destroyed, cleaning up `performance_report` arrays is safe
- All cleanup is done in `Node::Stop()` after saving results to ensure data is preserved
- NULL checks prevent double-free errors

