# Node Component Refactoring Plan

## Executive Summary

The `node.h` file (5,667 lines) is a monolithic component that violates the Single Responsibility Principle and makes maintenance, testing, and extension difficult. This plan outlines a comprehensive refactoring to split it into modular, well-documented, and extensible components following professional C++ conventions.

## Current Issues

### 1. **Size and Complexity**
- **5,667 lines** in a single file
- **45+ methods** with some exceeding 300 lines
- **100+ member variables** mixing concerns
- Difficult to navigate and understand

### 2. **Code Quality Issues**
- **31 TODO/FIXME comments** indicating incomplete features
- **22 commented-out code blocks** (deprecated/dead code)
- **Hardcoded values** scattered throughout
- **Deprecated variables** (BER, PER) still present
- Missing documentation for complex algorithms

### 3. **Architectural Issues**
- **Mixed concerns**: Transmission, reception, configuration, statistics, logging all in one class
- **Tight coupling**: Direct dependencies on implementation details
- **Poor separation**: Public/private access not well organized
- **No abstraction**: Difficult to extend or test individual components

### 4. **Maintainability Issues**
- Methods too long (e.g., `EndBackoff`: 340 lines, `InitializeVariables`: 308 lines)
- Duplicate logic in multiple places
- Complex state management spread across methods
- Difficult to unit test individual components

## Refactoring Strategy

### Phase 1: Extract Supporting Classes (Internal Modules)

Split the monolithic Node component into focused, single-responsibility classes that will be used by the Node component.

#### 1.1 **Node State Manager** (`node_state_manager.h`)
**Responsibility**: Manage node internal state and state transitions

**Extracted from**:
- `node_state` variable and state management logic
- State transition methods
- State validation

**Public Interface**:
```cpp
class NodeStateManager {
public:
    enum NodeState { SENSING, TRANSMITTING, RECEIVING, NAV, ... };
    void SetState(NodeState new_state);
    NodeState GetState() const;
    bool IsInState(NodeState state) const;
    bool CanTransmit() const;
    bool CanReceive() const;
};
```

**Benefits**:
- Encapsulates state logic
- Easier to test state transitions
- Prevents invalid state changes

#### 1.2 **Channel Manager** (`node_channel_manager.h`)
**Responsibility**: Manage channel selection, channel bonding, and channel state

**Extracted from**:
- Channel power detection (`channel_power`)
- Channel selection logic
- Channel bonding policies
- Channel availability checking

**Public Interface**:
```cpp
class NodeChannelManager {
public:
    void UpdateChannelPower(int channel, double power);
    bool IsChannelFree(int channel, double pd_threshold) const;
    std::vector<int> GetAvailableChannels(double pd_threshold) const;
    std::pair<int, int> GetTxChannels(int bonding_policy) const;
    void ResetChannelState();
};
```

**Benefits**:
- Isolates channel-related logic
- Easier to test channel selection algorithms
- Can be extended with new bonding policies

#### 1.3 **Transmission Manager** (`node_transmission_manager.h`)
**Responsibility**: Handle packet transmission, RTS/CTS/DATA/ACK generation

**Extracted from**:
- `GenerateNotification()`
- `StartTransmission()`
- `SendResponsePacket()`
- RTS/CTS/DATA/ACK creation logic
- Transmission power management

**Public Interface**:
```cpp
class NodeTransmissionManager {
public:
    Notification GenerateRTS(int destination_id, ...);
    Notification GenerateCTS(const Notification& rts);
    Notification GenerateDATA(int destination_id, ...);
    Notification GenerateACK(const Notification& data);
    void PrepareTransmission(const Notification& packet);
    bool CanStartTransmission() const;
};
```

**Benefits**:
- Separates transmission logic from node state
- Easier to test packet generation
- Can be extended with new packet types

#### 1.4 **Reception Manager** (`node_reception_manager.h`)
**Responsibility**: Handle packet reception, decoding, interference calculation

**Extracted from**:
- `InportSomeNodeStartTX()`
- `InportSomeNodeFinishTX()`
- Packet decoding logic
- SINR calculation
- Interference handling
- Capture effect

