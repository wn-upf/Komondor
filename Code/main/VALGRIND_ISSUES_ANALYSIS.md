# Valgrind Issues Analysis

## Current Status
- **Before FILE* fix**: 262 errors from 114 contexts
- **After FILE* fix**: 134 errors from 88 contexts
- **Progress**: 128 errors fixed (49% reduction)

## Error Categories

### 1. Memory Leaks (Definitely Lost)
**Total**: 13,540 bytes in 191 blocks

#### Key Leaks Identified:
- `nodes_transmitting` array (line 5718) - never freed
- `performance_report` arrays:
  - `SetSizeOfRssiList()` allocations (line 5860, 5866)
  - `SetSizeOfRssiPerStaList()` allocations (line 5862)
  - `SetSizeOfChannelLists()` allocations (line 5859)
- Various arrays in `komondor_main.cc`:
  - Line 291, 292: Arrays in Setup()
  - Line 247, 266: Arrays in Setup()

### 2. Memory Leaks (Indirectly Lost)
**Total**: 2,104 bytes in 42 blocks
- Related to arrays that are indirectly lost through parent objects

### 3. Uninitialized Value Errors
**Count**: ~100+ errors

#### Locations:
- `EndBackoff()` (line 3630) - conditional jump on uninitialized value
- `InportSomeNodeStartTX()` (lines 2377, 769, 776) - uninitialized values
- `GenerateNotification()` (line 3972) - uninitialized stack allocation
- `PrintOrWriteNodeStatistics()` (line 5354) - uninitialized values in printf

### 4. Invalid Read/Write Errors
**Count**: ~30 errors
- Most related to FILE* operations (should be fixed now)
- Some related to accessing freed memory

## Fix Priority

1. **High Priority**: Memory leaks (definitely lost) - causes actual memory issues
2. **Medium Priority**: Uninitialized values - can cause undefined behavior
3. **Low Priority**: Indirectly lost - less critical, but should be fixed

## Fix Strategy

### Phase 1: Memory Leaks
1. Add cleanup for `nodes_transmitting` in Node destructor/Stop()
2. Ensure `performance_report` arrays are properly cleaned up
3. Check all `new[]` allocations have corresponding `delete[]`

### Phase 2: Uninitialized Values
1. Initialize all member variables in constructors
2. Initialize stack variables before use
3. Fix uninitialized values in PrintOrWriteNodeStatistics

### Phase 3: Verification
1. Re-run Valgrind
2. Verify all errors resolved
3. Document fixes

