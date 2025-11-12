# Node Refactoring - Implementation Guide

## Overview

This guide provides step-by-step instructions for completing the Node component refactoring. The refactoring is designed to be incremental, allowing managers to be implemented and tested independently.

## Current Status

### ✅ Completed
- Infrastructure (types, constants)
- NodeStateManager (complete)
- NodeLoggingManager (header created)

### 🔄 In Progress
- Manager headers and interfaces

### ⏳ Pending
- Manager implementations
- Node component refactoring
- Testing and validation

## Implementation Steps

### Step 1: Complete NodeLoggingManager Implementation

**File**: `node_logging_manager.h` (header created, implementation needed)

**Tasks**:
1. Implement constructor/destructor
2. Implement `InitializeLogging()`
3. Implement `CloseLogging()`
4. Implement `LogNodeInfo()`
5. Implement `PrintNodeInfo()`
6. Implement `LogNodeConfiguration()`
7. Implement `LogReceivedConfiguration()`
8. Implement `LogEvent()`

**Source Code Location**: 
- Logging methods in `node.h` lines 4845-4956
- Logging initialization in `node.h` lines 550-559

**Testing**:
- Test file creation
- Test log writing
- Test console output
- Test log levels

### Step 2: Create NodeChannelManager

**File**: `node_channel_manager.h` (to be created)

**Responsibilities**:
- Channel power detection
- Channel availability checking
- Channel selection
- Channel bonding
- Channel state management

**Key Methods**:
```cpp
void UpdateChannelPower(int channel, double power);
bool IsChannelFree(int channel, double pd_threshold) const;
std::vector<int> GetAvailableChannels(double pd_threshold) const;
std::pair<int, int> GetTxChannels(int bonding_policy) const;
void ResetChannelState();
```

**Source Code Location**:
- Channel power: `node.h` line 310
- Channel methods: `methods/power_channel_methods.h`
- Channel selection: Throughout `node.h`

### Step 3: Create NodeBackoffManager

**File**: `node_backoff_manager.h` (to be created)

**Responsibilities**:
- Backoff computation
- Backoff pause/resume
- Contention window management
- Backoff state tracking

**Key Methods**:
```cpp
void StartBackoff();
void PauseBackoff();
void ResumeBackoff();
void EndBackoff();
double GetRemainingBackoff() const;
bool IsBackoffActive() const;
void UpdateContentionWindow(bool success);
```

**Source Code Location**:
- Backoff methods: `node.h` lines 4201-4282
- Backoff computation: `methods/backoff_methods.h`
- Backoff variables: `node.h` lines 322, 358-369

### Step 4: Create NodeNavManager

**File**: `node_nav_manager.h` (to be created)

**Responsibilities**:
- NAV state management
- NAV timeout handling
- Inter-BSS NAV handling
- NAV collision detection

**Key Methods**:
```cpp
void SetNAV(double duration, bool is_inter_bss = false);
void ClearNAV();
bool IsInNAV() const;
double GetRemainingNAVTime() const;
void UpdateNAV(const Notification& packet);
```

**Source Code Location**:
- NAV handling: `node.h` lines 4143-4163
- NAV variables: `node.h` line 333
- NAV triggers: Throughout `node.h`

### Step 5: Create Remaining Managers

Follow the same pattern for:
- NodeMCSManager
- NodeStatisticsManager
- NodeSpatialReuseManager
- NodeConfigurationManager
- NodeTransmissionManager
- NodeReceptionManager

### Step 6: Refactor Node Component

**Tasks**:
1. Include manager headers
2. Create manager instances as member variables
3. Replace direct state management with StateManager
4. Replace logging calls with LoggingManager
5. Replace channel operations with ChannelManager
6. Replace backoff operations with BackoffManager
7. Continue for all managers
8. Remove old code
9. Test integration

### Step 7: Testing and Validation

**Tasks**:
1. Unit tests for each manager
2. Integration tests for manager interactions
3. Regression tests for Node component
4. Performance testing
5. Validation against existing scenarios

## Code Extraction Patterns