**Public Interface**:
```cpp
class NodeReceptionManager {
public:
    enum PacketLossReason { NOT_LOST, COLLISION, INTERFERENCE, ... };
    PacketLossReason ProcessIncomingPacket(const Notification& packet);
    double CalculateSINR(const Notification& packet) const;
    bool CanDecodePacket(const Notification& packet, double pd_threshold) const;
    void UpdateInterference(const Notification& packet);
};
```

**Benefits**:
- Isolates reception logic
- Easier to test decoding algorithms
- Can be extended with new interference models

#### 1.5 **Backoff Manager** (`node_backoff_manager.h`)
**Responsibility**: Manage backoff procedures, contention window, DCF

**Extracted from**:
- `PauseBackoff()`
- `ResumeBackoff()`
- `EndBackoff()`
- Backoff computation
- Contention window management
- Slotted/continuous backoff logic

**Public Interface**:
```cpp
class NodeBackoffManager {
public:
    void StartBackoff();
    void PauseBackoff();
    void ResumeBackoff();
    void EndBackoff();
    double GetRemainingBackoff() const;
    bool IsBackoffActive() const;
    void UpdateContentionWindow(bool success);
};
```

**Benefits**:
- Encapsulates backoff logic
- Easier to test backoff algorithms
- Can be extended with new backoff schemes

#### 1.6 **NAV Manager** (`node_nav_manager.h`)
**Responsibility**: Manage Network Allocation Vector (NAV) and virtual carrier sense

**Extracted from**:
- NAV timeout handling
- NAV state management
- Inter-BSS NAV handling
- NAV collision detection

**Public Interface**:
```cpp
class NodeNavManager {
public:
    void SetNAV(double duration, bool is_inter_bss = false);
    void ClearNAV();
    bool IsInNAV() const;
    double GetRemainingNAVTime() const;
    void UpdateNAV(const Notification& packet);
};
```

**Benefits**:
- Isolates NAV logic
- Easier to test NAV behavior
- Can be extended with new NAV policies

#### 1.7 **Spatial Reuse Manager** (`node_spatial_reuse_manager.h`)
**Responsibility**: Handle spatial reuse operations, OBSS-PD, TXOP identification

**Extracted from**:
- `SpatialReuseOpportunityEnds()`
- `InportRequestSpatialReuseConfiguration()`
- `InportNewSpatialReuseConfiguration()`
- OBSS-PD threshold management
- TXOP identification
- Power restriction

**Public Interface**:
```cpp
class NodeSpatialReuseManager {
public:
    bool IdentifySpatialReuseOpportunity(double power_rx, double obss_pd_threshold);
    void ApplySpatialReuseConfiguration(const Configuration& config);
    double GetOBSSPDThreshold() const;
    double GetLimitedTxPower(double obss_pd) const;
    bool IsSpatialReuseEnabled() const;
};
```

**Benefits**:
- Isolates spatial reuse logic
- Easier to test SR algorithms
- Can be extended with new SR policies

#### 1.8 **MCS Manager** (`node_mcs_manager.h`)
**Responsibility**: Manage Modulation and Coding Scheme selection and adaptation

**Extracted from**:
- `RequestMCS()`
- `InportMCSRequestReceived()`
- `InportMCSResponseReceived()`
- MCS selection based on SINR
- MCS per node tracking

**Public Interface**:
```cpp
class NodeMCSManager {
public:
    void RequestMCS(int destination_id, int num_channels);
    void ProcessMCSRequest(const Notification& request);
    void ProcessMCSResponse(const Notification& response);
    int GetMCSForDestination(int destination_id, int num_channels) const;
    void UpdateMCSBasedOnSINR(double sinr, int destination_id);
};
```

**Benefits**:
- Encapsulates MCS logic
- Easier to test MCS selection
- Can be extended with new MCS algorithms

#### 1.9 **Configuration Manager** (`node_configuration_manager.h`)
**Responsibility**: Handle configuration management and agent interactions

**Extracted from**:
- `GenerateConfiguration()`
- `ApplyNewConfiguration()`
- `BroadcastNewConfigurationToStas()`
- `InportReceiveConfigurationFromAgent()`
- Configuration validation

