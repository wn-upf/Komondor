# Modularization Plan for Large Files

## Problem Statement

Both `node.h` (5,986 lines) and `komondor_main.cc` (1,918 lines) are **extremely large** and violate professional coding standards:

### Professional Standards
- **Recommended**: < 500-1000 lines per file
- **Best Practice**: Single Responsibility Principle (SRP)
- **Maintainability**: Large files are difficult to:
  - Navigate and understand
  - Test and debug
  - Maintain and extend
  - Review in code reviews
  - Collaborate on (merge conflicts)

### Current Situation
- `node.h`: **5,986 lines** (12x recommended size)
- `komondor_main.cc`: **1,918 lines** (4x recommended size)
- Both files have multiple responsibilities
- Difficult to test individual components
- High cognitive load for developers

## Modularization Strategy

### Phase 1: Analyze Current Structure

#### `komondor_main.cc` (1,918 lines)
**Main Responsibilities**:
1. Simulation lifecycle management (Setup, Start, Stop)
2. Network topology generation (nodes, WLANs)
3. Agent and central controller management
4. Input file parsing (nodes, agents, config)
5. Output generation (logging, statistics)
6. Configuration management
7. Node connections and relationships
8. Distance and power calculations

#### `node.h` (5,986 lines)
**Main Responsibilities**:
1. Node state management (already refactored with managers)
2. Channel management (already refactored)
3. Backoff management (already refactored)
4. NAV management (already refactored)
5. Transmission logic
6. Reception logic
7. Statistics collection
8. Configuration management
9. Logging
10. Performance measurements

### Phase 2: Modularization Plan for `komondor_main.cc`

#### Proposed Structure

```
Code/main/
├── komondor_main.cc (main file, ~200 lines)
│   └── Orchestrates simulation lifecycle
│
├── simulation/
│   ├── simulation_manager.h (SimulationManager class)
│   │   └── Handles Setup(), Start(), Stop()
│   │
│   ├── network_manager.h (NetworkManager class)
│   │   └── Handles node/WLAN generation, topology
│   │
│   ├── agent_manager.h (AgentManager class)
│   │   └── Handles agents and central controller
│   │
│   └── output_manager.h (OutputManager class)
│       └── Handles logging and output generation
│
├── input/
│   ├── input_parser.h (InputParser class)
│   │   └── Parses nodes, agents, config files
│   │
│   ├── node_file_parser.h (NodeFileParser class)
│   │   └── Specific node file parsing
│   │
│   └── agent_file_parser.h (AgentFileParser class)
│       └── Specific agent file parsing
│
├── network/
│   ├── topology_manager.h (TopologyManager class)
│   │   └── Computes distances, power, connectivity
│   │
│   └── connection_manager.h (ConnectionManager class)
│       └── Manages node connections
│
└── utils/
    ├── file_utils.h (File utilities)
    └── string_utils.h (String utilities)
```

#### Detailed Breakdown

**1. SimulationManager** (~300 lines)
- `Setup()` orchestration
- `Start()` and `Stop()` methods
- Simulation lifecycle management
- Coordinates other managers

**2. NetworkManager** (~400 lines)
- `GenerateNodesByReadingInputFile()`
- `GenerateWlans()`
- Node and WLAN container management
- Network topology initialization

**3. AgentManager** (~350 lines)
- `GenerateAgents()`
- `GenerateCentralController()`
- Agent configuration and management
- Central controller setup

**4. InputParser** (~250 lines)
- `SetupEnvironmentByReadingConfigFile()`
- Configuration file parsing
- Input validation
- File I/O operations

**5. TopologyManager** (~200 lines)
- `ComputeNodeDistancesAndPower()`
- `ComputeMaxPowerPerWlan()`
- Distance calculations
- Power calculations
- Network topology analysis

**6. ConnectionManager** (~150 lines)
- `SetupNodeConnections()`
- `SetupAgentConnections()`
- Node-to-node connections
- Agent-to-AP connections
- Traffic generator connections

**7. OutputManager** (~200 lines)
- `PrintSystemInfo()`
- `WriteSystemInfo()`
- `PrintAllWlansInfo()`
- `WriteAllWlansInfo()`
- All output generation methods

**8. NodeFileParser** (~200 lines)
- `GetNumOfLines()`
- `GetNumOfNodes()`
- Node file parsing logic
- CSV parsing for nodes

**9. AgentFileParser** (~150 lines)
- `CheckCentralController()`
- Agent file parsing logic
- CSV parsing for agents

**10. komondor_main.cc** (~200 lines)
- Main component definition
- COST framework integration
- Manager orchestration
- Public API

### Phase 3: Modularization Plan for `node.h`

#### Current Status
- ✅ Already has 11 manager classes (good!)
- ❌ But Node class itself is still 5,986 lines
- ❌ All implementation is in the header file

#### Proposed Structure

```
Code/main/
├── node.h (interface, ~200 lines)
│   └── Node class declaration
│   └── Public API
│   └── Manager declarations
│
├── node/
│   ├── node_core.h (core functionality, ~300 lines)
│   │   └── Core node logic
│   │   └── State transitions
│   │
│   ├── node_initialization.h (initialization, ~200 lines)
│   │   └── InitializeVariables()
│   │   └── Setup methods
│   │
│   ├── node_lifecycle.h (lifecycle, ~150 lines)
│   │   └── Start(), Stop()
│   │   └── Cleanup methods
│   │
│   └── node_utilities.h (utilities, ~200 lines)
│       └── Helper methods
│       └── Utility functions
│
└── (Managers already exist)
    ├── node_state_manager.h
    ├── node_channel_manager.h
    ├── node_backoff_manager.h
    ├── node_nav_manager.h
    ├── node_mcs_manager.h
    ├── node_statistics_manager.h
    ├── node_spatial_reuse_manager.h
    ├── node_configuration_manager.h
    ├── node_transmission_manager.h
    ├── node_reception_manager.h
    └── node_logging_manager.h
```

