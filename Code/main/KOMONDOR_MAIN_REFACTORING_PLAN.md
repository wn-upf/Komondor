# komondor_main.cc - Refactoring Plan

## Executive Summary

**Current Status**: Functional but not professional by modern C++ standards
**Recommendation**: Yes, significant improvements are needed
**Priority**: High - affects maintainability, safety, and extensibility

## Key Issues Identified

### Critical Issues (Must Fix)
1. ❌ **Global variables** - `total_nodes_number`, `tmp_nodes`
2. ❌ **Memory leaks** - Arrays allocated but not always freed
3. ❌ **No input validation** - Can crash on invalid input
4. ❌ **Inconsistent error handling** - Mix of exit(), return(), no handling

### High Priority (Should Fix)
1. ⚠️ **Very long methods** - 200+ line methods violate SRP
2. ⚠️ **Poor separation of concerns** - Komondor does too much
3. ⚠️ **Manual memory management** - Mix of new/delete and malloc/free
4. ⚠️ **No const correctness** - Missing const on methods/parameters

### Medium Priority (Nice to Have)
1. ⚠️ **Hard-coded paths** - Not portable
2. ⚠️ **Mixed C/C++ style** - Should use modern C++
3. ⚠️ **Commented-out code** - Should be removed
4. ⚠️ **Limited documentation** - Needs Doxygen comments

## Refactoring Strategy

### Phase 1: Critical Fixes (Week 1)

#### 1.1 Remove Global Variables
**Current**:
```cpp
int total_nodes_number;  // Global
char* tmp_nodes;         // Global
```

**Proposed**:
```cpp
// Move to Komondor class as member variables
class Komondor {
    int total_nodes_number_;  // Private member
    // Remove tmp_nodes, use std::string instead
};
```

**Benefits**:
- Better encapsulation
- Thread-safe (if needed in future)
- Easier to test

#### 1.2 Fix Memory Management
**Current**:
```cpp
Performance *performance_per_node = new Performance[total_nodes_number];
// ... use it ...
// Never deleted! Memory leak!
```

**Proposed**:
```cpp
// Option 1: Use std::vector
std::vector<Performance> performance_per_node(total_nodes_number);

// Option 2: Use smart pointers
std::vector<std::unique_ptr<Performance>> performance_per_node;
```

**Benefits**:
- Automatic cleanup
- Exception-safe
- No memory leaks

#### 1.3 Add Input Validation
**Current**:
```cpp
nodes_input_filename = argv[1];  // No validation!
```

**Proposed**:
```cpp
if (argc < 2) {
    throw std::invalid_argument("Missing nodes input filename");
}
std::string nodes_input_filename = argv[1];
if (!FileExists(nodes_input_filename)) {
    throw std::runtime_error("File not found: " + nodes_input_filename);
}
```

**Benefits**:
- Early error detection
- Better error messages
- Prevents crashes

#### 1.4 Consistent Error Handling
**Current**:
```cpp
exit(-1);  // Some places
return(-1); // Other places
// No handling in some places
```

**Proposed**:
```cpp
// Use exceptions for errors
class KomondorException : public std::runtime_error {
    // Custom exception class
};

// Or return error codes with Result<T> pattern
template<typename T>
class Result {
    bool success_;
    T value_;
    std::string error_;
};
```

**Benefits**:
- Consistent error handling
- Can be caught and handled
- Better error messages

### Phase 2: Code Structure Improvements (Week 2)

#### 2.1 Break Down Large Methods
**Current**: `Setup()` is 200+ lines

**Proposed**: Split into smaller methods
```cpp
void Komondor::Setup(...) {
    InitializeConfiguration(...);
    SetupOutputFiles(...);
    SetupEnvironment(...);
    GenerateNodes(...);
    CalculateDistances(...);
    InitializeAgents(...);
}
```

**Benefits**:
- Easier to test
- Easier to understand
- Easier to maintain

#### 2.2 Extract File I/O Operations
**Current**: File I/O mixed with business logic

**Proposed**: Create `FileManager` class
```cpp
class FileManager {
public:
    static bool FileExists(const std::string& path);
    static std::string ReadFile(const std::string& path);
    static void WriteFile(const std::string& path, const std::string& content);
    static void CreateDirectory(const std::string& path);
};
```

**Benefits**:
- Separation of concerns
- Reusable
- Testable

#### 2.3 Extract Configuration Parsing
**Current**: Configuration parsing in `main()`

**Proposed**: Create `ConfigurationParser` class
```cpp
class ConfigurationParser {
public:
    struct Config {
        std::string nodes_filename;
        std::string agents_filename;
        double simulation_time;
        int seed;
        // ... other config
    };
    
    static Config ParseCommandLine(int argc, char* argv[]);
    static Config ParseConfigFile(const std::string& filename);
};
```

**Benefits**:
- Cleaner main()
- Testable
- Reusable

#### 2.4 Extract Node Generation Logic
**Current**: Node generation in `Komondor::GenerateNodesByReadingInputFile()`

**Proposed**: Create `NodeFactory` class
```cpp
class NodeFactory {
public:
    static std::vector<Node> CreateNodesFromFile(
        const std::string& filename,
        const EnvironmentConfig& env_config
    );
    static Node CreateNodeFromCSVLine(const std::string& line, ...);
};
```

**Benefits**:
- Single responsibility
- Testable
- Reusable