**Public Interface**:
```cpp
class NodeConfigurationManager {
public:
    Configuration GenerateConfiguration() const;
    void ApplyConfiguration(const Configuration& config);
    void BroadcastConfigurationToSTAs(const Configuration& config);
    bool ValidateConfiguration(const Configuration& config) const;
    Configuration GetCurrentConfiguration() const;
};
```

**Benefits**:
- Isolates configuration logic
- Easier to test configuration handling
- Can be extended with new configuration types

#### 1.10 **Statistics Manager** (`node_statistics_manager.h`)
**Responsibility**: Collect and manage performance statistics

**Extracted from**:
- `UpdatePerformanceMeasurements()`
- `SaveSimulationPerformance()`
- `PrintOrWriteNodeStatistics()`
- All statistics variables and calculations
- Performance report generation

**Public Interface**:
```cpp
class NodeStatisticsManager {
public:
    void RecordPacketSent(int packet_type);
    void RecordPacketReceived(int packet_type);
    void RecordPacketLost(int reason);
    void RecordTransmissionTime(double duration, int num_channels);
    void UpdateThroughput(double bits, double time);
    Performance GeneratePerformanceReport() const;
    void ResetStatistics();
};
```

**Benefits**:
- Separates statistics from business logic
- Easier to test statistics collection
- Can be extended with new metrics

#### 1.11 **Logging Manager** (`node_logging_manager.h`)
**Responsibility**: Handle logging and output generation

**Extracted from**:
- `WriteNodeInfo()`
- `WriteNodeConfiguration()`
- `PrintNodeInfo()`
- `PrintNodeConfiguration()`
- File logging logic

**Public Interface**:
```cpp
class NodeLoggingManager {
public:
    void InitializeLogging(const std::string& file_path);
    void LogNodeInfo(int detail_level);
    void LogNodeConfiguration();
    void LogEvent(const std::string& event, int level);
    void CloseLogging();
};
```

**Benefits**:
- Isolates logging logic
- Easier to test logging
- Can be extended with new log formats

### Phase 2: Refactor Main Node Component

#### 2.1 **Node Component Structure** (`node.h`)

The main Node component will become a coordinator that uses the managers:

```cpp
component Node : public TypeII {
private:
    // Managers (composition)
    NodeStateManager state_manager_;
    NodeChannelManager channel_manager_;
    NodeTransmissionManager transmission_manager_;
    NodeReceptionManager reception_manager_;
    NodeBackoffManager backoff_manager_;
    NodeNavManager nav_manager_;
    NodeSpatialReuseManager spatial_reuse_manager_;
    NodeMCSManager mcs_manager_;
    NodeConfigurationManager config_manager_;
    NodeStatisticsManager statistics_manager_;
    NodeLoggingManager logging_manager_;
    
    // Core node data (simplified)
    NodeConfiguration config_;
    NodeStatistics statistics_;
    
public:
    // COST lifecycle
    void Setup();
    void Start();
    void Stop();
    
    // Inports (delegated to managers)
    inport void InportSomeNodeStartTX(Notification &notification);
    inport void InportSomeNodeFinishTX(Notification &notification);
    // ... other inports
    
    // Outports
    outport void outportSelfStartTX(Notification &notification);
    // ... other outports
    
    // Triggers
    Timer<trigger_t> trigger_end_backoff;
    // ... other timers
};
```

#### 2.2 **File Structure**

```
Code/main/
├── node.h                          # Main Node component (coordinator)
├── node_state_manager.h            # State management
├── node_channel_manager.h          # Channel management
├── node_transmission_manager.h     # Transmission management
├── node_reception_manager.h        # Reception management
├── node_backoff_manager.h          # Backoff management
├── node_nav_manager.h              # NAV management
├── node_spatial_reuse_manager.h    # Spatial reuse management
├── node_mcs_manager.h              # MCS management
├── node_configuration_manager.h    # Configuration management
├── node_statistics_manager.h       # Statistics management
├── node_logging_manager.h          # Logging management
└── node_internal/
    ├── node_types.h                # Internal types and enums
    ├── node_constants.h            # Node-specific constants
    └── node_utilities.h            # Utility functions
```

### Phase 3: Code Cleanup

#### 3.1 **Remove Redundant Code**
- Remove all commented-out code blocks
- Remove deprecated variables (BER, PER)
- Remove hardcoded test values
- Consolidate duplicate logic

