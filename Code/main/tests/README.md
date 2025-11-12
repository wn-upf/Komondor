# Unit Tests for Komondor Managers

This directory contains unit tests for the manager classes in the Komondor simulator.

## Test Framework

The test framework is a simple, lightweight testing system that provides:
- Test assertions (TEST_ASSERT, TEST_ASSERT_EQ, TEST_ASSERT_DOUBLE_EQ)
- Test execution and reporting
- Test suite support

## Building Tests

To build all tests:
```bash
cd Code/main/tests
make
```

To build and run all tests:
```bash
make test
```

To build a specific test:
```bash
make test_node_state_manager
```

## Running Tests

To run a specific test:
```bash
./test_node_state_manager
```

## Test Coverage

### Completed Tests
- ✅ NodeStateManager (basic tests)

### Pending Tests
- 📋 NodeChannelManager
- 📋 NodeBackoffManager
- 📋 NodeNavManager
- 📋 NodeLoggingManager
- 📋 NodeMCSManager
- 📋 NodeStatisticsManager
- 📋 NodeSpatialReuseManager
- 📋 NodeConfigurationManager
- 📋 NodeTransmissionManager
- 📋 NodeReceptionManager

## Adding New Tests

1. Create a new test file: `test_<manager_name>.cpp`
2. Include `test_framework.h` and the manager header
3. Write test functions using the TEST_ASSERT macros
4. Add a main function that runs the tests
5. Update the Makefile to build the new test
6. Run the test to verify it works

## Example Test

```cpp
#include "test_framework.h"
#include "../node_state_manager.h"

void TestBasicFunctionality() {
    NodeStateManager manager;
    TEST_ASSERT_EQ(manager.GetState(), NODE_STATE_SENSING, 
                   "Initial state should be SENSING");
}

int main() {
    InitTestFramework();
    RunTest("BasicFunctionality", TestBasicFunctionality);
    CleanupTestFramework();
    return 0;
}
```

## Notes

- Tests are compiled with the same flags as the main codebase
- Tests include the necessary headers from the main codebase
- Tests use the same constants and types as the main codebase
- Tests should be kept simple and focused on individual manager functionality

