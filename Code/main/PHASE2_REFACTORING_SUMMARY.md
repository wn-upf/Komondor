# Phase 2 Refactoring Summary

## Overview

Phase 2 focused on structural improvements and encapsulation of the `Komondor` class, specifically refactoring the large `Setup()` method into smaller, more focused helper methods and creating utility classes for configuration parsing.

## Changes Made

### 1. Refactored `Setup()` Method

**File**: `Code/main/komondor_main.cc`

The `Setup()` method was reduced from ~178 lines to ~50 lines by extracting functionality into focused helper methods:

#### New Private Helper Methods:

1. **`InitializeConfiguration()`**
   - Purpose: Initialize configuration parameters from console arguments
   - Responsibilities: Store all console input parameters into class members
   - Lines: ~15 lines

2. **`InitializeOutputFiles()`**
   - Purpose: Create and initialize output files for logging and script output
   - Responsibilities: Remove old log files, create script output file, initialize logger
   - Lines: ~20 lines

3. **`ComputeNodeDistancesAndPower()`**
   - Purpose: Compute distances and received power between all node pairs
   - Responsibilities: Allocate arrays, compute distances, compute received power
   - Lines: ~20 lines

4. **`ComputeMaxPowerPerWlan()`**
   - Purpose: Compute maximum power received from each WLAN (for APs)
   - Responsibilities: For each AP, compute max interference from each other WLAN
   - Lines: ~25 lines

5. **`InitializeTokenAccess()`**
   - Purpose: Initialize token-based channel access arrays
   - Responsibilities: Allocate and initialize token order and missed token arrays
   - Lines: ~15 lines

6. **`SetupNodeConnections()`**
   - Purpose: Setup node connections (traffic generators, notifications, MCS, spatial reuse)
   - Responsibilities: Connect traffic generators, setup notification channels, MCS connections, spatial reuse connections
   - Lines: ~30 lines

7. **`SetupAgentConnections()`**
   - Purpose: Setup agent connections (APs to agents, agents to central controller)
   - Responsibilities: Connect APs to agents, connect agents to central controller
   - Lines: ~25 lines

8. **`PrintConfigurationSummary()`**
   - Purpose: Print configuration summary (system, nodes, WLANs, and agents)
   - Responsibilities: Print system info, WLAN info, node info, agent info
   - Lines: ~15 lines

#### Benefits:

- **Improved Readability**: `Setup()` method is now a high-level orchestration method that clearly shows the setup sequence
- **Single Responsibility**: Each helper method has a single, well-defined responsibility
- **Easier Testing**: Helper methods can be tested independently (in theory, though COST framework may limit this)
- **Easier Maintenance**: Changes to specific setup steps are isolated to their respective methods
- **Better Documentation**: Each method has a clear purpose and can be documented independently

### 2. Created `ConfigurationParser` Utility Class

**File**: `Code/main/configuration_parser.h`

A new utility class for parsing configuration files with the following features:

#### Features:

- **Key-Value Parsing**: Parses configuration files with key-value pairs separated by a delimiter (default: "=")
- **Comment Support**: Ignores lines starting with "#" (comments)
- **Whitespace Handling**: Automatically trims whitespace from keys and values
- **Type-Safe Accessors**: Provides `GetInt()`, `GetDouble()`, and `GetString()` methods with default values
- **Error Handling**: Throws `ConfigurationParseException` with descriptive error messages
- **Validation**: Uses `ArgumentParser` for robust type conversion and validation

#### Methods:

- `ParseFile()`: Parse a configuration file into a map of key-value pairs
- `GetInt()`: Get an integer value from configuration map
- `GetDouble()`: Get a double value from configuration map
- `GetString()`: Get a string value from configuration map
- `Trim()`: Trim whitespace from strings (private helper)

#### Usage Example:

```cpp
// Parse configuration file
auto config = ConfigurationParser::ParseFile("../config_models");

// Get integer values with defaults
int path_loss_model = ConfigurationParser::GetInt(config, "path_loss_model", 0);
int collisions_model = ConfigurationParser::GetInt(config, "collisions_model", 0);

// Get double values with defaults
double constant_per = ConfigurationParser::GetDouble(config, "constant_per", 0.0);
```

#### Future Improvements:

- The `ConfigurationParser` is ready to be used in `SetupEnvironmentByReadingConfigFile()` if the config file format is updated to use named keys instead of positional parameters
- Currently, the existing implementation uses an index-based approach for backward compatibility

### 3. Improved Code Organization

#### Method Organization:

- Helper methods are now declared in the `private` section of the `Komondor` class
- Methods are grouped logically (configuration, setup, connections, etc.)
- Each method has clear documentation comments

#### Code Quality:

- Methods are focused and have single responsibilities
- Variable names are descriptive
- Comments explain the purpose of each method
- Error handling is consistent (though some methods still use `exit(-1)` which will be addressed in Phase 3)

## Testing

### Compilation:

The refactored code should compile without errors. The helper methods maintain the same functionality as the original `Setup()` method, just organized differently.

### Functional Testing:

- Run existing simulation scenarios to ensure functionality is preserved
- Verify that all connections are set up correctly
- Verify that configuration parameters are initialized correctly
- Verify that output files are created correctly

## Remaining Work

### Phase 2 (Partially Complete):
- ✅ Refactor `Setup()` method into smaller methods
- ✅ Create `ConfigurationParser` helper class
- ⏳ Improve error handling in `SetupEnvironmentByReadingConfigFile()` (optional, for future)
- ⏳ Apply `const` correctness to helper methods (Phase 3)

### Phase 3 (Pending):
- Replace C-style code with modern C++
- Add const correctness
- Use smart pointers for memory management
- Improve error handling (replace `exit(-1)` with exceptions or return codes)

### Phase 4 (Pending):
- Add Doxygen documentation
- Create unit tests

## Files Modified

1. `Code/main/komondor_main.cc` - Refactored `Setup()` method and added helper methods
2. `Code/main/configuration_parser.h` - New utility class for configuration parsing

## Files Created

1. `Code/main/configuration_parser.h` - Configuration parsing utility class
2. `Code/main/PHASE2_REFACTORING_SUMMARY.md` - This document

## Notes

- The refactoring maintains backward compatibility with existing code
- The `ConfigurationParser` is ready for use but the existing config file parsing still uses the index-based approach for compatibility
- All helper methods are private, maintaining encapsulation
- The `Setup()` method is now much more readable and maintainable

## Next Steps

1. Test the refactored code with existing simulation scenarios
2. Consider updating `SetupEnvironmentByReadingConfigFile()` to use `ConfigurationParser` if config file format can be updated
3. Continue with Phase 3: Modern C++ improvements and error handling
4. Add comprehensive documentation and unit tests in Phase 4

