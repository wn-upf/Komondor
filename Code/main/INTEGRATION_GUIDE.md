# Node Component Integration Guide

## Overview

This guide provides step-by-step instructions for integrating the manager classes into the Node component. The integration is designed to be incremental and safe, with testing at each step.

## Pre-Integration Checklist

- [x] All 11 managers created and tested
- [x] Managers have consistent interfaces
- [x] Backward compatibility methods available
- [ ] Backup of current node.h created
- [ ] Test suite prepared
- [ ] Integration plan reviewed

## Integration Strategy

### Approach: Incremental Integration with Backward Compatibility

1. **Add managers as private members** - Managers are internal implementation details
2. **Keep public interface unchanged** - Maintain backward compatibility
3. **Delegate to managers** - Public methods call manager methods
4. **Sync with existing variables** - Temporarily sync manager state with existing variables
5. **Remove redundant code** - After validation, remove duplicate code

## Step-by-Step Integration

### Step 1: Add Manager Includes and Declarations

Add to the top of `node.h` (after existing includes):

```cpp
// Manager includes
#include "node_state_manager.h"
#include "node_logging_manager.h"
#include "node_channel_manager.h"
#include "node_backoff_manager.h"
#include "node_nav_manager.h"
#include "node_mcs_manager.h"
#include "node_statistics_manager.h"
#include "node_spatial_reuse_manager.h"
#include "node_configuration_manager.h"
#include "node_transmission_manager.h"
#include "node_reception_manager.h"
```

Add to private section of Node class:

```cpp
private:
    // Managers
    NodeStateManager state_manager_;
    NodeLoggingManager logging_manager_;
    NodeChannelManager channel_manager_;
    NodeBackoffManager backoff_manager_;
    NodeNavManager nav_manager_;
    NodeMCSManager mcs_manager_;
    NodeStatisticsManager statistics_manager_;
    NodeSpatialReuseManager spatial_reuse_manager_;
    NodeConfigurationManager config_manager_;
    NodeTransmissionManager transmission_manager_;
    NodeReceptionManager reception_manager_;
```

### Step 2: Initialize Managers in InitializeVariables()

Add manager initialization at the beginning of `InitializeVariables()`:

```cpp
void Node::InitializeVariables() {
    // Initialize managers
    state_manager_.Initialize(STATE_SENSING);
    logging_manager_.Initialize();
    channel_manager_.Initialize(min_channel_allowed, max_channel_allowed);
    backoff_manager_.Initialize(pdf_backoff, backoff_type, cw_min_default, 
                                 cw_max_default, cw_stage_max, cw_adaptation);
    nav_manager_.Initialize();
    mcs_manager_.Initialize(wlan.num_stas);
    statistics_manager_.Initialize(num_stas, NUM_CHANNELS_KOMONDOR, num_channels_allowed);
    spatial_reuse_manager_.Initialize(bss_color, srg, non_srg_obss_pd, 
                                       srg_obss_pd, sensitivity_default);
    config_manager_.Initialize();
    transmission_manager_.Initialize(frame_length, max_num_packets_aggregated);
    reception_manager_.Initialize();
    
    // ... rest of existing initialization code
}
```

### Step 3: Integrate NodeStateManager (Example)

#### Replace state assignments:

```cpp
// Before:
node_state = STATE_TX_DATA;

// After:
state_manager_.SetStateFromInt(STATE_TX_DATA);
node_state = state_manager_.GetStateAsInt(); // Sync for backward compatibility
```

#### Replace state checks:

```cpp
// Before:
if (node_state == STATE_SENSING) { ... }

// After:
if (state_manager_.IsSensing()) { ... }
// Or:
if (state_manager_.GetStateAsInt() == STATE_SENSING) { ... }
```

### Step 4: Integrate NodeLoggingManager

#### Replace logging calls:

```cpp
// Before:
LOGS(save_node_logs, node_logger.file, "Message: %d\n", value);

// After:
if (logging_manager_.IsLoggingEnabled()) {
    logging_manager_.LogEvent("Message: %d\n", value);
}
```

#### Update StartSavingLogs():

```cpp
void Node::StartSavingLogs(trigger_t &) {
    char file_path[256];
    sprintf(file_path, "%s_%s_N%d_%s.txt", "../output/logs_output", 
            simulation_code.c_str(), node_id, node_code.c_str());
    logging_manager_.InitializeLogging(file_path, TRUE);
}
```

### Step 5: Integrate NodeChannelManager

#### Replace channel power access:

```cpp
// Before:
channel_power[channel] = power;

// After:
channel_manager_.UpdateChannelPower(channel, power, true);
// Sync for backward compatibility:
double* channel_powers = channel_manager_.GetChannelPowers();
channel_power[channel] = channel_powers[channel];
```