#### 3.2 **Extract Constants**
- Move hardcoded values to `node_constants.h`
- Use named constants instead of magic numbers
- Group related constants

#### 3.3 **Improve Documentation**
- Add Doxygen comments for all public methods
- Document complex algorithms
- Add usage examples for key methods
- Document state transitions
- Add architecture diagrams

#### 3.4 **Standardize Naming**
- Use consistent naming conventions
- Use descriptive names
- Follow C++ naming standards (PascalCase for classes, camelCase for methods)

### Phase 4: Testing and Validation

#### 4.1 **Unit Tests**
- Create unit tests for each manager
- Test state transitions
- Test edge cases
- Test error handling

#### 4.2 **Integration Tests**
- Test manager interactions
- Test complete transmission/reception flows
- Test configuration changes
- Test statistics collection

#### 4.3 **Regression Tests**
- Ensure existing functionality is preserved
- Run existing validation scenarios
- Compare output with previous version

## Implementation Plan

### Step 1: Preparation (Week 1)
1. Create backup of current `node.h`
2. Create new directory structure
3. Set up build system for new structure
4. Create base classes/interfaces for managers

### Step 2: Extract Managers (Weeks 2-4)
1. Extract State Manager
2. Extract Channel Manager
3. Extract Transmission Manager
4. Extract Reception Manager
5. Extract Backoff Manager
6. Extract NAV Manager
7. Extract Spatial Reuse Manager
8. Extract MCS Manager
9. Extract Configuration Manager
10. Extract Statistics Manager
11. Extract Logging Manager

### Step 3: Refactor Node Component (Week 5)
1. Refactor Node to use managers
2. Update inports/outports
3. Update triggers
4. Simplify Node methods

### Step 4: Cleanup and Documentation (Week 6)
1. Remove redundant code
2. Extract constants
3. Add documentation
4. Standardize naming

### Step 5: Testing (Week 7)
1. Write unit tests
2. Write integration tests
3. Run regression tests
4. Fix bugs

### Step 6: Validation (Week 8)
1. Run validation scenarios
2. Compare results with previous version
3. Performance testing
4. Final review

## Benefits

### 1. **Maintainability**
- Smaller, focused files (200-500 lines each)
- Clear responsibilities
- Easy to locate and fix bugs
- Easier code reviews

### 2. **Extensibility**
- Easy to add new features
- Easy to modify existing features
- Plug-in architecture for managers
- Clear extension points

### 3. **Testability**
- Unit testable components
- Isolated testing
- Mockable dependencies
- Better test coverage

### 4. **Documentation**
- Self-documenting code structure
- Clear interfaces
- Better documentation
- Easier onboarding

### 5. **Performance**
- Potential for optimization
- Better caching opportunities
- Reduced memory footprint
- Faster compilation

## Risks and Mitigation

### Risk 1: Breaking Existing Functionality
**Mitigation**: 
- Comprehensive testing
- Gradual migration
- Keep old code until validation complete
- Regression test suite

### Risk 2: Performance Degradation
**Mitigation**:
- Profile before and after
- Optimize hot paths
- Use inline functions where appropriate
- Minimize abstraction overhead

### Risk 3: Increased Complexity
**Mitigation**:
- Clear documentation
- Simple interfaces
- Good naming
- Code reviews

### Risk 4: Time Investment
**Mitigation**:
- Phased approach
- Incremental delivery
- Prioritize high-value refactorings
- Reuse existing code where possible

## Success Metrics

1. **Code Size**: Reduce from 5,667 lines to <500 lines per file
2. **Method Length**: No method >100 lines
3. **Cyclomatic Complexity**: Reduce average complexity by 50%
4. **Test Coverage**: Achieve >80% test coverage
5. **Documentation**: 100% of public methods documented
6. **Performance**: No performance regression
7. **Functionality**: 100% feature parity with original

## Conclusion

This refactoring plan transforms the monolithic Node component into a modular, maintainable, and extensible architecture. The phased approach minimizes risk while maximizing benefits. The new structure will make it easier for both authors and the community to understand, maintain, and extend the codebase.

## Next Steps

1. Review and approve this plan
2. Create detailed design documents for each manager
3. Set up development environment
4. Begin Phase 1 implementation

