# Node Component Architecture Diagram

## Current Architecture (Monolithic)

```
┌─────────────────────────────────────────────────────────────┐
│                      node.h (5,667 lines)                    │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Mixed Concerns (All in one class)                   │  │
│  │  • State Management                                   │  │
│  │  • Channel Management                                 │  │
│  │  • Transmission Logic                                 │  │
│  │  • Reception Logic                                    │  │
│  │  • Backoff Management                                 │  │
│  │  • NAV Management                                     │  │
│  │  • Spatial Reuse                                      │  │
│  │  • MCS Selection                                      │  │
│  │  • Configuration Management                           │  │
│  │  • Statistics Collection                              │  │
│  │  • Logging                                            │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
│  Issues:                                                     │
│  • Too large (5,667 lines)                                 │
│  • Too complex (45+ methods)                               │
│  • Mixed concerns                                          │
│  • Difficult to test                                       │
│  • Difficult to extend                                     │
└─────────────────────────────────────────────────────────────┘
```

## Proposed Architecture (Modular)

```
┌─────────────────────────────────────────────────────────────┐
│                    Node Component (Coordinator)              │
│                      node.h (~300 lines)                     │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  COST Component Interface                             │  │
│  │  • Setup()                                            │  │
│  │  • Start()                                            │  │
│  │  • Stop()                                             │  │
│  │  • Inports (delegated to managers)                   │  │
│  │  • Outports                                           │  │
│  │  • Triggers                                           │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   State      │  │   Channel    │  │ Transmission │     │
│  │   Manager    │  │   Manager    │  │   Manager    │     │
│  │  (~200 lines)│  │  (~250 lines)│  │  (~300 lines)│     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│         │                 │                 │               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │  Reception   │  │   Backoff    │  │     NAV      │     │
│  │   Manager    │  │   Manager    │  │   Manager    │     │
│  │  (~400 lines)│  │  (~250 lines)│  │  (~200 lines)│     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│         │                 │                 │               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   Spatial    │  │     MCS      │  │ Configuration│     │
│  │    Reuse     │  │   Manager    │  │   Manager    │     │
│  │   Manager    │  │  (~200 lines)│  │  (~200 lines)│     │
│  │  (~300 lines)│  └──────────────┘  └──────────────┘     │
│  └──────────────┘                                         │
│         │                                                 │
│  ┌──────────────┐  ┌──────────────┐                     │
│  │  Statistics  │  │   Logging    │                     │
│  │   Manager    │  │   Manager    │                     │
│  │  (~300 lines)│  │  (~200 lines)│                     │
│  └──────────────┘  └──────────────┘                     │
└─────────────────────────────────────────────────────────────┘
```

## Manager Relationships

```
                    ┌─────────────┐
                    │ Node (Main) │
                    └──────┬──────┘
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
        ▼                  ▼                  ▼
┌───────────────┐  ┌───────────────┐  ┌───────────────┐
│ State Manager │  │ Channel       │  │ Transmission  │
│               │  │ Manager       │  │ Manager       │
│ • State       │  │ • Channel     │  │ • RTS/CTS     │
│   transitions │  │   selection   │  │ • DATA/ACK    │
│ • State       │  │ • Channel     │  │ • Packet      │
│   validation  │  │   bonding     │  │   generation  │
└───────┬───────┘  └───────┬───────┘  └───────┬───────┘
        │                  │                  │
        │                  │                  │
        ▼                  ▼                  ▼
┌───────────────┐  ┌───────────────┐  ┌───────────────┐
│ Reception     │  │ Backoff       │  │ NAV Manager   │
│ Manager       │  │ Manager       │  │               │
│ • Packet      │  │ • Backoff     │  │ • NAV state   │
│   decoding    │  │   computation │  │ • NAV timeout │
│ • SINR        │  │ • CW          │  │ • Inter-BSS   │
│   calculation │  │   management  │  │   NAV         │
│ • Interference│  │ • Pause/      │  │               │
│   handling    │  │   Resume      │  │               │
└───────┬───────┘  └───────┬───────┘  └───────┬───────┘
        │                  │                  │
        │                  │                  │
        ▼                  ▼                  ▼
┌───────────────┐  ┌───────────────┐  ┌───────────────┐
│ Spatial Reuse │  │ MCS Manager   │  │ Configuration │
│ Manager       │  │               │  │ Manager       │
│ • OBSS-PD     │  │ • MCS         │  │ • Config      │
│   thresholds  │  │   selection   │  │   generation  │
│ • TXOP        │  │ • MCS         │  │ • Config      │
│   identification│ │   adaptation │  │   application │
│ • Power       │  │ • SINR-based  │  │ • Agent       │
│   restriction │  │   selection   │  │   interaction │
└───────┬───────┘  └───────┬───────┘  └───────┬───────┘
        │                  │                  │
        │                  │                  │
        ▼                  ▼                  ▼
┌───────────────┐  ┌───────────────┐
│ Statistics    │  │ Logging       │
│ Manager       │  │ Manager       │
│ • Throughput  │  │ • File        │
│ • Delay       │  │   logging     │
│ • Packet      │  │ • Console     │
│   statistics  │  │   output      │
│ • Performance │  │ • Event       │
│   reports     │  │   logging     │
└───────────────┘  └───────────────┘
```

## Data Flow

### Transmission Flow