### Phase 3: Modern C++ Improvements (Week 3)

#### 3.1 Replace C-style Code with C++
**Current**:
```cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char* tmp_nodes;
```

**Proposed**:
```cpp
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
std::string tmp_nodes;  // Or std::vector<char>
```

**Benefits**:
- Type-safe
- Exception-safe
- More readable

#### 3.2 Add Const Correctness
**Current**:
```cpp
void PrintSystemInfo();  // Should be const
void Setup(double sim_time, ...);  // Parameters should be const
```

**Proposed**:
```cpp
void PrintSystemInfo() const;  // Const method
void Setup(double sim_time, const std::string& nodes_filename, ...);  // Const parameters
```

**Benefits**:
- Clearer intent
- Compiler optimizations
- Prevents accidental modifications

#### 3.3 Use Smart Pointers
**Current**:
```cpp
Wlan *wlan_container;  // Raw pointer
delete[] wlan_container;  // Manual cleanup
```

**Proposed**:
```cpp
std::vector<Wlan> wlan_container_;  // Or std::unique_ptr<Wlan[]>
// Automatic cleanup
```

**Benefits**:
- Automatic cleanup
- Exception-safe
- No memory leaks

#### 3.4 Remove Hard-coded Paths
**Current**:
```cpp
if (stat("../output/", &st) == -1) {
    mkdir("../output/", 0777);
}
```

**Proposed**:
```cpp
class PathManager {
public:
    static std::string GetOutputDirectory();
    static std::string GetInputDirectory();
    static void CreateDirectoryIfNotExists(const std::string& path);
};

// Usage
PathManager::CreateDirectoryIfNotExists(PathManager::GetOutputDirectory());
```

**Benefits**:
- Portable
- Configurable
- Testable

### Phase 4: Documentation and Testing (Week 4)

#### 4.1 Add Doxygen Documentation
**Proposed**:
```cpp
/**
 * @brief Main Komondor simulation component
 * 
 * This class orchestrates the entire simulation, including:
 * - Node generation and initialization
 * - Agent setup and configuration
 * - Simulation execution
 * - Results collection and output
 * 
 * @note This class inherits from CostSimEng and uses the COST framework
 */
class Komondor : public CostSimEng {
    // ...
};
```

#### 4.2 Add Unit Tests
**Proposed**: Create test files
- `test_komondor_setup.cpp`
- `test_configuration_parser.cpp`
- `test_node_factory.cpp`
- `test_file_manager.cpp`

## Implementation Plan

### Step 1: Create Helper Classes (Low Risk)
1. Create `FileManager` class
2. Create `PathManager` class
3. Create `ConfigurationParser` class
4. Test each class independently

### Step 2: Refactor Memory Management (Medium Risk)
1. Replace raw pointers with smart pointers
2. Replace C arrays with std::vector
3. Test after each change
4. Run Valgrind to verify no leaks

### Step 3: Refactor Large Methods (Medium Risk)
1. Extract file I/O operations
2. Extract configuration parsing
3. Extract node generation
4. Test after each extraction

### Step 4: Remove Global Variables (High Risk)
1. Move globals to class members
2. Update all references
3. Test thoroughly
4. Run full regression tests

### Step 5: Add Error Handling (Medium Risk)
1. Define exception classes
2. Replace exit()/return() with exceptions
3. Add try-catch blocks in main()
4. Test error cases

### Step 6: Modernize C++ Code (Low Risk)
1. Replace C headers with C++ headers
2. Add const correctness
3. Use modern C++ features (auto, range-based for, etc.)
4. Test after each change

## Risk Assessment

### Low Risk Changes
- Adding helper classes
- Adding documentation
- Replacing C headers with C++ headers
- Adding const correctness

### Medium Risk Changes
- Refactoring large methods
- Replacing memory management
- Adding error handling

### High Risk Changes
- Removing global variables
- Changing public API
- Major architectural changes

## Testing Strategy

1. **Unit Tests**: Test each new class/method independently
2. **Integration Tests**: Test interactions between components
3. **Regression Tests**: Run existing simulations to verify no breakage
4. **Valgrind**: Verify no memory leaks after each change
5. **Performance Tests**: Ensure no performance regression

## Success Criteria

1. ✅ No global variables
2. ✅ No memory leaks (Valgrind clean)
3. ✅ All methods < 50 lines
4. ✅ All public methods have Doxygen comments
5. ✅ 80%+ code coverage with unit tests
6. ✅ All inputs validated
7. ✅ Consistent error handling
8. ✅ No hard-coded paths
9. ✅ Modern C++ idioms used
10. ✅ All tests pass

## Estimated Effort

- **Phase 1 (Critical Fixes)**: 1 week
- **Phase 2 (Code Structure)**: 1 week
- **Phase 3 (Modern C++)**: 1 week
- **Phase 4 (Documentation/Testing)**: 1 week
- **Total**: 4 weeks (1 month)

## Conclusion

The code **needs significant improvement** to meet professional standards. The refactoring plan above provides a structured approach to improving the codebase while minimizing risk. The improvements will make the code:

- **More maintainable**: Easier to understand and modify
- **More robust**: Better error handling and validation
- **More testable**: Smaller, focused methods
- **More portable**: No hard-coded paths
- **More professional**: Modern C++ idioms and best practices

**Recommendation**: Proceed with the refactoring plan, starting with Phase 1 (Critical Fixes).

