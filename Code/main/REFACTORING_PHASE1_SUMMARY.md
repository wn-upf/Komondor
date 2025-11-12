# komondor_main.cc Refactoring - Phase 1 Summary

## Completed Tasks ✅

### 1. Created Helper Classes
- ✅ `FileManager` class (`file_manager.h`) - File I/O operations with RAII
- ✅ `PathManager` class (`path_manager.h`) - Path management utilities (fixed `component` parameter name conflict)

### 2. Removed Global Variables
- ✅ Removed global `total_nodes_number` - moved to `Komondor` class as public member
- ✅ Removed global `tmp_nodes` - converted to local variables in functions that use it
- ✅ Fixed all references to use class members/local variables

### 3. Fixed Memory Leaks (Partial)
- ✅ Replaced `new Performance[]` with `std::vector<Performance>` in `Stop()` method
- ✅ Replaced `new Configuration[]` with `std::vector<Configuration>` in `Stop()` method
- ✅ Added null check before `fclose(script_output_file)`
- ✅ Added cleanup for arrays allocated in `komondor_main.cc`:
  - `distances_array`
  - `received_power_array`
  - `max_received_power_in_ap_per_wlan`
  - `token_order_list`
  - `num_missed_tokens_list`
- ✅ Added cleanup in `Node::Stop()` for arrays allocated in `komondor_main.cc`
- ✅ Set `simulation_performance` pointers to NULL before freeing to avoid dangling pointers

### 4. Improved Code Quality
- ✅ Added includes for `file_manager.h` and `path_manager.h`
- ✅ Added `#include <cstring>` and `#include <memory>` for modern C++
- ✅ Fixed `tmp_nodes` usage to use local variables with proper `free()` calls

## Remaining Issues ⚠️

### 1. Double-Free Error (Critical)
- **Status**: Still occurring
- **Symptoms**: `free(): double free detected in tcache 2` during simulation exit
- **Possible Causes**:
  - `performance_report.CleanupArrays()` might be freeing arrays that are also freed in `Node::Stop()`
  - Arrays might be shared between `performance_report` and `simulation_performance`
  - Need to investigate with Valgrind to identify exact source

### 2. Input Validation (Pending)
- Command-line arguments not validated
- File existence not checked before opening
- No error handling for invalid inputs

### 3. Error Handling (Pending)
- Still using `exit(-1)` and `return(-1)`
- No exception handling
- No consistent error reporting

## Build Status
- ✅ **Compilation**: Successful
- ⚠️ **Runtime**: Double-free error on exit

## Next Steps

### Immediate (Critical)
1. **Fix Double-Free**: Use Valgrind to identify exact source, then fix
2. **Investigate Array Sharing**: Check if `performance_report` and `simulation_performance` share arrays

### Short-term
1. Add input validation
2. Improve error handling
3. Replace hard-coded paths with `PathManager`
4. Use `FileManager` for file operations

### Long-term
1. Refactor large methods
2. Add const correctness
3. Use smart pointers
4. Add unit tests
5. Add Doxygen documentation

## Files Modified
1. `komondor_main.cc` - Removed globals, fixed memory leaks, added helper includes
2. `node.h` - Added cleanup for arrays allocated in `komondor_main.cc`
3. `file_manager.h` - New helper class
4. `path_manager.h` - New helper class (fixed parameter name conflict)

## Manager Classes Status
- ✅ **Confirmed**: All 11 manager classes ARE being used in `node.h`
- ✅ All managers are declared as private members
- ✅ All managers are actively used throughout node.h methods

## Notes
- The double-free error needs to be resolved before proceeding with further refactoring
- The code builds successfully, but runtime errors indicate memory management issues
- Valgrind should be used to identify the exact source of the double-free