#### Replace channel selection:

```cpp
// Before:
GetTxChannels(channels_for_tx, ...);

// After:
channel_manager_.GetTxChannels(channels_for_tx, ...);
```

### Step 6: Continue with Remaining Managers

Follow the same pattern for each manager:
1. Add manager initialization
2. Replace direct variable access with manager methods
3. Sync state for backward compatibility (temporary)
4. Test and validate
5. Remove redundant code after validation

## Testing Strategy

### Unit Tests

Create unit tests for each manager:

```cpp
// Example: test_node_state_manager.cpp
#include "node_state_manager.h"
#include <assert.h>

void test_state_manager() {
    NodeStateManager manager(STATE_SENSING);
    assert(manager.IsSensing());
    assert(manager.SetStateFromInt(STATE_TX_DATA));
    assert(manager.IsTransmitting());
}
```

### Integration Tests

Test Node component with managers:

```cpp
// Example: test_node_integration.cpp
// Create a Node instance
// Test that managers work correctly
// Test that functionality is preserved
```

### Regression Tests

Run existing test scenarios:

```bash
cd Code/input
./script_regression_validation_scenarios.sh
```

Compare outputs before and after integration.

## Backward Compatibility

During integration, maintain backward compatibility by:

1. **Keeping public variables** - Don't remove `node_state` immediately
2. **Syncing state** - Keep manager state in sync with public variables
3. **Gradual migration** - Replace usage incrementally
4. **Testing** - Verify behavior doesn't change

## Cleanup Phase

After all managers are integrated and tested:

1. **Remove redundant variables** - Remove variables now managed by managers
2. **Remove sync code** - Remove temporary synchronization code
3. **Update public interface** - Make managers the source of truth
4. **Remove unused includes** - Clean up includes
5. **Update documentation** - Document new architecture

## Common Patterns

### Pattern 1: State Management

```cpp
// Old:
node_state = new_state;
if (node_state == STATE_X) { ... }

// New:
state_manager_.SetStateFromInt(new_state);
node_state = state_manager_.GetStateAsInt(); // Sync
if (state_manager_.IsInState(NodeInternal::STATE_X)) { ... }
```

### Pattern 2: Logging

```cpp
// Old:
LOGS(save_node_logs, node_logger.file, format, ...);

// New:
logging_manager_.LogEvent(format, ...);
```

### Pattern 3: Channel Management

```cpp
// Old:
channel_power[ch] = power;
if (channel_power[ch] < threshold) { ... }

// New:
channel_manager_.UpdateChannelPower(ch, power, true);
if (channel_manager_.IsChannelFree(ch, threshold)) { ... }
```

### Pattern 4: Statistics

```cpp
// Old:
data_packets_sent++;
throughput = (data_frames_acked * frame_length) / SimTime();

// New:
statistics_manager_.RecordDataPacketSent();
double throughput = statistics_manager_.GetThroughput();
```

## Troubleshooting

### Issue: State not syncing correctly

**Solution**: Ensure manager state is synced with public variables after each state change.

### Issue: Performance degradation

**Solution**: Managers use inline implementations. Check for unnecessary copying or synchronization.

### Issue: Compilation errors

**Solution**: 
1. Check includes are correct
2. Check manager constructors are called correctly
3. Check method signatures match

### Issue: Runtime errors

**Solution**:
1. Check managers are initialized before use
2. Check manager state is valid
3. Check for null pointer accesses

## Validation Checklist

After integration:

- [ ] All managers initialized correctly
- [ ] State management works correctly
- [ ] Logging works correctly
- [ ] Channel management works correctly
- [ ] Backoff management works correctly
- [ ] NAV management works correctly
- [ ] MCS management works correctly
- [ ] Statistics collection works correctly
- [ ] Spatial reuse works correctly
- [ ] Configuration management works correctly
- [ ] Transmission management works correctly
- [ ] Reception management works correctly
- [ ] All regression tests pass
- [ ] Performance is acceptable
- [ ] Code is cleaner and more maintainable
- [ ] Documentation is updated

## Next Steps

1. **Create backup** of current node.h
2. **Start integration** with NodeStateManager
3. **Test thoroughly** after each manager
4. **Continue incrementally** with remaining managers
5. **Clean up** redundant code
6. **Update documentation**
7. **Run validation tests**

## Notes

- Integration can be done incrementally
- Each step should be tested before proceeding
- Backward compatibility should be maintained during transition
- Performance should be monitored throughout
- Documentation should be updated as integration progresses