#### Challenges with `node.h`

**COST Framework Constraints**:
- COST framework uses special `component` syntax
- May require class definition in header file
- CompC++ generates C++ code from `.cc` files
- Need to verify if implementation can be split

**Possible Solutions**:
1. **Keep interface in `node.h`, move implementation to `node_impl.h`**
   - Split into logical sections
   - Include implementation in main header
   - Maintains COST framework compatibility

2. **Use inline functions in separate headers**
   - Move method implementations to separate files
   - Include them in `node.h`
   - Maintains single compilation unit

3. **Extract more functionality to managers**
   - Move remaining logic to managers
   - Reduce Node class size
   - Better separation of concerns

## Implementation Plan

### Step 1: Create Directory Structure
```bash
Code/main/
├── simulation/
├── input/
├── network/
└── utils/
```

### Step 2: Extract Managers from `komondor_main.cc`

**Priority Order**:
1. **InputParser** (lowest risk, most independent)
2. **TopologyManager** (clear boundaries)
3. **ConnectionManager** (well-defined interface)
4. **OutputManager** (independent functionality)
5. **NetworkManager** (core functionality)
6. **AgentManager** (complex but isolated)
7. **SimulationManager** (orchestrator, highest risk)

### Step 3: Refactor `node.h`

**Priority Order**:
1. Extract initialization logic to `node_initialization.h`
2. Extract lifecycle methods to `node_lifecycle.h`
3. Extract utilities to `node_utilities.h`
4. Move remaining core logic to `node_core.h`
5. Keep only interface in `node.h`

### Step 4: Testing Strategy

1. **Unit Tests**: Test each manager independently
2. **Integration Tests**: Test manager interactions
3. **Regression Tests**: Ensure functionality is preserved
4. **Performance Tests**: Verify no performance degradation

## Benefits of Modularization

### Maintainability
- ✅ Smaller, focused files (< 500 lines each)
- ✅ Clear responsibilities
- ✅ Easy to locate and fix bugs
- ✅ Reduced cognitive load

### Testability
- ✅ Each component can be tested independently
- ✅ Mock dependencies easily
- ✅ Unit tests for each manager
- ✅ Integration tests for interactions

### Extensibility
- ✅ Easy to add new features
- ✅ Clear extension points
- ✅ Reduced coupling
- ✅ Better code reuse

### Collaboration
- ✅ Multiple developers can work on different modules
- ✅ Reduced merge conflicts
- ✅ Clearer code ownership
- ✅ Easier code reviews

### Performance
- ✅ Better compilation times (smaller compilation units)
- ✅ Better IDE performance
- ✅ Faster incremental builds
- ✅ Better code navigation

## Risks and Mitigation

### Risks
1. **COST Framework Compatibility**: Framework may require specific structure
2. **Breaking Changes**: Refactoring may introduce bugs
3. **Performance Impact**: Additional function calls may affect performance
4. **Testing Effort**: Requires comprehensive testing

### Mitigation
1. **Incremental Refactoring**: One module at a time
2. **Comprehensive Testing**: Test after each extraction
3. **Backward Compatibility**: Maintain public API
4. **Performance Profiling**: Monitor performance impact
5. **Documentation**: Document each module clearly

## Timeline Estimate

### Phase 1: Preparation (1-2 days)
- Analyze current structure
- Create directory structure
- Set up testing framework

### Phase 2: Extract Input Parsing (2-3 days)
- Create InputParser classes
- Extract file parsing logic
- Test and validate

### Phase 3: Extract Network Management (3-4 days)
- Create NetworkManager
- Create TopologyManager
- Create ConnectionManager
- Test and validate

### Phase 4: Extract Output Management (2-3 days)
- Create OutputManager
- Extract output generation
- Test and validate

### Phase 5: Extract Agent Management (2-3 days)
- Create AgentManager
- Extract agent logic
- Test and validate

### Phase 6: Create SimulationManager (2-3 days)
- Create SimulationManager
- Refactor main component
- Test and validate

### Phase 7: Refactor node.h (5-7 days)
- Extract initialization logic
- Extract lifecycle methods
- Extract utilities
- Test and validate

### Phase 8: Testing and Documentation (3-5 days)
- Comprehensive testing
- Update documentation
- Performance profiling
- Code review

**Total Estimated Time**: 20-30 days

## Success Criteria

### Code Quality
- ✅ All files < 1000 lines
- ✅ Each class has single responsibility
- ✅ Clear module boundaries
- ✅ Well-documented interfaces

### Functionality
- ✅ All tests pass
- ✅ No performance degradation
- ✅ Backward compatible
- ✅ All features working

### Maintainability
- ✅ Easy to navigate codebase
- ✅ Clear code organization
- ✅ Comprehensive documentation
- ✅ Easy to extend

## Next Steps

1. **Review and Approve Plan**: Get approval for modularization approach
2. **Start with Lowest Risk**: Begin with InputParser extraction
3. **Incremental Progress**: One module at a time
4. **Continuous Testing**: Test after each extraction
5. **Document Progress**: Update documentation as we go

## Conclusion

Modularization is **essential** for professional code quality. The current files are too large and violate best practices. The proposed plan will:

- ✅ Reduce file sizes to manageable levels
- ✅ Improve code organization
- ✅ Enhance maintainability
- ✅ Facilitate testing
- ✅ Enable better collaboration

The effort is significant but **necessary** for long-term code health and maintainability.