```
Node::StartTransmission()
    │
    ├─→ TransmissionManager::PrepareTransmission()
    │       │
    │       ├─→ ChannelManager::GetAvailableChannels()
    │       ├─→ MCSManager::GetMCSForDestination()
    │       ├─→ SpatialReuseManager::GetLimitedTxPower()
    │       └─→ TransmissionManager::GenerateRTS()
    │
    ├─→ StateManager::SetState(TRANSMITTING)
    │
    ├─→ StatisticsManager::RecordPacketSent()
    │
    └─→ LoggingManager::LogEvent("RTS transmitted")
```

### Reception Flow

```
Node::InportSomeNodeStartTX(Notification)
    │
    ├─→ ReceptionManager::ProcessIncomingPacket()
    │       │
    │       ├─→ ChannelManager::UpdateChannelPower()
    │       ├─→ ReceptionManager::CalculateSINR()
    │       ├─→ ReceptionManager::CanDecodePacket()
    │       └─→ ReceptionManager::UpdateInterference()
    │
    ├─→ NavManager::SetNAV() [if decodable]
    │
    ├─→ BackoffManager::PauseBackoff() [if in backoff]
    │
    ├─→ SpatialReuseManager::IdentifySpatialReuseOpportunity()
    │
    ├─→ StatisticsManager::RecordPacketReceived()
    │
    └─→ LoggingManager::LogEvent("Packet received")
```

### Configuration Flow

```
Node::InportReceiveConfigurationFromAgent(Configuration)
    │
    ├─→ ConfigurationManager::ValidateConfiguration()
    │
    ├─→ ConfigurationManager::ApplyConfiguration()
    │       │
    │       ├─→ ChannelManager::UpdateChannelSettings()
    │       ├─→ SpatialReuseManager::UpdateSRSettings()
    │       ├─→ BackoffManager::UpdateBackoffSettings()
    │       └─→ MCSManager::UpdateMCSSettings()
    │
    ├─→ StateManager::ValidateStateTransition()
    │
    └─→ LoggingManager::LogEvent("Configuration applied")
```

## Manager Dependencies

```
Node (Main Component)
    │
    ├─→ StateManager (no dependencies)
    │
    ├─→ ChannelManager (no dependencies)
    │
    ├─→ TransmissionManager
    │       ├─→ ChannelManager
    │       ├─→ MCSManager
    │       └─→ SpatialReuseManager
    │
    ├─→ ReceptionManager
    │       ├─→ ChannelManager
    │       ├─→ StateManager
    │       └─→ SpatialReuseManager
    │
    ├─→ BackoffManager
    │       └─→ StateManager
    │
    ├─→ NavManager
    │       └─→ StateManager
    │
    ├─→ SpatialReuseManager
    │       └─→ ChannelManager
    │
    ├─→ MCSManager
    │       └─→ ChannelManager
    │
    ├─→ ConfigurationManager
    │       ├─→ ChannelManager
    │       ├─→ SpatialReuseManager
    │       ├─→ BackoffManager
    │       └─→ MCSManager
    │
    ├─→ StatisticsManager (no dependencies)
    │
    └─→ LoggingManager (no dependencies)
```

## Benefits Visualization

### Before (Monolithic)
```
┌─────────────────────────────────┐
│        node.h (5,667 lines)     │
│                                 │
│  • Hard to navigate            │
│  • Difficult to test           │
│  • Mixed concerns              │
│  • High coupling               │
│  • Low cohesion                │
│  • Difficult to extend         │
└─────────────────────────────────┘
```

### After (Modular)
```
┌─────────────────────────────────┐
│    Node (Coordinator, ~300)     │
│                                 │
│  ┌──────────┐  ┌──────────┐   │
│  │ Manager  │  │ Manager  │   │
│  │  (~200)  │  │  (~250)  │   │
│  └──────────┘  └──────────┘   │
│                                 │
│  • Easy to navigate            │
│  • Easy to test                │
│  • Single responsibility       │
│  • Low coupling                │
│  • High cohesion               │
│  • Easy to extend              │
└─────────────────────────────────┘
```

## File Organization

```
Code/main/
│
├── node.h                          # Main Node component
│   └── Coordinates all managers
│
├── node_state_manager.h            # State management
├── node_channel_manager.h          # Channel management
├── node_transmission_manager.h     # Transmission logic
├── node_reception_manager.h        # Reception logic
├── node_backoff_manager.h          # Backoff procedures
├── node_nav_manager.h              # NAV management
├── node_spatial_reuse_manager.h    # Spatial reuse
├── node_mcs_manager.h              # MCS selection
├── node_configuration_manager.h    # Configuration
├── node_statistics_manager.h       # Statistics
└── node_logging_manager.h          # Logging
│
└── node_internal/
    ├── node_types.h                # Types and enums
    ├── node_constants.h            # Constants
    └── node_utilities.h            # Utility functions
```

## Summary

The refactoring transforms a monolithic 5,667-line file into a modular architecture with:

- **11 focused managers** (~200-400 lines each)
- **Clear responsibilities** (single responsibility principle)
- **Low coupling** (minimal dependencies between managers)
- **High cohesion** (related functionality grouped together)
- **Easy to test** (unit testable components)
- **Easy to extend** (clear extension points)
- **Professional structure** (well-documented and organized)

This architecture makes the codebase more maintainable, testable, and extensible for both authors and the community.

