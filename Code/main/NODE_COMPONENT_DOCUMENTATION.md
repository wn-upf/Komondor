# Node Component Documentation

## Overview

The `Node` component is the core of the Komondor wireless network simulator. It represents a single node (Access Point or Station) in a WLAN and handles all aspects of wireless communication, including channel access, packet transmission, reception, and state management.

## Architecture

The Node component has been refactored to use a modular manager-based architecture. Each manager encapsulates a specific set of responsibilities, improving code organization, maintainability, and extensibility.

### Manager Classes

The Node component uses the following manager classes:

1. **NodeStateManager**: Manages node state transitions and state queries
2. **NodeChannelManager**: Manages channel power sensing and channel selection
3. **NodeBackoffManager**: Manages backoff computation and contention window adaptation
4. **NodeNavManager**: Manages Network Allocation Vector (NAV) state and timeouts
5. **NodeLoggingManager**: Manages logging operations (file and console)
6. **NodeMCSManager**: Manages Modulation and Coding Scheme (MCS) selection
7. **NodeStatisticsManager**: Manages statistics collection and reporting
8. **NodeSpatialReuseManager**: Manages spatial reuse operations
9. **NodeConfigurationManager**: Manages node configuration
10. **NodeTransmissionManager**: Manages transmission operations
11. **NodeReceptionManager**: Manages reception operations

## Component Structure

### Public Interface

The Node component provides the following public methods:

#### COST Lifecycle Methods
- `Setup()`: Initialize the node component
- `Start()`: Start the node simulation
- `Stop()`: Stop the node simulation

#### Generic Methods
- `InitializeVariables()`: Initialize all node variables
- `RestartNode()`: Restart the node after a transmission or timeout
- `CallRestartSta()`: Restart STA nodes
- `CallSensing()`: Return to sensing state
- `PrintNodeInfo()`: Print node information
- `PrintNodeConfiguration()`: Print node configuration
- `WriteNodeInfo()`: Write node information to log file
- `WriteNodeConfiguration()`: Write node configuration to log file
- `WriteReceivedConfiguration()`: Write received configuration to log file
- `PrintOrWriteNodeStatistics()`: Print or write node statistics
- `HandleSlottedBackoffCollision()`: Handle slotted backoff collisions
- `StartSavingLogs()`: Start saving logs to file
- `RecoverFromCtsTimeout()`: Recover from CTS timeout
- `MeasureRho()`: Measure channel utilization
- `SaveSimulationPerformance()`: Save simulation performance metrics

#### Packet Methods
- `GenerateNotification()`: Generate a notification packet
- `SelectDestination()`: Select a destination for transmission
- `SendResponsePacket()`: Send a response packet (ACK/CTS)
- `AckTimeout()`: Handle ACK timeout
- `CtsTimeout()`: Handle CTS timeout
- `DataTimeout()`: Handle DATA timeout
- `NavTimeout()`: Handle NAV timeout
- `RequestMCS()`: Request MCS selection
- `StartTransmission()`: Start a transmission
- `AbortRtsTransmission()`: Abort an RTS transmission
- `SendLogicalNack()`: Send a logical NACK

#### Backoff Methods
- `PauseBackoff()`: Pause the backoff counter
- `ResumeBackoff()`: Resume the backoff counter

#### Configuration Methods
- `GenerateConfiguration()`: Generate node configuration
- `ApplyNewConfiguration()`: Apply a new configuration
- `BroadcastNewConfigurationToStas()`: Broadcast configuration to STAs
- `UpdatePerformanceMeasurements()`: Update performance measurements

#### Spatial Reuse Methods
- `SpatialReuseOpportunityEnds()`: Handle spatial reuse opportunity end

### Private Members

The Node component contains various private members for:
- Node identification and configuration
- Channel and transmission parameters
- Backoff and contention window state
- Statistics and performance metrics
- Manager instances

### Manager Integration

Managers are integrated into the Node component as follows:

1. **Initialization**: Managers are initialized in `InitializeVariables()`
2. **Usage**: Node methods call manager methods instead of directly manipulating state
3. **Synchronization**: Node variables are synced with managers for backward compatibility
4. **State Management**: Managers maintain their own internal state

## Usage Example

```cpp
// Node is created and initialized by the simulation framework
// Managers are automatically initialized in InitializeVariables()

// State transitions use the state manager
state_manager_.SetStateFromInt(STATE_TX_RTS);
node_state = state_manager_.GetStateAsInt(); // Sync for backward compatibility

// Channel power updates use the channel manager
channel_manager_.UpdateChannelsPowerFromNotification(notification, TX_INITIATED,
    central_frequency, path_loss_model, adjacent_channel_model, 
    received_power_array[notification.source_id], node_id);

// Backoff operations use the backoff manager
remaining_backoff = backoff_manager_.ComputeNewBackoff(current_traffic_type);
backoff_manager_.PauseBackoff(time_remaining);
remaining_backoff = backoff_manager_.ResumeBackoff();

// NAV operations use the NAV manager
nav_manager_.SetNAV(nav_duration, is_inter_bss, SimTime());
nav_manager_.ClearNAV(SimTime());
```

