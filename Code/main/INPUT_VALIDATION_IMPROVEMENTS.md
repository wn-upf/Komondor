# Input Validation and Error Handling Improvements

## Summary

This document summarizes the improvements made to `komondor_main.cc` for robust input validation and error handling as part of Phase 1 refactoring.

## Changes Made

### 1. Created `ArgumentParser` Utility Class

**File**: `Code/main/argument_parser.h`

A new utility class that provides:
- **Safe integer parsing**: `ParseInt()` with overflow detection
- **Safe double parsing**: `ParseDouble()` with overflow/underflow detection
- **Range validation**: `ParseIntRange()` and `ParseDoubleRange()` for value validation
- **File validation**: `ValidateFileExists()` to verify input files exist and are readable
- **Exception-based error handling**: `ArgumentParseException` for clear error reporting

**Benefits**:
- Prevents undefined behavior from invalid input
- Provides clear, descriptive error messages
- Handles edge cases (empty strings, overflow, invalid formats)
- Consistent error reporting throughout the application

### 2. Refactored `main()` Function

**File**: `Code/main/komondor_main.cc`

#### Input Validation Improvements

1. **Replaced `atoi()` and `atof()` calls**:
   - All command-line arguments now use `ArgumentParser::ParseInt()` or `ArgumentParser::ParseDouble()`
   - Prevents crashes from invalid numeric input
   - Detects overflow/underflow conditions

2. **Added file validation**:
   - Input files are validated before use using `ArgumentParser::ValidateFileExists()`
   - Clear error messages if files are missing or unreadable

3. **Added range validation**:
   - `sim_time` must be positive ( > 0.0)
   - `seed` must be non-negative ( >= 0)
   - All boolean flags (save_node_logs, print_system_logs, etc.) must be 0 or 1

4. **Improved error messages**:
   - Usage message displayed on argument errors
   - Specific error messages for each validation failure
   - Error messages include the problematic argument name and value

#### Error Handling Improvements

1. **Exception-based error handling**:
   - Wrapped argument parsing in `try-catch` blocks
   - `ArgumentParseException` for argument-related errors
   - `std::exception` for other errors
   - Catch-all handler for unknown errors

2. **Graceful error recovery**:
   - Returns error codes (1) instead of calling `exit(-1)`
   - Allows for proper cleanup and resource deallocation
   - Error messages printed to `stderr` for better separation from normal output

3. **Improved output directory handling**:
   - Uses `PathManager::GetOutputDirectory()` for consistent path handling
   - Uses `FileManager::CreateDirectoryIfNotExists()` for directory creation
   - Better error messages if directory creation fails

#### Code Quality Improvements

1. **Initialized variables**:
   - All variables are properly initialized with default values
   - Prevents uninitialized variable usage

2. **Used `const` where appropriate**:
   - Input filenames are `const char*` to prevent modification
   - Usage message is `const std::string`

3. **Improved readability**:
   - Clear separation between argument parsing, validation, and simulation execution
   - Consistent error handling pattern
   - Better comments explaining validation logic

## Testing

### Test Cases

1. **Invalid number of arguments**:
   ```bash
   ./komondor_main
   # Expected: Error message with usage instructions
   ```

2. **Invalid file path**:
   ```bash
   ./komondor_main nonexistent_file.txt 10.0 777
   # Expected: Error message indicating file not found
   ```

3. **Invalid numeric input**:
   ```bash
   ./komondor_main input_nodes.csv abc 777
   # Expected: Error message indicating invalid double format
   ```

4. **Out of range values**:
   ```bash
   ./komondor_main input_nodes.csv -5.0 777
   # Expected: Error message indicating sim_time must be positive
   ```

5. **Invalid flag values**:
   ```bash
   ./komondor_main input_nodes.csv output.txt TEST 2 1 1 10.0 777
   # Expected: Error message indicating save_node_logs must be 0 or 1
   ```

### Validation

- All argument parsing uses `ArgumentParser` methods
- All file operations validate file existence
- All numeric inputs are validated for range and format
- Error messages are clear and informative
- Error handling is consistent and graceful

## Remaining Work

### Phase 1 (Completed)
- ✅ Remove global variables
- ✅ Fix memory leaks
- ✅ Add input validation
- ✅ Improve error handling

### Phase 2 (Pending)
- Refactor large methods in `komondor_main.cc`
- Create `ConfigurationParser` helper class
- Improve error handling in `Setup()` and other methods

### Phase 3 (Pending)
- Replace C-style code with modern C++
- Add const correctness
- Use smart pointers for memory management

### Phase 4 (Pending)
- Add Doxygen documentation
- Create unit tests

## Notes

- The `Setup()` method and other internal methods still use `exit(-1)` for error handling. These will be addressed in Phase 2.
- The `ArgumentParser` class is designed to be extensible and can be enhanced with additional validation methods as needed.
- File validation is performed early in `main()` to fail fast if input files are missing.

## Files Modified

1. `Code/main/komondor_main.cc` - Refactored `main()` function
2. `Code/main/argument_parser.h` - New utility class for argument parsing
3. `Code/main/path_manager.h` - Updated `GetOutputDirectory()` to return `"../output"`
4. `Code/main/file_manager.h` - Added `mode_t` typedef for Windows compatibility

## Files Created

1. `Code/main/argument_parser.h` - Argument parsing utility class
2. `Code/main/INPUT_VALIDATION_IMPROVEMENTS.md` - This document