### Pattern 1: Extract State Management

**Before**:
```cpp
int node_state;
node_state = STATE_TX_DATA;
if (node_state == STATE_TX_DATA) { ... }
```

**After**:
```cpp
NodeStateManager state_manager_;
state_manager_.SetState(NodeInternal::STATE_TX_DATA);
if (state_manager_.IsTransmitting()) { ... }
```

### Pattern 2: Extract Logging

**Before**:
```cpp
LOGS(save_node_logs, node_logger.file, "Message\n");
fprintf(node_logger.file, "Message\n");
```

**After**:
```cpp
logging_manager_.LogEvent("Message\n");
logging_manager_.LogNodeInfo(...);
```

### Pattern 3: Extract Channel Operations

**Before**:
```cpp
channel_power[channel] = power;
if (channel_power[channel] < pd_threshold) { ... }
```

**After**:
```cpp
channel_manager_.UpdateChannelPower(channel, power);
if (channel_manager_.IsChannelFree(channel, pd_threshold)) { ... }
```

## Integration Checklist

### For Each Manager

- [ ] Header file created with clear interface
- [ ] Implementation file created (if needed)
- [ ] All methods implemented
- [ ] Documentation added (Doxygen)
- [ ] Unit tests created
- [ ] Integration with Node component tested
- [ ] Old code removed
- [ ] Backward compatibility maintained (if needed)

### For Node Component

- [ ] All managers included
- [ ] Manager instances created
- [ ] Old code replaced with manager calls
- [ ] Compilation successful
- [ ] Basic functionality tested
- [ ] Regression tests pass
- [ ] Performance validated
- [ ] Documentation updated

## Common Pitfalls

### 1. Circular Dependencies
**Problem**: Managers might need to reference each other
**Solution**: Use forward declarations and dependency injection

### 2. COST Framework Dependencies
**Problem**: Some managers need COST-specific types (triggers, timers)
**Solution**: Keep COST dependencies in Node component, pass needed data to managers

### 3. Performance Overhead
**Problem**: Manager abstraction might add overhead
**Solution**: Use inline functions for hot paths, profile and optimize

### 4. State Consistency
**Problem**: Multiple managers might have conflicting state
**Solution**: Clear ownership of state, use StateManager as source of truth

## Testing Strategy

### Unit Tests

Each manager should have unit tests:
```cpp
TEST(NodeStateManager, SetState) {
    NodeStateManager manager;
    ASSERT_TRUE(manager.SetState(NodeInternal::STATE_TX_DATA));
    ASSERT_TRUE(manager.IsTransmitting());
}
```

### Integration Tests

Test managers working together:
```cpp
TEST(NodeIntegration, TransmissionFlow) {
    NodeStateManager state_manager;
    NodeChannelManager channel_manager;
    // Test transmission flow
}
```

### Regression Tests

Ensure existing functionality preserved:
```cpp
TEST(NodeRegression, BasicTransmission) {
    // Test that basic transmission still works
}
```

## Documentation Requirements

### For Each Manager

1. **Header Documentation**:
   - Purpose and responsibilities
   - Public interface description
   - Usage examples
   - Dependencies

2. **Method Documentation**:
   - Purpose of each method
   - Parameters and return values
   - Preconditions and postconditions
   - Usage examples
   - Error handling

3. **Implementation Notes**:
   - Algorithm descriptions
   - Performance considerations
   - Extension points
   - Known limitations

## Progress Tracking

Update `NODE_REFACTORING_STATUS.md` as you complete each step:
- Mark tasks as completed
- Update implementation status
- Note any issues or blockers
- Document design decisions

## Next Actions

1. **Immediate**: Complete NodeLoggingManager implementation
2. **Short-term**: Create NodeChannelManager and NodeBackoffManager
3. **Medium-term**: Create remaining managers
4. **Long-term**: Refactor Node component and test

## Support

For questions or issues:
1. Review this guide
2. Check `NODE_REFACTORING_PLAN.md` for architecture details
3. Review existing manager implementations for patterns
4. Consult with team members