## State Management

The Node component uses a state machine to track its current operational state. States include:
- `STATE_SENSING`: Sensing the channel and decreasing backoff counter
- `STATE_TX_DATA`: Transmitting data packet
- `STATE_RX_DATA`: Receiving a data packet
- `STATE_WAIT_ACK`: Waiting for ACK after data transmission
- `STATE_TX_ACK`: Transmitting ACK packet
- `STATE_RX_ACK`: Receiving ACK packet
- `STATE_TX_RTS`: Transmitting RTS packet
- `STATE_TX_CTS`: Transmitting CTS packet
- `STATE_RX_RTS`: Receiving RTS packet
- `STATE_RX_CTS`: Receiving CTS packet
- `STATE_WAIT_CTS`: Waiting for CTS packet after RTS
- `STATE_WAIT_DATA`: Waiting for data packet after CTS
- `STATE_NAV`: Virtual Carrier Sense (NAV active)
- `STATE_SLEEP`: Sleep state (not currently used)

State transitions are managed by the `NodeStateManager`, which provides methods for:
- Setting state: `SetState()`, `SetStateFromInt()`
- Getting state: `GetState()`, `GetStateAsInt()`
- State queries: `IsSensing()`, `IsTransmitting()`, `IsReceiving()`, `IsInNAV()`, etc.

## Channel Management

Channel power sensing and channel selection are managed by the `NodeChannelManager`. The manager:
- Maintains channel power array
- Updates channel power based on notifications
- Determines free channels
- Selects transmission channels based on bonding policy

## Backoff Management

Backoff computation and contention window adaptation are managed by the `NodeBackoffManager`. The manager:
- Computes new backoff values
- Manages backoff pause/resume
- Handles contention window adaptation
- Tracks backoff interruptions

## NAV Management

Network Allocation Vector (NAV) state and timeouts are managed by the `NodeNavManager`. The manager:
- Sets and clears NAV
- Tracks NAV timeout
- Manages inter-BSS NAV
- Tracks NAV statistics

## Logging

Logging operations are managed by the `NodeLoggingManager`. The manager:
- Initializes logging to file
- Logs node information and configuration
- Provides logging methods for events
- Manages log file operations

## Integration Status

### Completed Integrations
- ✅ State Manager: Fully integrated
- ✅ Channel Manager: Fully integrated
- ✅ Backoff Manager: Fully integrated (token-based uses legacy code)
- ✅ NAV Manager: Fully integrated
- ✅ Logging Manager: Partially integrated

### Pending Integrations
- 📋 MCS Manager: Not yet integrated
- 📋 Statistics Manager: Not yet integrated
- 📋 Spatial Reuse Manager: Not yet integrated (partially used)
- 📋 Configuration Manager: Not yet integrated
- 📋 Transmission Manager: Not yet integrated
- 📋 Reception Manager: Not yet integrated

## Backward Compatibility

All manager integrations maintain backward compatibility with existing code:
- Node variables are synced with managers after operations
- State synchronization ensures existing code continues to work
- Channel power pointer points to manager's internal array
- Legacy code paths are maintained during transition

## Performance Considerations

- Managers use inline functions for performance
- State synchronization is minimal overhead
- Channel power array is shared (no copying)
- Managers are initialized once at startup

## Extension Points

The manager-based architecture provides several extension points:
1. **New Managers**: Add new manager classes for additional functionality
2. **Manager Methods**: Extend manager methods for new features
3. **State Management**: Add new states to the state machine
4. **Channel Management**: Add new channel selection algorithms
5. **Backoff Management**: Add new backoff algorithms
6. **NAV Management**: Add new NAV handling logic

## Testing

Unit tests are available for managers in the `tests/` directory. See `tests/README.md` for more information.

## References

- `INTEGRATION_COMPLETE_SUMMARY.md`: Complete integration summary
- `INTEGRATION_STATUS.md`: Current integration status
- `INTEGRATION_PROGRESS.md`: Integration progress tracking
- `NODE_REFACTORING_PLAN.md`: Original refactoring plan
- Manager header files: `node_*_manager.h`

---

**Last Updated**: Integration Phase 1-3 Complete
**Status**: ✅ 4 Managers Integrated, Build Successful

