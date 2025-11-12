# komondor_main.cc Refactoring Progress

## Status: Phase 1 - Critical Fixes (In Progress)

### Completed
1. ✅ Created `FileManager` helper class (`file_manager.h`)
2. ✅ Created `PathManager` helper class (`path_manager.h`)

### In Progress
1. 🔄 Remove global variables (`total_nodes_number`, `tmp_nodes`)
2. 🔄 Fix memory leaks (arrays in `Stop()` method)
3. 🔄 Add input validation
4. 🔄 Improve error handling

### Next Steps
1. Move global variables to Komondor class members
2. Replace `new Performance[]` with `std::vector`
3. Add validation for command-line arguments
4. Replace `exit()`/`return(-1)` with exceptions

## Manager Classes Status

### Confirmed: Managers ARE being used in node.h
- ✅ `NodeStateManager` - Used for state management
- ✅ `NodeLoggingManager` - Used for logging
- ✅ `NodeChannelManager` - Used for channel management
- ✅ `NodeBackoffManager` - Used for backoff management
- ✅ `NodeNavManager` - Used for NAV management
- ✅ `NodeMCSManager` - Used for MCS management
- ✅ `NodeStatisticsManager` - Used for statistics
- ✅ `NodeSpatialReuseManager` - Used for spatial reuse
- ✅ `NodeConfigurationManager` - Used for configuration
- ✅ `NodeTransmissionManager` - Used for transmission
- ✅ `NodeReceptionManager` - Used for reception

All managers are declared as private members and actively used in node.h methods.

